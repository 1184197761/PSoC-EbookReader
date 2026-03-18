/* ******************************************************************************
 * 文件名: main_cm4.c
 * 版  本：0.2
 * 日  期：2021-01-01
 * 作  者：Wz
 * IDE   : PSoC Creator 4.4
 * 工具链: ARM GCC 5.4-2016-q2-update
 * 目标板: CYPRESS CY8CKIT-062-WiFi-BT 或 CY8CKIT-062-BLE
 ********************************************************************************
 */
/* 头文件 */
#include "project.h"
#include <BSP.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "GUI.h"
#include <string.h>
#include <math.h>
/* 宏定义 */
#define GESTURE_TIME_INCREMENT 3 /* 指势时间戳增量，设为扫描间隔一半以检测慢速flick动作 */
#define STACK_SIZE 128           /* 任务栈大小的最小推荐值 */
#define TEXTBOX_HEIGHT 224       /* 文本框高度（屏幕高度-页脚高度） */
#define LIST_ROWS 5              /* 书籍列表行数 */
#define LISTBOX_TOP 44           /* 列表框位置（左上角坐标） */
#define LISTBOX_BOTTOM 184       /* 列表框高度（列表行数*行间距） */

/* 全局变量 */
uint32 slider_position;       /* 保存当前电容滑条触摸位置信息（不触摸时为高阻态，不保存） */
uint32 slider_gesture;        /* 保存当前电容滑条触摸手势信息（按下、单击、双击和双击拖动） */
uint32 button_symbol;         /* 保存按键符号（ABCD分别为机械键盘1、2和电容键盘1、2） */
uint32 capBTN0_untouched = 1, /* 外部变量，电容按键0、1当前是否未触摸（抬起） */
    capBTN1_untouched = 1;
GUI_FONT GUI_FontChinese_12, /* 外部变量，中文字体（新宋体），已修改Y轴距离和基线位置，程序中可根据需要自行修改 */
    GUI_FontChinese_16,
    GUI_FontChinese_20;
typedef struct /* 书籍类型（结构体） */
{
    char *name;     /* 书名 */
    char *content;  /* 内容 */
    uint8 marker;   /* 书签 */
    GUI_FONT *font; /* 字体 */
} Book;

Book books[] = {
    /* 书籍数组 */
    {"You Have Only One Life",
     "\tThere are moments in life when you miss someone so much that you just want to pick them from your dreams and hug them for real! Dream what you want to dream;go where you want to go;be what you want to be,because you have only one life and one chance to do all the things you want to do. \n\
     \tMay you have enough happiness to make you sweet,enough trials to make you strong,enough sorrow to keep you human, enough hope to make you happy ? Always put yourself in others'shoes.If you feel that it hurts you, it probably hurts the other person, too.\n\
     The happiest of people don 't necessarily have the best of everything;they just make the most of everything that comes along their way.Happiness lies for those who cry,those who hurt, those who have searched,and those who have tried,for only they can appreciate the importance of people who have touched their lives.Love begins with a smile, grows with a kiss and ends with a tear.The brightest future will always be based on a forgotten past, you can' t go on well in lifeuntil you let go of your past failures and heartaches.\n\
     \tWhen you were born, you were crying and everyone around you was smiling.Live your life so that when you die, you're the one who is smiling and everyone around you is crying.\n\
     \tPlease send this message to those people who mean something to you, to those who have touched your life in one way or another, to those who make you smile when you really need it, to those that make you see the brighter side of things when you are really down, to those who you want to let them know that you appreciate their friendship.And if you don 't, don' t worry, nothing bad will happen to you, you will just miss out on the opportunity to brighten someone's day with this message.",
     1, NULL},
    {"你只有一次生命",
     "\t生命中有些时刻，你会如此想念某人，以至于你只想从梦中把他们拣出来，真正拥抱他们！做你想做的梦；去你想去的地方；成为你想成为的人，因为你只有一次生命和一次机会去做所有你想做的事。\n\
     \t你有足够的快乐让你甜蜜，足够的考验让你坚强，足够的悲伤让你有人性，足够的希望让你快乐吗？总是设身处地为别人着想。如果你觉得它伤害了你，它可能也伤害了其他人。\n\
     \t最幸福的人不一定拥有最好的一切；他们只会充分利用沿途的一切。幸福属于那些哭泣的人，那些受伤的人，那些寻找过的人，那些尝试过的人，因为只有他们才能体会到那些影响过他们生活的人的重要性。爱从微笑开始，在亲吻中成长，在眼泪中结束。最光明的未来总是建立在忘记过去的基础上，只有放下过去的失败和心痛，你的生活才能过得很好。\n\
     \t当你出生时，你在哭泣，你周围的人都在微笑。过好你的生活，这样当你死的时候，你就是那个微笑的人，而你周围的人都在哭泣。\n\
     \t请将此信息传达给那些对你有意义的人，那些以某种方式影响你生活的人，那些在你真正需要时让你微笑的人，那些在你情绪低落时让你看到事情光明面的人，那些你想让他们知道你欣赏他们友谊的人。如果你不这样做，别担心，不会有什么不好的事情发生在你身上，你只会错失用这条信息来照亮别人一天的机会。",
     1, NULL},
    {"book 3", "This is the third book!", 1, NULL},
    {"book 4", "This is the fourth book!", 1, NULL},
    {"book 5", "This is the firth book!", 1, NULL},
    {"book 6", "This is the sixth book!", 1, NULL},
    {"book 7", "This is the seventh book!", 1, NULL},
    {"book 8", "This is the eighth book!", 1, NULL},
};

/* 任务函数声明 */
void StartupTask();        /* 开启任务 */
void LcdTask();            /* Lcd任务 */
void TouchPBTask();        /* 触摸板任务 */
void ReadView(Book *book); /* 阅读视图 */

/* 主函数
   进行板的前段初始化（除emWin和开始A/D转换），创建第一个用户任务StartupTask，然后启动FreeRTOS并不再返回。
 */
int main(void)
{
    /* FreeRTOS启动前的电路板初始化 */
    BSP_PreInit();
    /* 使用断言确保正确创建启动任务 */
    configASSERT(
        xTaskCreate(StartupTask, "StartupTask", STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL));
    /* 启动计划程序-除非出现问题，否则不应返回 */
    vTaskStartScheduler();
    /* 如果您在这里结束了，那么可能存在溢出FreeRTOS堆的问题 */
    while (true)
        ;
}

/* 初始任务
   进行板的后段初始化（emWin），然后创建其余2个用户任务，最后删除自己。
 */
void StartupTask()
{
    /* FreeRTOS启动后的电路板初始化 */
    BSP_PostInit();
    /* 使用断言确保正确创建动态任务 */
    configASSERT(
        xTaskCreate(LcdTask, "LcdTask", STACK_SIZE * 10, NULL, tskIDLE_PRIORITY + 1, NULL) &
        xTaskCreate(TouchPBTask, "TouchPBTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL));
    /* 任务可以通过向vTaskDelete传递NULL来删除自身 */
    vTaskDelete(NULL);
    while (1)
        ; /* 任务永远无法完成，它就在这里 */
}

/* LCD显示更新任务
   显示列表和书籍信息，除非系统错误，否则持续运行直到关机
 */
void LcdTask()
{
    char title[32];      /* 标题缓冲区 */
    uint8 listpage = 0;  /* 书籍列表页 */
    uint8 index_cur = 0; /* 当前索引（对应书籍数组下标值，这里不能使用index，因其在arm_math.h中已定义） */
    uint8 books_num =    /* 书籍数量 */
        sizeof(books) / sizeof(*books);

    while (true)
    {
        /* 这里不需要清除屏幕是因为未翻页文本可以完美覆盖 */
        GUI_SetFont(GUI_FONT_32B_ASCII);             /* 设置软件名字体 */
        GUI_DispStringHCenterAt("E-Reader", 160, 0); /* 居中打印软件名 */
        GUI_GotoXY(0, LISTBOX_TOP);                  /* 跳转到文本框首行，这里换行会与标题左对齐 */
        GUI_SetFont(&GUI_FontChinese_16);            /* 设置主页字体 */
        if (index_cur / LIST_ROWS != listpage)       /* 若书籍不在当前列表页则更新列表 */
        {
            GUI_ClearRect(0, LISTBOX_TOP, 320, LISTBOX_BOTTOM); /* 清除列表框，防止翻页时字体残留 */
            listpage = index_cur / LIST_ROWS;                   /* 切换到当前书籍所在列表页 */
        }
        /* 遍历当前列表页，首行索引=列表页*行数 */
        for (int i = listpage * LIST_ROWS; i < books_num && i < (listpage + 1) * LIST_ROWS; i++)
        {
            if (i == index_cur) /* 若书籍选中（当前行为选中行） */
            {
                // GUI_SetTextMode(GUI_TEXTMODE_REV); /* 当前行反色显示（修改字体YDist后无法显示，原因不明） */
                GUI_SetTextStyle(GUI_TS_UNDERLINE); /* 当前行添加下划线 */
            }
            sprintf(title, "%03d %s\n", i + 1, books[i].name); /* 格式化赋值标题 */
            GUI_DispString(title);                             /* 显示标题 */
            GUI_SetTextStyle(GUI_TS_NORMAL);                   /* 恢复正常显示 */
        }
        GUI_DispStringAt("</> 滑条L/R-选择书籍 | 划动-翻页\n", 0, 196);
        GUI_DispString("(X) 按键0-进入阅读 | 按键1-切换主题");

        while (!slider_gesture && !button_symbol) /* 等待直到检测到输入 */
            ;
        switch (slider_gesture) /* 检测滑条左/右划动手势，修改了长度和时间阈值，使之更容易触发 */
        {
        /* 左划 */
        case CapSense_ONE_FINGER_FLICK_LEFT:
            if (listpage < (books_num - 1) / LIST_ROWS) /* 当前不是最后一页 */
            {
                index_cur = listpage * LIST_ROWS + LIST_ROWS; /* 选择下页第一行 */
            }
            break;
        /* 右划 */
        case CapSense_ONE_FINGER_FLICK_RIGHT:
            if (listpage > 0) /* 当前不是第一页 */
            {
                index_cur = listpage * LIST_ROWS - LIST_ROWS; /* 选择上页第一行 */
            }
            break;
        /* 点击，这里不用LIFT_OFF是因为松开后无法判断触摸位置（为高阻态）
           不保存高阻态值后，可以使用LIFT_OFF，但由于开启了CLICK手势，必须使用SINGLE_CLICK和DOLBLE_CLICK(快速单击会触发双击，不加入可能导致第二次点击无反应）
           这里开启手势是因为阅读界面要用到双击拖拽
        */
        case CapSense_ONE_FINGER_SINGLE_CLICK:
        case CapSense_ONE_FINGER_DOUBLE_CLICK:
            if (slider_position < 50 && index_cur > 0) /* 点击左半区域，如果不是第一行则执行语句 */
            {
                index_cur--; /* 选择上一行 */
            }
            else if (slider_position > 50 && index_cur < books_num - 1) /* 右半区域，这里仍然要判断触摸位置 */
            {
                index_cur++; /* 选择下一行 */
            }
            break;
        }
        switch (button_symbol) /* 检测按键消息（这里不检测机械按键，个人感觉不顺手） */
        {
        /* 若获取到按键0消息 */
        case 'C':
            ReadView(&books[index_cur]); /* 进入阅读视图 */
            GUI_Clear();                 /* 返回时清除屏幕防止文本残留 */
            break;
        /* 若获取到按键1消息 */
        case 'D':
            GUI_SetBkColor(~GUI_GetBkColor()); /* 反转背景色 */
            GUI_SetColor(~GUI_GetColor());     /* 反转前景色 */
            GUI_Clear();                       /* 以新的背景色覆盖屏幕 */
            break;
        }
    }
}
/* 阅读视图
   根据当前书籍显示阅读区和和状态栏
 */
void ReadView(Book *book)
{
    char footer[64];               /* 页脚缓冲区 */
    float32 progress;              /* 阅读进度 */
    uint8 pageNum;                 /* 书籍页数 */
    uint8 page_cur = book->marker; /* 当前页码 */
    GUI_FONT *font_cur =           /* 阅读字体 */
        book->font ? book->font : &GUI_FontChinese_16;
    GUI_RECT rect = /* 文本框矩形 */
        {0, (1 - page_cur) * TEXTBOX_HEIGHT, 320, TEXTBOX_HEIGHT - 1};

    GUI_SetFont(font_cur); /* 装载阅读字体，提前设置是因为字体大小会影响GUI_WrapGetNumLines()返回值 */
    /* ，为文本总行数/每页行数；每页行数=文本框高度/每行高度（总行数/行数很可能有余数，应向上取整） */
    pageNum = ceil(GUI_WrapGetNumLines(book->content, rect.x1, GUI_WRAPMODE_CHAR) / (double)(TEXTBOX_HEIGHT / font_cur->YDist));
    while (true)
    {
        GUI_SetFont(font_cur);                             /* 装载阅读字体 */
        GUI_ClearRect(rect.x0, rect.y0, rect.x1, rect.y1); /* 清除上一页面的字体残留，缺点是翻页闪屏现象比较严重 */
        GUI_DispStringInRectWrap(                          /* 在文本框中打印文本信息，左对齐并以字符方式换行（单词会被拆开，但是比较整齐） */
                                 book->content, &rect, GUI_TA_LEFT, GUI_WRAPMODE_CHAR);
        GUI_SetFont(&GUI_FontChinese_16); /* 单独设置页脚字体，避免缩放导致页脚溢出 */
        sprintf(footer, "</> 页码 %03d/%03d  (X) 字号 %d | 退出", page_cur, pageNum, font_cur->YSize);
        GUI_DispStringAt(footer, 0, rect.y1 + 1); /* 在文本框下方打印页脚信息 */
        do                                        /* 循环检测滑条手势和按键消息 */
        {
            book->marker = page_cur; /* 保存当前页面以便循环判断是否翻页 */
            book->font = font_cur;   /* 保存当前字体以便循环判断是否缩放 */
            switch (slider_gesture)  /* 检测滑条手势 */
            {
            /* 左滑 */
            case CapSense_ONE_FINGER_FLICK_LEFT:
                if (page_cur < pageNum) /* 当前不是最后一页 */
                {
                    page_cur++;                /* 往后翻页 */
                    rect.y0 -= TEXTBOX_HEIGHT; /* 文本框位置上移一个文本框高度，显示下一页内容 */
                }
                break;
            /* 右滑 */
            case CapSense_ONE_FINGER_FLICK_RIGHT:
                if (page_cur > 1) /* 当前不是第一页 */
                {
                    page_cur--; /* 往前翻页 */
                    rect.y0 += TEXTBOX_HEIGHT;
                }
                break;
            /* 左右区域按下，TOUCHDOWN可加快翻页响应 */
            case CapSense_ONE_FINGER_TOUCHDOWN:
                if (slider_position < 30 && page_cur > 1) /* 左边缘 */
                {
                    page_cur--;
                    rect.y0 += TEXTBOX_HEIGHT;
                }
                else if (slider_position > 70 && page_cur < pageNum) /* 右边缘 */
                {
                    page_cur++;
                    rect.y0 -= TEXTBOX_HEIGHT;
                }
                break;
            /* 中间区域双击并拖动，拖动中如果断触应停止修改，不保存高阻态后可以忽略 */
            case CapSense_TMG_CLICK_AND_DRAG:
                // if (slider_position != CapSense_SLIDER_NO_TOUCH) /* 防止断触时0XFFFF造成页码和文本框位置异常 */
                // {
                page_cur = slider_position * pageNum / (double)101 + 1; /* 按触摸位置等比例地更新页码 */
                rect.y0 = (1 - page_cur) * TEXTBOX_HEIGHT;              /* 根据当前页码更新文本框位置 */
                // }
                break;
            }
            switch (button_symbol) /* 检测按键消息 */
            {
            /* 按键0 */
            case 'C':
                switch (font_cur->YSize) /* 检测当前字号 */
                {
                case 16:
                    font_cur = &GUI_FontChinese_20;
                    break;
                case 20:
                    font_cur = &GUI_FontChinese_12;
                    break;
                case 12:
                    font_cur = &GUI_FontChinese_16;
                    break;
                }
                progress = (double)page_cur / pageNum; /* 保存当前阅读进度 */
                GUI_SetFont(font_cur);                 /* 由于GUI_WrapGetNumLines()返回值与字号有关，提前设置可防止页数错误 */
                pageNum = ceil(GUI_WrapGetNumLines(book->content, rect.x1, GUI_WRAPMODE_CHAR) / (double)(TEXTBOX_HEIGHT / font_cur->YDist));
                page_cur = ceil(progress * pageNum);       /* 按阅读进度更新当前页码 */
                rect.y0 = (1 - page_cur) * TEXTBOX_HEIGHT; /* 根据当前页码更新文本框位置 */
                break;
            /* 按键1 */
            case 'D':
                book->marker = page_cur; /* 保存书籍书签 */
                book->font = font_cur;   /* 保存书籍字体 */
                return;                  /* 返回书籍列表 */
            }
        } while (page_cur == book->marker && font_cur->YSize == book->font->YSize); /* 若页码和字号未改变则不刷新屏幕，避免高频刷新 */
    }
}

void TouchPBTask()
{
    uint8 count4samplButt = 0; /* 采样按键状态的时间计数器(20ms采样间隔/10ms时钟周期) */

    CapSense_dsRam.timestampInterval = GESTURE_TIME_INCREMENT; /* 用于flick动作速度的参数 */
    while (1)
    {
        /* 1. Button Module (firmware)组件处理（含机械和电容按键） ...... */
        /* 1.1 每隔50ms进行一次全部按键采样、按键动作识别、按键消息入队列 */
        if (count4samplButt == 0)
        {
            Button_Module_Handle();
            count4samplButt = 5; /* 50ms采样间隔/10ms时钟周期 */
        }
        count4samplButt--;
        /* 1.2 机械和电容按键按键消息获取与处理 */
        button_symbol = (QButton_FetchData() & MSG_BUTTON_SYM_MASK) >> MSG_BUTTON_SYM_MASK_SHIFTBITS;

        /* 2. CapSense处理与扫描 ...... */
        if (CapSense_IsBusy() == CapSense_NOT_BUSY) /* 仅当CapSense未在忙于扫描时 */
        {
            /* 处理各widgets，读取触摸信息 */
            CapSense_ProcessAllWidgets();
            /* 获取电容按键0触摸状态 */
            capBTN0_untouched = CapSense_IsWidgetActive(CapSense_BUTTON0_WDGT_ID) ? 0 : 1;
            /* 获取电容按键1触摸状态 */
            capBTN1_untouched = CapSense_IsWidgetActive(CapSense_BUTTON1_WDGT_ID) ? 0 : 1;
            /* 获取与处理电容滑条上的指势信息或触摸信息 */
            slider_gesture = CapSense_DecodeWidgetGestures(CapSense_LINEARSLIDER0_WDGT_ID);
            /* 当电容滑条触摸时（触摸位置：未触摸0xFFFF，触摸0-100） */
            if (CapSense_GetCentroidPos(CapSense_LINEARSLIDER0_WDGT_ID) != CapSense_SLIDER_NO_TOUCH)
            {
                slider_position = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0_WDGT_ID);
            }
            /* 开始下一次CapSense扫描 */
            CapSense_ScanAllWidgets();
        }
        /* 递增指势时间戳值 */
        CapSense_IncrementGestureTimestamp();
        /* ...... CapSense处理与扫描结束 */
        /* 延时10ms */
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
