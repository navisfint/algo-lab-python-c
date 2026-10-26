/* ============================================================================
 * 实验三 · 蒙提霍尔悖论（Monty Hall Problem）—— C 语言对应实现
 * ----------------------------------------------------------------------------
 * 规则：3 道门，1 道后是汽车、2 道后是山羊。玩家先选 1 道 →
 *       主持人知道门后是什么，故意打开另外两扇中一扇后面是山羊的门 →
 *       问玩家"要不要改选剩下的那扇门" → 揭晓输赢。
 *
 * 与 Python 版对照：
 *   Python 用 dict{门号: 奖品} + 集合差集求"主持人可开的门" + try/except 拦非法输入；
 *   C 没有字典和集合，改用"数组 + 下标遍历"，输入校验改用"返回值 + 重试循环"。
 *   两版游戏逻辑、函数划分完全一致，差别只在数据容器与错误处理机制。
 *
 * 编译（MSVC）： cl /O2 /W3 exp3_montyhall.c
 * 编译（GCC） ： gcc -O2 -Wall -o exp3_montyhall exp3_montyhall.c
 * 运行：exp3_montyhall                进入菜单，可选人工玩/自动模拟
 *       exp3_montyhall 100000         直接自动模拟 100000 局
 * ========================================================================== */
/* MSVC 会把 sscanf 这类传统 C 函数报成"不安全"，必须在包含头文件之前定义该宏 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N_DOORS 3
#define GOAT 0
#define CAR  1

static const char *prize_name(int p) { return p == CAR ? "CAR" : "GOAT"; }

/* --------------------------------------------------------------------------
 * 游戏建模
 * ------------------------------------------------------------------------ */
static void init_doors(int doors[N_DOORS])
{
    int i;
    for (i = 0; i < N_DOORS; i++) doors[i] = GOAT;   /* 先全是羊 */
    doors[rand() % N_DOORS] = CAR;                   /* 再把车塞进随机一扇门 */
}

static int car_door(const int doors[N_DOORS])
{
    int i;
    for (i = 0; i < N_DOORS; i++)
        if (doors[i] == CAR) return i;
    return -1;
}

/* 主持人打开一扇"不是玩家所选、且后面是山羊"的门
 * 对应 Python 的 hosts_candidates = set(doors) - {choice} - {car_door} */
static int host_open(const int doors[N_DOORS], int choice)
{
    int cand[N_DOORS], m = 0, i;

    for (i = 0; i < N_DOORS; i++)
        if (i != choice && doors[i] != CAR)
            cand[m++] = i;

    /* m == 1 → 玩家选到羊，主持人没得挑；m == 2 → 玩家选到车，随便挑一扇羊门 */
    if (m == 0) { fprintf(stderr, "no candidate to open\n"); exit(1); }
    return cand[rand() % m];
}

/* 剩下那扇"既没被玩家选、也没被打开"的门（换门就改选它） */
static int other_closed(int choice, int opened)
{
    int i;
    for (i = 0; i < N_DOORS; i++)
        if (i != choice && i != opened) return i;
    return -1;
}

/* --------------------------------------------------------------------------
 * 输入处理：C 没有异常机制，用"读一行 + 校验 + 重试循环"实现同样的保护。
 *   返回 0 表示遇到 EOF —— 相当于 Python 里捕获 EOFError 的那条分支。
 * ------------------------------------------------------------------------ */
static int read_line(char *buf, int size)
{
    if (fgets(buf, size, stdin) == NULL) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';       /* 去掉行尾换行 */
    return 1;
}

static int read_door(void)
{
    char buf[64];
    int v;

    for (;;) {
        printf("Choose a door (1/2/3) > ");
        fflush(stdout);
        if (!read_line(buf, sizeof buf)) { printf("\nEOF, bye\n"); exit(0); }
        if (sscanf(buf, "%d", &v) == 1 && v >= 1 && v <= N_DOORS)
            return v - 1;                   /* 转成 0 基下标 */
        printf("  invalid input, expected 1, 2 or 3\n");
    }
}

static int read_yes_no(const char *prompt)
{
    char buf[64];

    for (;;) {
        printf("%s (y/n) > ", prompt);
        fflush(stdout);
        if (!read_line(buf, sizeof buf)) { printf("\nEOF, bye\n"); exit(0); }
        if (buf[0] == 'y' || buf[0] == 'Y') return 1;
        if (buf[0] == 'n' || buf[0] == 'N') return 0;
        printf("  invalid input, please type y or n\n");
    }
}

/* --------------------------------------------------------------------------
 * 人工玩一局
 * ------------------------------------------------------------------------ */
static void play_manual(void)
{
    int doors[N_DOORS], choice, opened, target, final_door, sw, won;

    init_doors(doors);
    printf("\nThree doors are ready: [1] [2] [3]  (you cannot see behind them)\n");

    choice = read_door();
    printf("Your choice: door %d\n", choice + 1);

    opened = host_open(doors, choice);
    target = other_closed(choice, opened);
    printf("Host opens door %d, and behind it is a %s.\n",
           opened + 1, prize_name(doors[opened]));

    sw = read_yes_no("Do you want to switch to the other door");
    final_door = sw ? target : choice;
    won = (doors[final_door] == CAR);

    printf("\nFinal door: %d, behind it is a %s\n",
           final_door + 1, prize_name(doors[final_door]));
    printf("%s\n", won ? "*** YOU WIN A CAR! ***" : "--- sorry, you get a goat ---");
    printf("(you %s; the car was behind door %d)\n",
           sw ? "switched" : "stayed", car_door(doors) + 1);
}

/* --------------------------------------------------------------------------
 * 自动模拟：同一批局面下，分别统计"坚持"与"换门"的胜局数
 * ------------------------------------------------------------------------ */
static void simulate(long times)
{
    long win_stay = 0, win_switch = 0, i;
    double p_stay, p_switch;

    for (i = 0; i < times; i++) {
        int doors[N_DOORS], choice, opened, alt;

        init_doors(doors);
        choice = rand() % N_DOORS;
        opened = host_open(doors, choice);
        alt = other_closed(choice, opened);

        if (doors[choice] == CAR) win_stay++;
        if (doors[alt] == CAR)    win_switch++;
    }

    p_stay = (double)win_stay / (double)times;
    p_switch = (double)win_switch / (double)times;

    printf("\n===============================================================\n");
    printf("auto simulation: %ld games (random choice, host always opens a goat)\n", times);
    printf("===============================================================\n");
    printf("%-14s %12s %12s %12s\n", "strategy", "wins", "simulated", "theory");
    printf("---------------------------------------------------------------\n");
    printf("%-14s %12ld %11.2f%% %11.2f%%\n", "stay", win_stay, p_stay * 100.0, 100.0 / 3.0);
    printf("%-14s %12ld %11.2f%% %11.2f%%\n", "switch", win_switch, p_switch * 100.0, 200.0 / 3.0);
    printf("---------------------------------------------------------------\n");
    printf("wins(stay) + wins(switch) = %ld = games  -> every game one strategy wins\n",
           win_stay + win_switch);
    printf("conclusion: switching (%.2f%%) beats staying (%.2f%%) -> SWITCH\n",
           p_switch * 100.0, p_stay * 100.0);
}

/* --------------------------------------------------------------------------
 * 主菜单
 * ------------------------------------------------------------------------ */
static void menu(void)
{
    char buf[64];

    for (;;) {
        printf("\n1) play one game manually\n");
        printf("2) auto simulation: compare STAY vs SWITCH\n");
        printf("0) quit\n");
        printf("choose > ");
        fflush(stdout);

        if (!read_line(buf, sizeof buf)) { printf("\nbye\n"); return; }

        if (strcmp(buf, "0") == 0) { printf("\nbye\n"); return; }
        if (strcmp(buf, "1") == 0) { play_manual(); continue; }
        if (strcmp(buf, "2") == 0) {
            long times = 10000;
            printf("games (default 10000) > ");
            fflush(stdout);
            if (!read_line(buf, sizeof buf)) { printf("\nbye\n"); return; }
            if (buf[0] != '\0') {
                times = atol(buf);
                if (times <= 0) { printf("  invalid number, use 10000\n"); times = 10000; }
            }
            simulate(times);
            continue;
        }
        printf("  invalid input, expected 0 / 1 / 2\n");
    }
}

int main(int argc, char *argv[])
{
    srand(20260922u);            /* 固定种子，结果可复现 */

    if (argc > 1) {              /* 命令行直接给局数 → 跳过菜单 */
        long times = atol(argv[1]);
        if (times <= 0) { printf("times must be a positive integer\n"); return 1; }
        simulate(times);
        return 0;
    }

    printf("===============================================================\n");
    printf("Experiment 3: Monty Hall Problem simulation\n");
    printf("===============================================================\n");
    menu();
    return 0;
}
