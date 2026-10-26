/* ============================================================================
 * 实验 16  抓狐狸游戏设计与实现
 * ----------------------------------------------------------------------------
 * 对应教材 Python 参考代码：
 *      def catchMe(n=5, maxStep=10)
 * 游戏规则：
 *      n 个洞口（0 ~ n-1），初始只有一个洞口有狐狸（用列表标记为 1）；
 *      玩家每轮输入一个洞口编号；
 *        · 猜中 -> 成功，游戏结束；
 *        · 猜错 -> 狐狸跳到“隔壁”洞口后进入下一轮；
 *      用完 maxStep 次仍未抓到则失败。
 *
 * 设计要点（与 Python 版对照）：
 *   1) 数据表示   ：Python 用列表 positions = [0]*n 标记每个洞口是否有狐狸
 *                   C    用等价的 int positions[n]（VLA / 动态数组均可）
 *   2) 初始位置   ：Python random.randrange(0,n)
 *                   C     rand() % n
 *   3) 输入校验   ：Python “while True + try/except + assert 0<=x<n”
 *                   C     “按字符串读入 + 手动解析 + 范围判断 + 重读循环”
 *                   —— 这是 C 里等价于“异常处理式输入约束”的惯用写法。
 *   4) 狐狸移动   ：Python 边界单独处理 oldPos==n-1 / oldPos==0，
 *                   内部洞口用 random.choice((-1,1))
 *                   C    完全同构，用 rand()%2 ? 1 : -1 得到 ±1
 *   5) 状态更新   ：positions[oldPos], positions[newPos] = 0, 1
 *                   C    两条普通赋值（C 没有多重赋值语法）
 * ==========================================================================*/

#define _CRT_SECURE_NO_WARNINGS      /* 允许使用标准 scanf（MSVC 默认会报 C4996 安全告警） */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

/* 读入一个非负整数：成功返回 1；非法（含非数字）返回 0 */
static int read_int(int *out)
{
    char buf[64];
    if (scanf("%63s", buf) != 1) return 0;
    int i = 0;
    if (buf[0] == '+') i = 1;
    if (buf[i] == '\0') return 0;
    for (int j = i; buf[j] != '\0'; j++) {
        if (!isdigit((unsigned char)buf[j])) return 0;
    }
    *out = (int)strtol(buf, NULL, 10);
    return 1;
}

/* 抓狐狸
 *   n       : 洞口个数（编号 0 ~ n-1）
 *   maxStep : 最多允许猜的次数
 */
void catch_me(int n, int maxStep)
{
    /* Python: positions = [0] * n —— 0 表示无狐狸，1 表示有狐狸 */
    int *positions = (int *)calloc((size_t)n, sizeof(int));
    if (positions == NULL) {                 /* C 特有的资源申请失败检查 */
        printf("memory allocation failed\n");
        return;
    }

    int oldPos = rand() % n;                 /* 狐狸的随机初始位置 */
    positions[oldPos] = 1;

    int alive = 1;                           /* 是否仍在游戏（对应 while maxStep >= 0） */
    while (alive && maxStep > 0) {
        maxStep--;

        /* 这个循环保证用户输入的是有效洞口编号（等价于 while True + try/except + assert） */
        int x = -1;
        for (;;) {
            printf("请输入洞口编号(0-%d):", n - 1);
            if (!read_int(&x)) {
                printf("要按套路来啊,再给你一次机会。\n");
                continue;                    /* 非数字 -> 重新输入 */
            }
            if (x >= 0 && x < n) break;      /* 有效范围 -> 跳出输入循环 */
            printf("要按套路来啊,再给你一次机会。\n");  /* 越界 -> 重新输入 */
        }

        if (positions[x] == 1) {
            printf("成功,我抓到小狐狸。\n");
            alive = 0;
            break;
        } else {
            printf("今天又没抓到。\n");
        }

        /* 这次没抓到，狐狸就跳到隔壁洞口 */
        int newPos;
        if (oldPos == n - 1) {
            newPos = oldPos - 1;
        } else if (oldPos == 0) {
            newPos = oldPos + 1;
        } else {
            newPos = oldPos + (rand() % 2 ? 1 : -1);   /* 等价 random.choice((-1,1)) */
        }
        /* Python: positions[oldPos], positions[newPos] = 0, 1 */
        positions[oldPos] = 0;
        positions[newPos] = 1;
        oldPos = newPos;
    }

    if (alive) {                             /* 次数用完还没抓到 */
        printf("放弃吧,你这样乱试是没有希望的。\n");
    }

    free(positions);
}

int main(void)
{
    srand((unsigned)time(NULL));
    catch_me(5, 10);                         /* Python: catchMe() 默认 n=5, maxStep=10 */
    return 0;
}
