/* ============================================================================
 * 实验一 · 约瑟夫环（Josephus Problem）—— C 语言对应实现
 * ----------------------------------------------------------------------------
 * 问题：n 个人围成一圈，编号 1..n。从 1 号开始 1..k 报数，报到 k 的人出圈，
 *       圈子缩小后从下一个人继续报数，直到圈中只剩 1 人。求最后留下者的编号。
 *
 * 四种实现，与 Python 版设计思想一一对照：
 *   1) sim_array_ring()   数组 + 存活标记 + 环形下标推进
 *                         ← 对应 Python 的 itertools.cycle 迭代器法
 *                           （本质都是"游标绕圈 + 跳过出局者"）
 *   2) sim_array_shift()  数组 + memmove 搬移，数组始终保持存活者稠密
 *                         ← 对应 Python 的列表切片法（people[:i] + people[i+1:]）
 *   3) sim_linked_list()  循环单链表 + 手写 malloc/free
 *                         ← 对应 Python 迭代器法；额外体现 C 必须自己管内存
 *   4) solve_formula()    O(n) 数学递推 f(i) = (f(i-1) + k) % i
 *                         ← 对应 Python 的 josephus_recur()（解析解，无过程）
 *
 * 编译（MSVC）： cl /O2 /W3 exp1_josephus.c
 * 编译（GCC） ： gcc -O2 -Wall -o exp1_josephus exp1_josephus.c
 * 运行：exp1_josephus               默认 n = 10, k = 3
 *       exp1_josephus 41 3         自定义 n、k
 *       exp1_josephus 20000 3 bench   只跑性能对比
 *
 * 说明：输出文字用英文。C 的源码编码、控制台代码页（中文 Windows 默认 GBK）
 *       与字符串字面量三者一旦不一致就会乱码，英文输出是跨编译器最稳的选择。
 *       若确需中文输出：源码存为 UTF-8 并在 main() 开头调用
 *       SetConsoleOutputCP(65001)（同时 #include <windows.h>）。
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* --------------------------------------------------------------------------
 * 方法一：数组 + 存活标记 + 环形下标推进
 *   不需要任何"搬移"，靠取模让下标绕着数组转圈；出圈者只翻一个标记位。
 *   时间：期望 O(n·ln n)（跳过死者的空转），空间 O(n)。
 * ------------------------------------------------------------------------ */
static int sim_array_ring(int n, int k, int verbose)
{
    int *alive = (int *)malloc((size_t)n * sizeof(int));
    int i = 0, left = n, count = 0, round = 0, last = 1;

    if (alive == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
    for (i = 0; i < n; i++) alive[i] = 1;
    i = 0;                               /* 关键：初始化循环跑完 i == n，必须把游标归零 */

    while (left > 1) {
        if (alive[i]) {
            if (++count == k) {          /* 报到 k → 出圈 */
                alive[i] = 0;
                count = 0;
                left--;
                if (verbose)
                    printf("  round %4d : no.%4d is OUT | left %4d\n",
                           ++round, i + 1, left);
            }
        }
        i = (i + 1) % n;                 /* 环形推进：取模代替 if 判断 */
    }
    for (i = 0; i < n; i++)
        if (alive[i]) { last = i + 1; break; }

    free(alive);                         /* 手动释放 */
    return last;
}

/* --------------------------------------------------------------------------
 * 方法二：数组 + memmove 搬移（"紧凑数组"）
 *   出圈者的位置 i = (i + k - 1) % m，把它后面的元素整体左移一格，
 *   等价于 Python 的 people[:i] + people[i+1:] 切片拼接。
 *   时间：O(n^2)（每次搬移 O(n)），空间 O(n)。
 * ------------------------------------------------------------------------ */
static int sim_array_shift(int n, int k, int verbose)
{
    int *p = (int *)malloc((size_t)n * sizeof(int));
    int i = 0, m = n, round = 0, last;

    if (p == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
    for (i = 0; i < n; i++) p[i] = i + 1;     /* p = [1, 2, ..., n] */

    while (m > 1) {
        i = (i + k - 1) % m;                   /* 报数 k 次落在谁头上 */
        if (verbose)
            printf("  round %4d : no.%4d is OUT | left %4d\n", ++round, p[i], m - 1);
        /* 等价于切片删除 p[i]：把 [i+1, m) 整体左移一格 */
        memmove(&p[i], &p[i + 1], (size_t)(m - i - 1) * sizeof(int));
        m--;
        if (i == m) i = 0;                     /* 删的是队尾 → 游标回队首 */
    }
    last = p[0];
    free(p);
    return last;
}

/* --------------------------------------------------------------------------
 * 方法三：循环单链表
 *   线性的链表靠"尾结点指向头结点"变成真正的环，"下一个存活者"就是 next 指针，
 *   不需要跳过死者的空转。但每删一个结点就要 free 一次，内存管理全在程序员手上。
 *   时间：O(n·k)（k 次指针后移），空间 O(n) + 每个结点的指针开销。
 * ------------------------------------------------------------------------ */
typedef struct Node {
    int id;
    struct Node *next;
} Node;

static int sim_linked_list(int n, int k, int verbose)
{
    Node *head, *tail, *prev, *cur;
    int i, round = 0, last;

    if (n == 1) return 1;

    head = (Node *)malloc(sizeof(Node));
    if (head == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
    head->id = 1;
    tail = head;
    for (i = 2; i <= n; i++) {
        tail->next = (Node *)malloc(sizeof(Node));
        if (tail->next == NULL) { fprintf(stderr, "malloc failed\n"); exit(1); }
        tail = tail->next;
        tail->id = i;
    }
    tail->next = head;                    /* 首尾相接 —— 真正的"环" */

    prev = tail;
    cur = head;
    while (cur->next != cur) {            /* 只剩一个结点时 next 指向自己 */
        for (i = 1; i < k; i++) {         /* 沿环走 k 步 */
            prev = cur;
            cur = cur->next;
        }
        prev->next = cur->next;           /* 摘链 */
        if (verbose)
            printf("  round %4d : no.%4d is OUT\n", ++round, cur->id);
        free(cur);                        /* 必须手动释放，否则内存泄漏 */
        cur = prev->next;
    }
    last = cur->id;
    free(cur);
    return last;
}

/* --------------------------------------------------------------------------
 * 方法四：O(n) 数学递推（解析解，不模拟、无过程输出）
 *   f(1) = 0，f(i) = (f(i-1) + k) % i，最后 +1 转成 1 基编号。
 * ------------------------------------------------------------------------ */
static int solve_formula(int n, int k)
{
    int pos = 0, i;
    for (i = 2; i <= n; i++) pos = (pos + k) % i;
    return pos + 1;
}

/* --------------------------------------------------------------------------
 * 对拍：小规模枚举，校验四种实现结果完全一致
 * ------------------------------------------------------------------------ */
static int cross_check(int limit)
{
    static const int ks[] = { 1, 2, 3, 5, 7 };
    int n, t, k, a, b, c, d, bad = 0, total = 0;

    for (n = 1; n <= limit; n++) {
        for (t = 0; t < (int)(sizeof(ks) / sizeof(ks[0])); t++) {
            k = ks[t];
            total++;
            a = sim_array_ring(n, k, 0);
            b = sim_array_shift(n, k, 0);
            c = sim_linked_list(n, k, 0);
            d = solve_formula(n, k);
            if (!(a == b && b == c && c == d)) {
                bad++;
                printf("  MISMATCH n=%d k=%d : %d / %d / %d / %d\n", n, k, a, b, c, d);
            }
        }
    }
    printf("[cross-check] %d cases checked, %s\n",
           total, bad == 0 ? "ALL MATCH" : "SOME MISMATCH");
    return bad == 0;
}

/* --------------------------------------------------------------------------
 * 性能对比
 * ------------------------------------------------------------------------ */
static void bench_run(const char *name, int result, clock_t t0, clock_t t1)
{
    printf("  %-16s result %-8d  %.4f s\n",
           name, result, (double)(t1 - t0) / CLOCKS_PER_SEC);
}

static void benchmark(int n, int k)
{
    clock_t t0, t1;
    int r;

    printf("\n=== performance (n = %d, k = %d) ===\n", n, k);

    t0 = clock(); r = sim_array_ring(n, k, 0);    t1 = clock(); bench_run("array+ring", r, t0, t1);
    t0 = clock(); r = sim_array_shift(n, k, 0);   t1 = clock(); bench_run("array+memmove", r, t0, t1);
    t0 = clock(); r = sim_linked_list(n, k, 0);   t1 = clock(); bench_run("linked list", r, t0, t1);
    t0 = clock(); r = solve_formula(n, k);        t1 = clock(); bench_run("math formula", r, t0, t1);
}

int main(int argc, char *argv[])
{
    int n = 10, k = 3, bench = 0;
    int r1, r2, r3, r4;

    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) k = atoi(argv[2]);
    if (argc > 3 && strcmp(argv[3], "bench") == 0) bench = 1;

    if (n < 1 || k < 1) {
        printf("n and k must be positive integers\n");
        return 1;
    }

    if (!bench) {
        printf("=== Experiment 1: Josephus Problem (n = %d, k = %d) ===\n\n", n, k);

        printf("--- Method 1: array + alive flag + ring index ---\n");
        r1 = sim_array_ring(n, k, 1);
        printf("=> survivor = %d\n\n", r1);

        printf("--- Method 2: array + memmove (compact) ---\n");
        r2 = sim_array_shift(n, k, 1);
        printf("=> survivor = %d\n\n", r2);

        printf("--- Method 3: circular singly linked list ---\n");
        r3 = sim_linked_list(n, k, 1);
        printf("=> survivor = %d\n\n", r3);

        printf("--- Method 4: O(n) formula (no trace) ---\n");
        r4 = solve_formula(n, k);
        printf("=> survivor = %d\n\n", r4);

        printf("--- consistency ---\n");
        printf("  %s\n\n", (r1 == r2 && r2 == r3 && r3 == r4) ? "all four agree" : "MISMATCH");
        cross_check(200);
    }

    benchmark(bench ? n : 20000, k);
    return 0;
}
