#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "acllib.h"

HANDLE consoleHandle;

int width, height;
int Blood = 100;
int Boss_Blood = 200000;
time_t last, current;
int refresh = 1000;
int situation = 0;
int score = 0;
int chapter = 0;
int oldchapter;

#define Map_width 80
#define Map_height 40

typedef struct image {
    int x;
    int y;
    int len;
    int wid;
    int speed;
    char shape[9][9];
    char *text;
    int scaned;
    struct image *next;
    struct image *prev;
} Image;

Image Boss = {
    35,
    3,
    5,
    5,
    8,
    {{0, 1, 1, 1, 0}, {1, 1, 1, 1, 1}, {1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}},
    "请想象贼恐怖可怕几乎吓死人炮炮管管",
    999,
    NULL,
    NULL
};

Image Fighter = {
    0,
    0,
    2,
    3,
    8,
    {{0, 1, 0}, {1, 1, 1}},
    "敌战斗机",
    0,
    NULL,
    NULL
};

Image Boom = {
    10,
    20,
    2,
    1,
    0,
    {{1}, {1}},
    "爆炸",
    0,
    NULL,
    NULL
};

Image Pplane = {
    Map_width / 2,
    Map_height - 2,
    2,
    3,
    1,
    {{0, 1, 0}, {1, 1, 1}},
    "你的飞机",
    1,
    NULL,
    NULL
};

Image Pbullet = {
    Map_width / 2 + 2,
    Map_height - 2 - 2,
    2,
    1,
    4,
    {{1}, {1}},
    "炮弹",
    0,
    NULL,
    NULL
};

Image Enemy = {
    10,
    5,
    2,
    3,
    0,
    {{1, 1, 1}, {0, 1, 0}},
    "这是敌机",
    2,
    NULL,
    NULL
};

int Board[Map_width][Map_height];

Image *newbullet = NULL;
Image *prevbullet = NULL;
Image *newenemy = NULL;
Image *prevenemy = NULL;
Image *currentbullet = &Pbullet;
Image *currentenemy = &Enemy;

void DrawMilk ();// 绘制银河
void pushimage (Image Pika);//放置obje
void flash(int x, int y);//移动光标，相当于gotoxy
void KeyboardEvent (int key, int event);//读取键盘消息
void PaintBoard ();//绘制
void CreateBullet ();//制作炮弹，并且进行各项操作（因为任何事物变化间隔都相当于bullet的产生间隔
void CreateEnemy ();//制作普通敌人
void Timer (int TimeID);//计时器
void CreateFighter ();//制作移动敌人
void Begin ();//开局
void Finish ();//结束
void Lose ();//失败

int Setup ()
{
    initWindow ("Plane Biu Biu Biu", DEFAULT, DEFAULT, 50, 50);
	initConsole ();
    srand ((unsigned) time (NULL));
    // system("color 5F");

    for (int i = 0; i < Map_width; i++) {
        for (int j = 0; j < Map_height; j++)
            Board[i][j] = 0;
    }

    consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 100;
    cci.bVisible = FALSE;
    SetConsoleCursorInfo (consoleHandle, &cci);//隐藏光标

    Begin ();
    situation = 1;//标记目前所在阶段，因为acllib并没有结束聆听键盘的函数
    oldchapter = 1;
    system ("cls");
    
    CreateEnemy ();
	CreateBullet ();//小小的顺序不同，就会导致程序崩溃

    registerTimerEvent (Timer);
    startTimer (0, refresh);
    startTimer (1, 5000);
    
	registerKeyboardEvent (KeyboardEvent);

    // cci.bVisible = TRUE;
    // SetConsoleCursorInfo (consoleHandle, &cci);

	return 0;
}

void Begin ()
{
    situation = 0;
    flash (Map_width / 2, Map_height / 2);
    printf("在黑暗中无尽的漂流");
    Sleep (5000);
}

void Timer (int TimeID)
{
    switch (TimeID) {
        case 0 : 
            CreateBullet (); 
            break;
        case 1 : 
            CreateEnemy ();
            break;
        case 2:
            CreateFighter ();
            break;
        default:
            break;
    }
}

void KeyboardEvent (int key, int event)
{
	switch (key) {
		case VK_UP : 
			if (event == KEY_DOWN && Pplane.y - 1 > 1) {}Pplane.y -= Pplane.speed; break;
		case VK_DOWN : 
			if (event == KEY_DOWN && Pplane.y + 1 < Map_height - 2) Pplane.y += Pplane.speed; break;
		case VK_LEFT : 
			if (event == KEY_DOWN && Pplane.x - 1 > 1) Pplane.x -= Pplane.speed*2; break;
		case VK_RIGHT : 
			if (event == KEY_DOWN && Pplane.x + 1 < Map_width - 3) Pplane.x += Pplane.speed*2; break;
		default :
			break;
	}
    
	if (situation == 1)PaintBoard ();
}

void PaintBoard ()
{
	system("cls");
    DrawMilk();
    pushimage (Pplane);

    while (!currentenemy->scaned) {
        int live = 1;
        for (int i = currentenemy->x; i < currentenemy->x + currentenemy->wid * 2; i++) {
            for (int j = currentenemy->y; j < currentenemy->y +currentenemy->len; j++){
                if (Board[i][j] == 1) {
                    if (currentenemy->len != 5) {
                        live = 0;
                        Board[i][j] = 2;//让下一轮检测这个子弹是否存活
                    } else {
                        Boss_Blood -= 1000;
                        if (Boss_Blood <= 0) Finish ();
                    }
                }
            }
        }//检测本敌机是否死亡，并且控制炮弹数据，减少boss血量
        if (live == 1) {
            if (currentenemy->speed) {
                int move = (rand() % 3 - 1) * Fighter.speed;
                if (currentenemy->x + move < Map_width - 6 && currentenemy->x + move > 0)
                    currentenemy->x += move;
            }
            pushimage (*currentenemy);
            currentenemy->scaned = 1;
            currentenemy = currentenemy->next;
        } else {
            Boom.x = currentenemy->x + 1;
            Boom.y = currentenemy->y;
            pushimage (Boom);
            currentenemy->prev->next = currentenemy->next;
            currentenemy->next->prev = currentenemy->prev;
        }//制作爆炸效果，并且移动敌机，放置敌机
    }

    while (currentenemy->scaned) {
        currentenemy->scaned = 0;
        currentenemy = currentenemy->next;
    }//恢复检查状况

    while (!currentbullet->scaned) {
        if (Board[currentbullet->x][currentbullet->y] != 2)
            pushimage (*currentbullet);
        else {
            Boom.x = currentbullet->x;
            Boom.y = currentbullet->y;
            pushimage (Boom);
            score += 10;
        }
        currentbullet->scaned = 1;
        currentbullet = currentbullet->next;
    }//分数控制

    while (currentbullet->scaned) {
        currentbullet->scaned = 0;
        currentbullet = currentbullet->next;
    }

    chapter = score / 500 + 1;
    if (score > 1500) {
        flash(Map_width + 4, Map_height - 1);
        printf("你的血量：%d", Blood);
        flash(Map_width + 4, Map_height);
        printf("敌舰血量：%d", Boss_Blood);
    } else {
        flash(Map_width + 4, Map_height - 1);
        printf("你的得分：%d", score);
        flash(Map_width + 4, Map_height);
        printf("所在章节：%d", chapter);
    }

    if (Blood == 0) Lose ();
}

void CreateBullet ()
{
    newbullet = (Image *) malloc (sizeof(Image));
    newbullet->x = Pplane.x + 2;
    newbullet->y = Pplane.y - 2;
    Board[newbullet->x][newbullet->y] = 1;//只标记炮弹，因为其他单位都是遇到我方炮弹才能爆炸
    Board[newbullet->x][newbullet->y + 1] = 1;
    Board[newbullet->x+1][newbullet->y] = 1;
    Board[newbullet->x+1][newbullet->y + 1] = 1;
    Pbullet.x = Pplane.x + 2;
    Pbullet.y = Pplane.y - 2;
    newbullet->len = Pbullet.len;
    newbullet->shape[0][0] = 1;
    newbullet->shape[1][0] = 1;
    newbullet->text = Pbullet.text;
    newbullet->wid = Pbullet.wid;
    newbullet->speed = Pbullet.speed;
    newbullet->scaned = Pbullet.scaned;
    if (Pbullet.next == NULL) {
        Pbullet.next = newbullet;
        Pbullet.prev = newbullet;
        newbullet->next = &Pbullet;
        newbullet->prev = &Pbullet;
    } else {
        newbullet->prev = Pbullet.prev;
        newbullet->next = &Pbullet;
        Pbullet.prev->next = newbullet;
        Pbullet.prev = newbullet;
    }//循环链表制作子弹

    Pplane.speed = chapter * 2;//速度提升

    if (chapter > oldchapter){
        refresh -= 150;
        cancelTimer (0);
        startTimer (0, refresh);
        oldchapter = chapter;
    } //第二章

    if (chapter == 4) {
        currentenemy = &Boss;
        currentenemy->next = currentenemy;
        currentenemy->prev = currentenemy;
        Blood --;
        cancelTimer (1);//这种timer便于控制
    }

    PaintBoard ();//嵌入子弹产生板块，一方面可以保证同步，一方面少用了一个计数器

    while (!currentbullet->scaned) {
        if (currentbullet->y - currentbullet->speed < 1 ||
            Board[currentbullet->x][currentbullet->y] == 2) {
            Board[currentbullet->x][currentbullet->y] = 0;
            Board[currentbullet->x][currentbullet->y + 1] = 0;
            Board[currentbullet->x+1][currentbullet->y] = 0;
            Board[currentbullet->x+1][currentbullet->y + 1] = 0;
            Image *badbullet = currentbullet;
            currentbullet->prev->next = currentbullet->next;
            currentbullet->next->prev = currentbullet->prev;
            currentbullet = currentbullet->next;
            free (badbullet);
        }else {
            Board[currentbullet->x][currentbullet->y] = 0;
            Board[currentbullet->x][currentbullet->y + 1] = 0;
            Board[currentbullet->x+1][currentbullet->y] = 0;
            Board[currentbullet->x+1][currentbullet->y + 1] = 0;
            currentbullet->y -= currentbullet->speed;
            Board[currentbullet->x][currentbullet->y] = 1;
            Board[currentbullet->x][currentbullet->y + 1] = 1;
            Board[currentbullet->x+1][currentbullet->y] = 1;
            Board[currentbullet->x+1][currentbullet->y + 1] = 1;
        }
        currentbullet->scaned = 1;
        currentbullet = currentbullet->next;
    }

    while (currentbullet->scaned) {
        currentbullet->scaned = 0;
        currentbullet = currentbullet->next;
    }
}

void CreateEnemy ()
{
    if (chapter > 1) CreateFighter ();
    newenemy = (Image *) malloc (sizeof(Image));
    if (currentenemy == NULL)
        currentenemy = newenemy;
    newenemy->x = rand () % 76 + 2;
    newenemy->y = rand () % 10 + 1;
    newenemy->len = Enemy.len;
    for (int i = 0; i < Enemy.len; i++) {
        for (int j = 0; j < Enemy.wid; j++) {
            newenemy->shape[i][j] = Enemy.shape[i][j];
        }
    }
    newenemy->text = Enemy.text;
    newenemy->wid = Enemy.wid;
    newenemy->speed = Enemy.speed;
    newenemy->scaned = 0;
    if (currentenemy->next == NULL) {
        currentenemy->next = newenemy;
        currentenemy->prev = newenemy;
        newenemy->next = currentenemy;
        newenemy->prev = currentenemy;
    } else {
        newenemy->next = currentenemy->next;
        newenemy->prev = currentenemy;
        currentenemy->next->prev = newenemy;
        currentenemy->next = newenemy;
    }
}

void CreateFighter ()
{
    newenemy = (Image *) malloc (sizeof(Image));
    if (currentenemy == NULL)
        currentenemy = newenemy;
    newenemy->x = rand () % 76 + 2;
    newenemy->y = rand () % 10 + 1;
    newenemy->len = Fighter.len;
    for (int i = 0; i < Fighter.len; i++) {
        for (int j = 0; j < Fighter.wid; j++) {
            newenemy->shape[i][j] = Fighter.shape[i][j];
        }
    }
    newenemy->text = Fighter.text;
    newenemy->wid = Fighter.wid;
    newenemy->speed = Fighter.speed;
    newenemy->scaned = 0;
    if (currentenemy->next == NULL) {
        currentenemy->next = newenemy;
        currentenemy->prev = newenemy;
        newenemy->next = currentenemy;
        newenemy->prev = currentenemy;
    } else {
        newenemy->next = currentenemy->next;
        newenemy->prev = currentenemy;
        currentenemy->next->prev = newenemy;
        currentenemy->next = newenemy;
    }
}

void flash(int x, int y)
{
    COORD coord = {.X = x, .Y = y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void pushimage (Image Pika)
{
    int toal = 0;

    COORD co = (COORD){ .X = Pika.x, .Y = Pika.y };
    SetConsoleCursorPosition (consoleHandle, co);

    for (int i = 0; i < Pika.len; i++){
        flash (Pika.x, Pika.y + i);
        for (int j = 0; j < Pika.wid; j++){
            if (Pika.shape[i][j]){
                printf("%c%c", Pika.text[toal], Pika.text[toal+1]);
                toal += 2;
            }
            else{
                printf("  ");
            }
        }
    } //保证所有image都可以正常放置
}

void DrawMilk ()
{
    for (int i = 0; i < Map_width; i += 4)
    {
        flash(i, 0);
        printf("银河");
        flash(i, Map_height);
        printf("银河");
    }
    for (int i = 0; i < Map_height; i += 2)
    {
        flash(0, i);
        printf("银");
        flash(0, i + 1);
        printf("河");
        flash(Map_width, i);
        printf("银");
        flash(Map_width, i + 1);
        printf("河");
    }
    flash(Map_width, Map_height);
    printf("银");
}

void Finish ()
{
    situation = 2;
    system ("cls");
    cancelTimer (0);
    
    system ("cls");
    flash (Map_width/2, Map_height/2);
    printf ("后面的路，即将铺展");
    Sleep (80000);
}

void Lose ()
{
    situation = 2;
    system ("cls");
    cancelTimer (0);
    
    system ("cls");
    flash (Map_width/2, Map_height/2);
    printf ("也许后面还有路");
    Sleep (80000);
}
