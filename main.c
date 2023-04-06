#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define SCREEN_HEIGHT 10 // �ù�����
#define SCREEN_WIDTH 50  // �ù�����
#define DINO_PLACE 6     // �p���s�b�����W���ĴX��
#define ROAD_PLACE 6     // ���b�ù����ĴX��
#define CENTER 25        // �ù�������m
#define DINO 'F'         // dino �˦�

struct dino
{
    /* screen */
    char screen[SCREEN_HEIGHT][SCREEN_WIDTH + 1]; // �C���ù��A�r�굲���ݭn'\0'
    int rgb[SCREEN_HEIGHT][SCREEN_WIDTH + 1];     // �C���ù��C��
    /* road */
    char road[SCREEN_WIDTH + 1]; // �����A�r�굲���ݭn'\0'
    int road_count;              // �ͦ���ê�������j�p��
    int barrier_chance;          // ��m��ê�������v
    /* ���a�S�� */
    int wait_time;                                    // �C���j�餧�����ݪ��ɶ�
    char meteorite[ROAD_PLACE + 1][SCREEN_WIDTH + 1]; // �k�۰Ϫ��e���A��0������ܿù��Ĥ@��A�̫�@���x�s�z���ɶ�
    int frame_count;                                  // �{�{�Ϫ����j�p��
    /* �ޯ� */
    int jump;   // �ٳѦh�[���^�a��
    int cannon; // �j���Ѿl�ɶ�
    int bomb;   // ���u�Ѿl�ɶ�
    int laser;  // laser �N�o�ɶ�
    /* ��T */
    int score;         // ����
    int highest_score; // ���v�̰���
    int money;         // ��
    int kb_key;        // �ϥΪ̫��F�ƻ���
    /* �ө� */
    int color[5]; // �}���C��
    int cost[5];  // �ө���O
    /* dino */
    int dino_color; // �p���s�{�b�C��
};
typedef struct dino Dino;
Dino *game; // game object

// detect what user typed, if nothing, return -1
int keyboard(void)
{
    if (_kbhit() != 0)
        return _getch();
    else
        return -1;
}

// goto console (x, y)
void gotoxy(short x, short y)
{
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// print the screen arrays into console
void display()
{
    gotoxy(0, 0);

    setvbuf(stdout, NULL, _IOFBF, 1024); // use buffer to reduce the times of io

    // print the screen arrays
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            char line[20];
            sprintf(line, "\033[%dm", game->rgb[i][j]);
            fprintf(stdout, "%s", line);
            fprintf(stdout, "%c", game->screen[i][j]);
            fprintf(stdout, "\033[0m", line);
        }
        fprintf(stdout, "\n");
    }

    // print buffer to the console
    fflush(stdout);
}

// clear screen arrays and fill rgb arrays with color
void cls_color(int color)
{
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            game->screen[i][j] = ' ';
            game->rgb[i][j] = color;
        }

        game->screen[i][SCREEN_WIDTH] = '\0';
    }
}

// clear screen arrays and rgb arrays
void cls()
{
    cls_color(0);
}

// add string into screen arrays with color
void print_color(int x, int y, char *str, int color)
{
    int len = strlen(str);
    for (int i = 0; i < len && i < SCREEN_WIDTH; i++)
    {
        game->screen[y][x + i] = str[i];
        game->rgb[y][x + i] = color;
    }
}

// add string into screen arrays
void print(int x, int y, char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len && i < SCREEN_WIDTH; i++)
    {
        game->screen[y][x + i] = str[i];
    }
}

// add string into the center of screen arrays with color
void print_center_color(int y, char *str, int color)
{
    print_color(CENTER - strlen(str) / 2, y, str, color);
}

// add string into the center of screen arrays with color
void print_center(int y, char *str)
{
    print(CENTER - strlen(str) / 2, y, str);
}

// save user data
void saveData()
{
    FILE *fptr = fopen("data", "w");
    if (fptr)
    {
        fprintf(fptr, "%d\n", game->highest_score);
        fprintf(fptr, "%d\n", game->money);
        fprintf(fptr, "%d\n", game->dino_color);
        fprintf(fptr, "%d %d %d %d %d\n", game->cost[0], game->cost[1], game->cost[2], game->cost[3], game->cost[4]);

        fclose(fptr);
    }
}

// read data, if null then save file
void readData()
{
    FILE *fptr = fopen("data", "r");
    if (fptr)
    {
        fscanf(fptr, "%d", &(game->highest_score));
        fscanf(fptr, "%d", &(game->money));
        fscanf(fptr, "%d", &(game->dino_color));
        fscanf(fptr, "%d %d %d %d %d", &(game->cost[0]), &(game->cost[1]), &(game->cost[2]), &(game->cost[3]), &(game->cost[4]));
        fclose(fptr);
    }
    else
    {
        game->highest_score = 0;
        game->money = 0;
        game->dino_color = 0;
        game->cost[0] = 0;
        game->cost[1] = 100000;
        game->cost[2] = 500000;
        game->cost[3] = 1000000;
        game->cost[4] = 10000000;

        saveData();
    }
}

// init game settings
void init(void)
{
    // ���ù���
    CONSOLE_CURSOR_INFO cursor;
    cursor.bVisible = false;
    cursor.dwSize = sizeof(cursor);
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorInfo(handle, &cursor);

    // init game and read data
    srand(time(NULL));
    game = (Dino *)malloc(sizeof(Dino));
    if (game != NULL)
    {
        readData();

        game->color[0] = 0;
        game->color[1] = 34;
        game->color[2] = 35;
        game->color[3] = 36;
        game->color[4] = 96;

        cls(); // clear screen and rgb
    }
}

// store
void store()
{
    int choice = 0;  // choice of product
    int kb_key = -1; // detect what user typed

    do
    {
        // detect what user typed

        // esc return
        if (kb_key == 27)
        {
            return;
        }

        // move left
        if (kb_key == 75)
        {
            choice--;
        }
        // move right
        else if (kb_key == 77)
        {
            choice++;
        }

        // �����Ӱӫ~�A����W�X�d��
        if (choice < 0)
        {
            choice = 0;
        }
        else if (choice > 4)
        {
            choice = 4;
        }

        // if typed space, then buy it
        if (kb_key == 32)
        {
            // something that haven't buy yet
            if (game->cost[choice] != 0)
            {
                // do not have enough money to buy
                if (game->money < game->cost[choice])
                {
                    print_center_color(5, "Do not have enough money", 31);
                    display();

                    Sleep(1000);
                }
                // buy it and change dino color
                else
                {
                    game->money -= game->cost[choice];
                    game->cost[choice] = 0;
                    game->dino_color = choice;

                    saveData();
                }
            }
            // something that already buy, change color to it
            else
            {
                game->dino_color = choice;

                saveData();
            }
        }

        // redresh the screen
        cls();

        // �L�[�l
        print_center(0, "Store");
        print_center(2, "|--------|--------|--------|--------|--------|");
        print_center(3, "|        |        |        |        |        |");
        print_center(4, "|        |        |        |        |        |");
        print_center(5, "|--------|--------|--------|--------|--------|");
        print_center(6, "|        |        |        |        |        |");
        print_center(7, "|--------|--------|--------|--------|--------|");

        // �L��ܬ[�l
        char selected[6][11] = {
            "|--------|",
            "|        |",
            "|        |",
            "|--------|",
            "|        |",
            "|--------|"};
        for (int i = 0; i < 6; i++)
        {
            print_color(9 * choice + 2, i + 2, selected[i], 34);
        }

        // �L�p���s�M����
        char line[20];
        for (int i = 0; i < 5; i++)
        {
            print_color(9 * i + 6, 3, "F", game->color[i]);

            if (game->cost[i] != 0)
            {
                sprintf(line, "%d", game->cost[i]);
            }
            else if (i == game->dino_color)
            {
                sprintf(line, "using");
            }
            else
            {
                sprintf(line, "use");
            }
            print_color(9 * i + 3, 6, line, 0);
        }

        // �L�Ѿl�h�ֿ�
        sprintf(line, "Money: %d", game->money);
        print_center(8, line);
        print_center(9, "press space to buy items and change style");

        // display to the screen
        display();
    } while (kb_key = _getch());
}

// �}�l�C�����˭p��
void timer()
{
    char word[3][2] = {
        "1", "2", "3"};
    for (int i = 3; i > 0; i--)
    {
        cls();
        print_center(5, word[i - 1]);
        display();

        Sleep(1000);
    }
}

// �}�Y
void opening(void)
{
    int frameCount = 0; // �p��{�{���j
    int choice = 1;     // ��� store �� start
    int kb_key;         // detect what user typed

    // refresh screen every 100ms
    while (true)
    {
        // key events
        kb_key = keyboard();
        // space key event
        if (kb_key == 32)
        {
            // go to store
            if (choice == 0)
            {
                store();
            }
            // start the game
            else
            {
                timer();
                return;
            }
        }
        // down
        if (kb_key == 80)
        {
            choice = 1;
        }
        // up
        if (kb_key == 72)
        {
            choice = 0;
        }

        cls();
        print_center(2, "CRAZY DINO"); // print title
        // print menu
        if (choice == 0)
        {
            print_center_color(4, "STORE", 34);
            print_center(5, "START");
        }
        else
        {
            print_center(4, "STORE");
            print_center_color(5, "START", 34);
        }

        // �{�{�ĪG
        if (frameCount % 13 < 8)
        {
            print_center(7, "Press space to continue");
        }
        frameCount++;
        if (frameCount == 13)
        {
            frameCount = 0;
        }

        display();
        Sleep(100);
    }
}

// game over
void ending(void)
{
    // change money and save
    game->money += game->score;
    if (game->score > game->highest_score)
    {
        game->highest_score = game->score;
    }
    saveData();

    // display
    cls_color(31); // ��ܬ���

    print_center(2, "GAME OVER"); // print title

    print(0, 6, game->road); // print road

    // print highest score and score
    char line[SCREEN_WIDTH + 1];
    sprintf(line, "HIGHEST SCORE %d", game->highest_score);
    print_center(8, line);

    sprintf(line, "SCORE %d", game->score);
    print_center(9, line);

    display();

    // wait for space
    while (_getch() != 32)
        ;
}

// init game
void initDino(void)
{
    if (game != NULL)
    {
        game->score = 0;
        // game->score = 4000; // debugging
        game->jump = 0;
        game->wait_time = 100;
        game->barrier_chance = 10;
        game->road_count = 0;
        game->frame_count = 0;
        game->cannon = 0;
        game->bomb = 0;
        game->laser = 0;

        // clear screen and rgb
        cls();

        // clear meteorite
        for (int i = 0; i < ROAD_PLACE; i++)
        {
            for (int j = 0; j < SCREEN_WIDTH; j++)
            {
                game->meteorite[i][j] = ' ';
            }

            game->meteorite[i][SCREEN_WIDTH] = '\0';
        }
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            game->meteorite[ROAD_PLACE][j] = 0;
        }
        game->meteorite[ROAD_PLACE][SCREEN_WIDTH] = '\0';

        // generate road
        for (int i = 0; i < SCREEN_WIDTH; i++)
        {
            game->road[i] = '_';
        }
        game->road[SCREEN_WIDTH] = '\0';
    }
}

// jump and drop
void jump(void)
{
    // jump, space key
    if (game->jump == 0 && game->kb_key == 32)
    {
        game->jump = 7;
    }
    // drop
    if (game->jump > 0)
    {
        game->jump--;
    }
}

// quickly jump down
void jump_down(void)
{
    // down key
    if (game->kb_key == 80)
    {
        game->jump = 0;
    }
}

// debugging
void auto_jump(void)
{
    if (game->kb_key == 'p')
    {
        return;
    }
    else
    {
        if (game->road[DINO_PLACE + 1] != '_')
        {
            game->kb_key = 32;
        }
        else if (game->road[DINO_PLACE] == '_')
        {
            game->kb_key = 80;
        }
    }
}

// cannon
void cannon()
{
    // 60 ~ 50 �M���e���ê��
    // 50 ~ 0 �N�o
    if (game->cannon == 0 && game->kb_key == 'c')
    {
        game->cannon = 611;
    }

    if (game->cannon != 0)
    {
        game->cannon--;
    }
}

// bomb
void bomb()
{
    // 16~3 �y��
    // 3~0 �z��
    if (game->bomb == 0 && game->kb_key == 'b')
    {
        game->bomb = 17;
    }

    if (game->bomb != 0)
    {
        game->bomb--;
    }
}

// laser
void laser()
{
    if (game->kb_key == 'l' && game->laser == 0)
    {
        game->laser = 601;
    }

    // 600 �N�o�ɶ�
    if (game->laser > 0)
    {
        game->laser--;
    }
}

// skills
void skills()
{
    game->kb_key = keyboard();
    cannon();
    bomb();
    laser();
    // auto_jump();
    jump();
    jump_down();
}

/* render skills */
void render_road()
{
    // move road
    for (int i = 0; i < SCREEN_WIDTH - 1; i++)
    {
        game->road[i] = game->road[i + 1];
    }
    // spawn barrier
    int random_number = rand();
    if (game->road_count > 8 && game->barrier_chance > rand() % 100)
    {
        game->road[SCREEN_WIDTH - 1] = 'X';
        game->road_count = 0;
    }
    else
    {
        game->road[SCREEN_WIDTH - 1] = '_';
        game->road_count++;
    }
    game->road[SCREEN_WIDTH] = '\0';

    // render to screen
    strcpy(&game->screen[ROAD_PLACE], game->road);
}

void render_cannon()
{
    // 650 ~ 600 �M���e���ê��
    // 50 ~ 0 �N�o
    for (int i = DINO_PLACE + 1; i < SCREEN_WIDTH; i++)
    {
        game->screen[ROAD_PLACE][i] = '=';
        game->rgb[ROAD_PLACE][i] = 33;
        game->road[i] = '_';
    }
}

void render_bomb()
{
    // 16~3 �y��
    // 3~0 �z��
    // ���p���s�e��11~15��

    // �W��
    if (game->bomb >= 9.5 && game->bomb < 16)
    {
        int t = 16 - game->bomb;
        double height = t - 1.5 / 6.5 / 6.5 * t * t;
        game->screen[ROAD_PLACE - (int)height][DINO_PLACE + 16 - game->bomb] = '.';
    }
    // �U��
    else if (game->bomb > 3 && game->bomb <= 9.5)
    {
        int t = game->bomb - 3;

        double height = t - 1.5 / 6.5 / 6.5 * t * t;
        game->screen[ROAD_PLACE - (int)height][DINO_PLACE + 16 - game->bomb] = '.';
    }
    // �z��
    else
    {
        // �a�O�ܬ�
        if (game->bomb == 3)
        {
            char explosion[] = "___";
            for (int i = 12; i < 15; i++)
            {
                game->rgb[ROAD_PLACE][DINO_PLACE + i] = 31;
                game->screen[ROAD_PLACE][DINO_PLACE + i] = explosion[i - 12];

                if (game->road[DINO_PLACE + i] == 'X')
                {
                    game->road[DINO_PLACE + i] = '_';
                }
            }
        }
        // ����S��
        if (game->bomb == 2)
        {
            char explosion[] = "\\___/";
            for (int i = 11; i < 16; i++)
            {
                game->rgb[ROAD_PLACE][DINO_PLACE + i] = 31;
                game->screen[ROAD_PLACE][DINO_PLACE + i] = explosion[i - 11];

                if (game->road[DINO_PLACE + i] == 'X')
                {
                    game->road[DINO_PLACE + i] = '_';
                }
            }
        }
    }
}

void render_laser()
{
    int laser[SCREEN_WIDTH - DINO_PLACE] = {0};

    // generate laser
    for (int i = 1; i < SCREEN_WIDTH - DINO_PLACE; i++)
    {
        int random_number = rand() % 2;
        if (random_number == 0)
        {
            random_number = -1;
        }

        if (laser[i - 1] + random_number == -1)
        {
            random_number = 1;
        }
        if (laser[i - 1] + random_number == 6)
        {
            random_number = -1;
        }

        laser[i] = laser[i - 1] + random_number;
    }

    // display laser
    for (int i = 1; i < SCREEN_WIDTH - DINO_PLACE; i++)
    {
        if (laser[i] - laser[i - 1] > 0)
        {
            game->screen[ROAD_PLACE - laser[i] + 1][DINO_PLACE + i] = '/';
            if (laser[i] == 1 && game->road[DINO_PLACE + i] == 'X')
            {
                game->road[DINO_PLACE + i] = '_';
            }
        }
        else
        {
            game->screen[ROAD_PLACE - laser[i]][DINO_PLACE + i] = '\\';
            if (laser[i] == 0 && game->road[DINO_PLACE + i] == 'X')
            {
                game->road[DINO_PLACE + i] = '_';
            }
        }
        display();
        Sleep(10);
    }

    Sleep(400);
}

void render_dino()
{
    int y = ROAD_PLACE;
    if (game->jump > 0)
    {
        y--;
    }

    game->screen[y][DINO_PLACE] = DINO;
    game->rgb[y][DINO_PLACE] = game->color[game->dino_color];
}

void render_map()
{
    // �s���
    if (game->score <= 500)
    {
        print_center(0, "Newbie Village");
    }
    // �[�t��
    else if (game->score > 500 && game->score <= 1000)
    {
        game->wait_time = 100 - (game->score - 500) / 500.0 * 50;
        print_center(0, "Accel World");
    }
    // �����B�װ�
    else if (game->score > 1000 && game->score <= 2000)
    {
        // cloud
        print(DINO_PLACE + 8, ROAD_PLACE - 4, "           __   _");
        print(DINO_PLACE + 8, ROAD_PLACE - 3, "    __   _(  )_( )          __   _");
        print(DINO_PLACE + 8, ROAD_PLACE - 2, "  _(  )_(_   _    _)  _   _(  )_( )_");
        print(DINO_PLACE + 8, ROAD_PLACE - 1, " (_   _    _)  _(  )_( )_(_   _    _)");
        print(DINO_PLACE + 8, ROAD_PLACE, "   (_) (__)   (_   _    _) (_) (__)");

        print_center(0, "Flying over the Mist");
    }
    // �{�{��
    else if (game->score > 2000 && game->score <= 3000)
    {
        // 0~4�����
        // 5~9���
        game->frame_count++;
        if (game->frame_count % 10 < 5)
        {
            cls();
        }
        else if (game->frame_count % 10 == 9)
        {
            game->frame_count = -1;
        }

        print_center(0, "Flash Land");
    }
    // �k�۰�
    else if (game->score > 3000)
    {
        // �V�k�U���ʹk��
        for (int i = ROAD_PLACE - 1; i > 0; i--)
        {
            for (int j = SCREEN_WIDTH - 1; j > 0; j--)
            {
                if (i != 0 && j != 0)
                {
                    game->meteorite[i][j] = game->meteorite[i - 1][j - 1];
                }
            }
        }

        // �a�O
        for (int j = 0; j < SCREEN_WIDTH; j++)
        {
            // �V������
            game->meteorite[ROAD_PLACE][j] = game->meteorite[ROAD_PLACE][j + 1];

            // ��s�z�����A
            if (game->meteorite[ROAD_PLACE][j] > 0)
            {
                game->meteorite[ROAD_PLACE][j]--;
            }

            // �z���S��
            if (game->meteorite[ROAD_PLACE][j] != 0)
            {
                // 3 �a�O�ܬ�
                if (game->meteorite[ROAD_PLACE][j] >= 3)
                {
                    game->rgb[ROAD_PLACE][j] = 31;
                }
                // 2 �z��
                else if (game->meteorite[ROAD_PLACE][j] >= 2)
                {
                    game->rgb[ROAD_PLACE][j - 1] = 31;
                    game->rgb[ROAD_PLACE][j] = 31;
                    game->rgb[ROAD_PLACE][j + 1] = 31;

                    game->screen[ROAD_PLACE][j - 1] = '\\';
                    game->screen[ROAD_PLACE][j + 1] = '/';
                }
                // 1 �M�Ŧa�O
                else if (game->meteorite[ROAD_PLACE][j] == 1)
                {
                    game->screen[ROAD_PLACE][j - 1] = ' ';
                    game->screen[ROAD_PLACE][j] = ' ';
                    game->screen[ROAD_PLACE][j + 1] = ' ';
                    game->road[j - 1] = ' ';
                    game->road[j] = ' ';
                    game->road[j + 1] = ' ';
                }
            }
        }
        game->meteorite[ROAD_PLACE][SCREEN_WIDTH] = 0;

        // generate rock
        int i = rand() % 100;
        if (i < 50)
        {
            int y = rand() % 3 + 1, x = rand() % (SCREEN_WIDTH - 17) + 17, flag = 0;
            // �P�_�P��O�_���y�P
            for (int j = x - 15; j <= x + 15 && !flag; j++)
            {
                for (int k = 1; k < ROAD_PLACE; k++)
                {
                    if (game->meteorite[k][j] == '\\')
                    {
                        flag = 1;
                        break;
                    }
                }
            }

            // �P�_�P��O�_�Q���L
            for (int j = x - 15; j <= x + 15 && !flag; j++)
            {
                if (game->road[j] == ' ' || game->meteorite[ROAD_PLACE][j] != 0)
                {
                    flag = 1;
                }
            }

            if (flag == 0)
            {
                game->meteorite[y][x] = '\\';
            }
        }

        // render to screen, and determine if it is on the floor
        for (int i = 1; i <= ROAD_PLACE; i++)
        {
            for (int j = 0; j < SCREEN_WIDTH; j++)
            {
                if (game->meteorite[i - 1][j] == '\\')
                {
                    game->screen[i][j] = game->meteorite[i - 1][j];

                    // �b�a�O�W���ܴN����z���S��
                    if (i == ROAD_PLACE)
                    {
                        game->road[j] = '_';
                        game->meteorite[ROAD_PLACE][j] = 4;
                    }
                }
            }
        }

        print_center(0, "Meteor Zone");
    }
}

void render_info()
{
    char line[SCREEN_WIDTH + 1];
    // score
    sprintf(line, "Score: %d", game->score);
    print_center(8, line);

    // skills
    sprintf(line, "Skills: ");
    int skills[] = {
        game->cannon,
        game->bomb,
        game->jump,
        game->laser};
    char skill_name[] = "cbjl";
    for (int i = 0; i < 4; i++)
    {
        line[7 + 2 * i] = ' ';
        if (skills[i] == 0)
        {
            line[8 + 2 * i] = skill_name[i];
        }
        else
        {
            line[8 + 2 * i] = ' ';
        }
        line[9 + 2 * i] = '\0';
    }
    print_center(9, line);
}

// render to screen
void render(void)
{
    cls();

    /* road */
    render_road();

    /* skills */
    // cannon
    if (game->cannon > 600)
    {
        render_cannon();
    }

    // bomb
    if (game->bomb > 0)
    {
        render_bomb();
    }

    // laser
    if (game->laser == 600)
    {
        render_laser();
    }

    /* ���a�S�� */
    render_map();

    // dino
    render_dino();

    /* ��T */
    render_info();

    display();
}

// �P�_�p�s����m�O���O�a�O
bool is_bumped(void)
{
    return game->jump == 0 && game->road[DINO_PLACE] != '_';
}

// main function
int main()
{
    init(); // init game settings

    while (true)
    {
        opening(); // �}�Y

        initDino(); // ��l��game

        // game loop
        while (true)
        {
            skills(); // trigger skills

            // render game
            render();

            // �P�_�����F�S
            if (is_bumped())
            {
                break;
            }

            // add score
            game->score++;

            // wait
            Sleep(game->wait_time);
        }

        ending(); // game over
    }

    free(game);

    return 0;
}
