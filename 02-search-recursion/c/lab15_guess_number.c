/* ============================================================================
 * 实验 15  无界面版猜数游戏设计与实现
 * ----------------------------------------------------------------------------
 * 对应教材 Python 参考代码：
 *      def guessNumber(maxValue=10, maxTimes=3)
 * 设计要点（与 Python 版对照）：
 *   1) 随机数生成   ：Python random.randint(1,maxValue)
 *                     C     rand()%maxValue + 1  （同样是 [1, maxValue] 闭区间）
 *   2) 大小比较分支 ：if / elif x>value / else 三种情形
 *                     C     同构的 if / else if / else
 *   3) 输入合法性   ：Python input() 后 int() 转换，失败抛异常被 except 捕获
 *                     这里不直接用 scanf 求值（scanf 失败会把非法字符留在缓冲区，
 *                     造成死循环），改为“按字符串读入 + 手动校验 + 循环重读”，
 *                     语义上等价于原来的 try/except 结构。
 *   4) 次数用完     ：Python for-else（循环未被 break 时执行 else 分支）
 *                     C     用 “是否猜中” 的标志变量来等价模拟 for-else。
 *   5) 提前结束     ：break
 *                     C     break / 提前 return
 *
 * 编译：  cl lab15_guess_number.c      (MSVC)
 *         gcc lab15_guess_number.c -o lab15_guess_number
 * ==========================================================================*/

#define _CRT_SECURE_NO_WARNINGS      /* 允许使用标准 scanf（MSVC 默认会报 C4996 安全告警） */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* 读入一个整数字符串并解析；成功返回 1，失败（含非法字符 / 空串）返回 0 */
static int read_int(int *out)
{
    char buf[64];
    if (scanf("%63s", buf) != 1) {
        return 0;                 /* 输入流结束或读取失败 */
    }
    /* 逐字符校验：允许首位正负号，其余必须全是数字 */
    int i = 0, sign = 1;
    if (buf[0] == '+' || buf[0] == '-') {
        sign = (buf[0] == '-') ? -1 : 1;
        i = 1;
    }
    if (buf[i] == '\0') return 0; /* 只有一个符号，非法 */
    for (; buf[i] != '\0'; i++) {
        if (!isdigit((unsigned char)buf[i])) return 0;
    }
    *out = sign * (int)strtol(buf, NULL, 10);
    return 1;
}

/* 核心函数：参数含义与 Python 版一一对应
 *   maxValue : 随机数取值范围 [1, maxValue]
 *   maxTimes : 最多允许猜的次数
 */
void guess_number(int maxValue, int maxTimes)
{
    int value = rand() % maxValue + 1;   /* 随机生成一个整数 */
    int guessed = 0;                     /* 等价于 Python 的 for-else：是否猜中 */

    for (int i = 0; i < maxTimes; i++) {
        /* prompt = 'Start to GUESS:' if i==0 else 'Guess again:' */
        printf(i == 0 ? "Start to GUESS: " : "Guess again: ");

        int x;
        if (!read_int(&x)) {
            /* 等价于 Python 的 except 分支 */
            printf("Must input an integer between 1 and %d\n", maxValue);
            continue;                    /* 本次不计入有效次数之外，继续下一次 */
        }

        if (x == value) {                /* 猜对了 */
            printf("Congratulations!\n");
            guessed = 1;
            break;
        } else if (x > value) {          /* elif x > value */
            printf("Too big\n");
        } else {                         /* else */
            printf("Too little\n");
        }
    }

    if (!guessed) {                      /* 等价于 for-else 的 else 分支 */
        printf("Game over. FAIL.\n");
        printf("The value is %d\n", value);
    }
}

int main(void)
{
    srand((unsigned)time(NULL));         /* 用时间做种子，保证每次运行随机数不同 */
    guess_number(10, 3);                 /* Python: guessNumber() 默认参数 maxValue=10, maxTimes=3 */
    return 0;
}
