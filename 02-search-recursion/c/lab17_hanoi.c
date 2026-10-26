/* ============================================================================
 * 实验 17  模拟汉诺塔问题
 * ----------------------------------------------------------------------------
 * 对应教材 Python 参考代码：
 *      def hannoi(num, src, dst, temp=None)     # 默认参数 + 递归
 * 规则：3 个底座 A/B/C，n 个大小不等的盘子从 A 全部移到 C，
 *       每次只移动一个盘子，且任何时刻都保持“大盘在下、小盘在上”。
 *
 * 设计要点（与 Python 版对照）：
 *   1) 函数默认参数  ：Python 的 def f(..., temp=None) 依据实参个数自动补默认值
 *                      C 没有默认参数 -> 用“同名的薄封装 + 内部实现函数”模拟：
 *                      hannoi(num, src, dst)          <- 对外接口（2 个参数）
 *                      hannoi_impl(num, src, dst, temp) <- 真正的实现（4 个参数）
 *   2) 全局计数      ：Python 用 global times 声明后自增
 *                      C     用文件作用域静态变量（static），语义一致
 *   3) 递归结构      ：hannoi(n-1, src, temp, dst) -> 移最大盘 -> hannoi(n-1, temp, dst, src)
 *                      C     完全同构，递归调用顺序一模一样
 *   4) 列表对象      ：Python towers = {'A': list(range(n,0,-1)), 'B': [], 'C': []}
 *                      C     用 int stack[3][MAXN] 保存三个底座上的盘子，
 *                            另用 int top[3] 表示每个底座栈顶下标
 *                      Python 的 towers[dst].append(towers[src].pop())
 *                      C      等价于 stack[dst][++top[dst]] = stack[src][top[src]--]
 * ==========================================================================*/

#include <stdio.h>

#define MAXN 64
#define MAXTOWER 3

#define TOWER_A 0
#define TOWER_B 1
#define TOWER_C 2

static int  g_times = 0;                 /* 等价于 Python 的 global times */
static int  g_stack[MAXTOWER][MAXN];     /* 每个底座上的盘子（栈结构） */
static int  g_top[MAXTOWER];             /* 每个底座当前栈顶下标，-1 表示空 */
static char g_name[MAXTOWER] = { 'A', 'B', 'C' };

/* 输出 3 个底座上的盘子分布情况（对应 Python 的 for tower in 'ABC': print(...)） */
static void print_towers(void)
{
    for (int t = 0; t < MAXTOWER; t++) {
        printf("%c:", g_name[t]);
        for (int i = g_top[t]; i >= 0; i--) {   /* 栈顶（小盘）在前，符合书中打印顺序 */
            printf(" %d", g_stack[t][i]);
        }
        printf("\n");
    }
}

/* 真正的递归实现：temp 显式传入，对应 Python 的 hannoi(num, src, dst, temp) */
static void hannoi_impl(int num, int src, int dst, int temp)
{
    if (num < 1) {                       /* 递归出口（对应 Python: if num < 1: return） */
        return;
    }

    /* 递归调用函数自身，先把除最后一个盘子之外的所有盘子移动到临时柱子上 */
    hannoi_impl(num - 1, src, temp, dst);

    /* 移动最后一个盘子 */
    g_times++;
    printf("The %d Times move:[%c]=>[%c]\n", g_times, g_name[src], g_name[dst]);

    /* Python: towers[dst].append(towers[src].pop()) */
    g_stack[dst][++g_top[dst]] = g_stack[src][g_top[src]--];

    print_towers();                      /* 输出 3 个底座上的盘子 */

    /* 把除最后一个盘子之外的其他盘子从临时底座上移动到目标底座上 */
    hannoi_impl(num - 1, temp, dst, src);
}

/* 对外接口：模拟 Python 的默认参数 temp=None（调用时自动选用第三个底座作临时柱） */
void hannoi(int num, int src, int dst)
{
    int temp;
    if (src != TOWER_A && dst != TOWER_A)      temp = TOWER_A;
    else if (src != TOWER_B && dst != TOWER_B) temp = TOWER_B;
    else                                       temp = TOWER_C;
    hannoi_impl(num, src, dst, temp);
}

int main(void)
{
    int n = 3;                           /* 盘子数量，可自行修改（教材例为 64，此处取 3 便于观察） */

    /* Python: towers = {'A': list(range(n,0,-1)), 'B': [], 'C': []} —— 所有盘子都在 A 上 */
    g_top[TOWER_A] = n - 1;
    g_top[TOWER_B] = -1;
    g_top[TOWER_C] = -1;
    for (int i = 0; i < n; i++) {
        /* range(n,0,-1) 依次得到 n, n-1, ..., 1；下标 0 存最大的盘 n */
        g_stack[TOWER_A][i] = n - i;
    }

    printf("初始状态:\n");
    print_towers();

    /* A 表示最初放置盘子的底座；C 是目标底座；B 是临时底座 */
    hannoi(n, TOWER_A, TOWER_C);

    printf("共移动 %d 次\n", g_times);
    return 0;
}
