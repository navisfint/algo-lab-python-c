/* ============================================================================
 * 实验二 · 模拟轮盘抽奖 —— C 语言对应实现
 * ----------------------------------------------------------------------------
 * 扇区模型（面积越小奖品越高）：
 *      1st prize   [  0,  30)   30 deg   30/360  =  8.33%
 *      2nd prize   [ 30, 108)   78 deg   78/360  = 21.67%
 *      3rd prize   [108, 360)  252 deg  252/360  = 70.00%
 *   指针位置用 0~360 的随机数模拟，试玩 times 次统计各奖项中奖次数。
 *
 * 与 Python 版对照：
 *   Python 用 dict{'一等奖': [0, 30], ...} 描述扇区，靠 items() 遍历做区间匹配；
 *   C 没有字典，用"结构体数组 + 下标"表达同一张表（下标即奖项编号）。
 *   两种实现思路相同，差别在于 Python 的字典是哈希表、C 的数组是连续内存，
 *   前者按 key 而后者按下标访问。
 *
 * 随机数：Python 的 random.randrange() 基于 Mersenne Twister，无偏；
 *         C 的 rand() 由实现决定（MSVC 是 32 位 LCG，RAND_MAX 只有 32767，
 *         低位随机性差），因此这里用 rand()/(RAND_MAX+1) 缩放，
 *         而不是常见的 rand() % 360——后者会把低位偏差放大到结果里。
 *
 * 编译（MSVC）： cl /O2 /W3 exp2_roulette.c
 * 编译（GCC） ： gcc -O2 -Wall -o exp2_roulette exp2_roulette.c
 * 运行：exp2_roulette              10000 次，固定种子
 *       exp2_roulette 100000       自定义次数
 *       exp2_roulette 10000 0      次数 + 种子（种子 0 = 用当前时间）
 * ========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    const char *name;
    int lo;              /* 起始角度（含） */
    int hi;              /* 结束角度（不含） */
} Sector;

static const Sector AREAS[] = {
    { "1st prize",   0,  30 },
    { "2nd prize",  30, 108 },
    { "3rd prize", 108, 360 }
};
#define N_AREAS ((int)(sizeof(AREAS) / sizeof(AREAS[0])))

/* --------------------------------------------------------------------------
 * 转盘：把 rand() 均匀映射到 [0, 360)
 * ------------------------------------------------------------------------ */
static int spin_angle(void)
{
    return (int)((double)rand() / ((double)RAND_MAX + 1.0) * 360.0);
}

/* --------------------------------------------------------------------------
 * 实现一：区间线性匹配（对应教材参考代码的字典遍历写法）
 * ------------------------------------------------------------------------ */
static int play_linear(void)
{
    int angle = spin_angle();
    int i;

    for (i = 0; i < N_AREAS; i++)
        if (AREAS[i].lo <= angle && angle < AREAS[i].hi)
            return i;
    return -1;                   /* 只有扇区划分写错才会走到这里 */
}

/* --------------------------------------------------------------------------
 * 实现二（拓展）：累积度数 + 二分查找
 *   扇区一多（几十上百个），线性匹配 O(m)；先把边界累加成前缀和，
 *   再用 upper_bound 定位，复杂度降到 O(log m)。
 * ------------------------------------------------------------------------ */
static int cum[N_AREAS];

static void build_cum(void)
{
    int i, s = 0;
    for (i = 0; i < N_AREAS; i++) {
        s += AREAS[i].hi - AREAS[i].lo;
        cum[i] = s;                       /* cum = {30, 108, 360} */
    }
}

static int play_bisect(void)
{
    int angle = spin_angle();
    int lo = 0, hi = N_AREAS - 1, mid;

    while (lo < hi) {                     /* upper_bound：第一个 cum[i] > angle */
        mid = (lo + hi) / 2;
        if (cum[mid] <= angle) lo = mid + 1;
        else                   hi = mid;
    }
    return lo;
}

/* --------------------------------------------------------------------------
 * 统计与报告
 * ------------------------------------------------------------------------ */
static void report(const char *title, const long *cnt, long times)
{
    long span_sum = 0, cnt_sum = 0;
    int i;

    printf("===============================================================\n");
    printf("%s  (%ld spins)\n", title, times);
    printf("===============================================================\n");
    printf("%-12s %9s %10s %11s %10s %10s\n",
           "prize", "span", "theory", "count", "freq", "dev");
    printf("---------------------------------------------------------------\n");
    for (i = 0; i < N_AREAS; i++) {
        int span = AREAS[i].hi - AREAS[i].lo;
        double theory = (double)span / 360.0;
        double freq = (double)cnt[i] / (double)times;

        printf("%-12s %6d deg %9.2f%% %11ld %9.2f%% %+9.2f%%\n",
               AREAS[i].name, span, theory * 100.0, cnt[i],
               freq * 100.0, (freq - theory) * 100.0);

        span_sum += span;
        cnt_sum += cnt[i];
    }
    printf("---------------------------------------------------------------\n");
    printf("%-12s %6ld deg %9.2f%% %11ld %9.2f%%\n",
           "TOTAL", span_sum, 100.0, cnt_sum, 100.0);
}

int main(int argc, char *argv[])
{
    long times = 10000;
    unsigned seed = 20260922u;
    long i;
    long *cnt1, *cnt2;
    int k;
    int same = 1;

    if (argc > 1) times = atol(argv[1]);
    if (argc > 2) {
        seed = (unsigned)strtoul(argv[2], NULL, 10);
        if (seed == 0) seed = (unsigned)time(NULL);   /* 0 = 用当前时间 */
    }
    if (times <= 0) {
        printf("times must be a positive integer\n");
        return 1;
    }

    /* 防御性检查：扇区总角度必须正好等于 360 */
    build_cum();
    if (cum[N_AREAS - 1] != 360) {
        printf("sector definition error: total span = %d, expected 360\n", cum[N_AREAS - 1]);
        return 1;
    }

    printf("roulette sectors:\n");
    for (k = 0; k < N_AREAS; k++)
        printf("  %-12s [%3d, %3d)  span %3d deg\n",
               AREAS[k].name, AREAS[k].lo, AREAS[k].hi, AREAS[k].hi - AREAS[k].lo);
    printf("seed = %u\n\n", seed);

    cnt1 = (long *)calloc((size_t)N_AREAS, sizeof(long));
    cnt2 = (long *)calloc((size_t)N_AREAS, sizeof(long));
    if (cnt1 == NULL || cnt2 == NULL) { fprintf(stderr, "calloc failed\n"); return 1; }

    /* 实现一：线性匹配。两种实现共用同一随机序列（各调一次 spin_angle），
     * 因此相同种子下应得到逐次一致的结果，便于严格对比。 */
    srand(seed);
    for (i = 0; i < times; i++) cnt1[play_linear()]++;

    /* 实现二：累积度数 + 二分查找 */
    srand(seed);
    for (i = 0; i < times; i++) cnt2[play_bisect()]++;

    report("Implementation 1: linear interval match", cnt1, times);
    printf("\n");
    report("Implementation 2: cumulative + binary search", cnt2, times);

    for (k = 0; k < N_AREAS; k++)
        if (cnt1[k] != cnt2[k]) same = 0;
    printf("\nboth implementations give identical counts: %s\n", same ? "YES" : "NO");

    free(cnt1);
    free(cnt2);
    return 0;
}
