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
    "���������ֲ����¼������������ڹܹ�",
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
    "��ս����",
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
    "��ը",
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
    "��ķɻ�",
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
    "�ڵ�",
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
    "���ǵл�",
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

void DrawMilk ();// ��������
void pushimage (Image Pika);//����obje
void flash(int x, int y);//�ƶ���꣬�൱��gotoxy
void KeyboardEvent (int key, int event);//��ȡ������Ϣ
void PaintBoard ();//����
void CreateBullet ();//�����ڵ������ҽ��и����������Ϊ�κ�����仯������൱��bullet�Ĳ������
void CreateEnemy ();//������ͨ����
void Timer (int TimeID);//��ʱ��
void CreateFighter ();//�����ƶ�����
void Begin ();//����
void Finish ();//����
void Lose ();//ʧ��

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
    SetConsoleCursorInfo (consoleHandle, &cci);//���ع��

    Begin ();
    situation = 1;//���Ŀǰ���ڽ׶Σ���Ϊacllib��û�н����������̵ĺ���
    oldchapter = 1;
    system ("cls");
    
    CreateEnemy ();
	CreateBullet ();//СС��˳��ͬ���ͻᵼ�³������

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
    printf("�ںڰ����޾���Ư��");
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
                        Board[i][j] = 2;//����һ�ּ������ӵ��Ƿ���
                    } else {
                        Boss_Blood -= 1000;
                        if (Boss_Blood <= 0) Finish ();
                    }
                }
            }
        }//��Ȿ�л��Ƿ����������ҿ����ڵ����ݣ�����bossѪ��
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
        }//������ըЧ���������ƶ��л������õл�
    }

    while (currentenemy->scaned) {
        currentenemy->scaned = 0;
        currentenemy = currentenemy->next;
    }//�ָ����״��

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
    }//��������

    while (currentbullet->scaned) {
        currentbullet->scaned = 0;
        currentbullet = currentbullet->next;
    }

    chapter = score / 500 + 1;
    if (score > 1500) {
        flash(Map_width + 4, Map_height - 1);
        printf("���Ѫ����%d", Blood);
        flash(Map_width + 4, Map_height);
        printf("�н�Ѫ����%d", Boss_Blood);
    } else {
        flash(Map_width + 4, Map_height - 1);
        printf("��ĵ÷֣�%d", score);
        flash(Map_width + 4, Map_height);
        printf("�����½ڣ�%d", chapter);
    }

    if (Blood == 0) Lose ();
}

void CreateBullet ()
{
    newbullet = (Image *) malloc (sizeof(Image));
    newbullet->x = Pplane.x + 2;
    newbullet->y = Pplane.y - 2;
    Board[newbullet->x][newbullet->y] = 1;//ֻ����ڵ�����Ϊ������λ���������ҷ��ڵ����ܱ�ը
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
    }//ѭ�����������ӵ�

    Pplane.speed = chapter * 2;//�ٶ�����

    if (chapter > oldchapter){
        refresh -= 150;
        cancelTimer (0);
        startTimer (0, refresh);
        oldchapter = chapter;
    } //�ڶ���

    if (chapter == 4) {
        currentenemy = &Boss;
        currentenemy->next = currentenemy;
        currentenemy->prev = currentenemy;
        Blood --;
        cancelTimer (1);//����timer���ڿ���
    }

    PaintBoard ();//Ƕ���ӵ�������飬һ������Ա�֤ͬ����һ����������һ��������

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
    } //��֤����image��������������
}

void DrawMilk ()
{
    for (int i = 0; i < Map_width; i += 4)
    {
        flash(i, 0);
        printf("����");
        flash(i, Map_height);
        printf("����");
    }
    for (int i = 0; i < Map_height; i += 2)
    {
        flash(0, i);
        printf("��");
        flash(0, i + 1);
        printf("��");
        flash(Map_width, i);
        printf("��");
        flash(Map_width, i + 1);
        printf("��");
    }
    flash(Map_width, Map_height);
    printf("��");
}

void Finish ()
{
    situation = 2;
    system ("cls");
    cancelTimer (0);
    
    system ("cls");
    flash (Map_width/2, Map_height/2);
    printf ("�����·��������չ");
    Sleep (80000);
}

void Lose ()
{
    situation = 2;
    system ("cls");
    cancelTimer (0);
    
    system ("cls");
    flash (Map_width/2, Map_height/2);
    printf ("Ҳ����滹��·");
    Sleep (80000);
}
