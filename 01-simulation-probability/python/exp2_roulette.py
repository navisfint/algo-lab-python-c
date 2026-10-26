# -*- coding: utf-8 -*-
"""
实验二 · 模拟轮盘抽奖
===========================================================================
游戏规则
    轮盘按面积划分为三个扇区，面积越小奖品价值越高：

        一等奖  [0,   30)   30°   理论概率 30/360  =  8.33%
        二等奖  [30, 108)   78°   理论概率 78/360  = 21.67%
        三等奖  [108, 360) 252°   理论概率 252/360 = 70.00%

    指针所指区域的序号即为本次中奖等级。指针位置用 0~360 的随机数模拟。
    试玩 10000 次，记录每个奖项的中奖次数。

知识点对应（实验目的）
    (1) 先建模再动手：把"轮盘"抽象成"角度区间 → 奖项"的映射
    (2) 字典解决实际问题：areas 用"奖项 → [起始角, 结束角]"描述扇区
    (3) 字典的 get() 方法：counts.get(prize, 0) + 1 统计次数
    (4) random 常用函数：randrange() / random() / seed()
    (5) 循环结构和选择结构

运行：
    python exp2_roulette.py            10000 次
    python exp2_roulette.py 100000     自定义次数
===========================================================================
"""
import sys
from bisect import bisect_right
from random import randrange, seed

# --------------------------------------------------------------------------
# 数据建模：字典描述扇区（角度的左闭右开区间，总面积必须等于 360°）
# --------------------------------------------------------------------------
areas = {
    '一等奖': [0, 30],      # 30°  面积最小 → 奖品最高
    '二等奖': [30, 108],    # 78°
    '三等奖': [108, 360],   # 252°
}


# --------------------------------------------------------------------------
# 实现一：教材参考代码写法——字典遍历 + 区间线性匹配
# --------------------------------------------------------------------------
def play_game():
    """转动一次轮盘，返回本次中奖等级。

    randrange(360) 产生 0 ≤ value < 360 的整数，所以右端点用开区间比较，
    区间写成 [108, 360] 也不会越界（360 取不到）。
    """
    value = randrange(360)
    for prize, (lo, hi) in areas.items():
        if lo <= value < hi:
            return prize
    # 三个扇区并集已覆盖 [0, 360)，正常不可能走到这里
    raise AssertionError(f'扇区划分有误，value = {value}')


# --------------------------------------------------------------------------
# 实现二（拓展）：累积度数 + 二分查找
#   扇区数量少时线性匹配更快；扇区一多（几十上百个），
#   先把边界累加成 cum，再用 bisect 定位，复杂度从 O(m) 降到 O(log m)。
# --------------------------------------------------------------------------
sectors = [('一等奖', 30), ('二等奖', 78), ('三等奖', 252)]
cum = []
_total = 0
for _name, _span in sectors:
    _total += _span
    cum.append(_total)          # cum = [30, 108, 360]
assert _total == 360, '扇区总角度必须等于 360°'


def play_game_bisect():
    """累积度数 + 二分查找版本，返回本次中奖等级。

    与实现一取的是同一随机序列（都调一次 randrange(360)），
    所以两种实现在相同种子下能得到逐次一致的结果，便于严格对比。
    """
    value = randrange(360)                      # 0 ≤ value < 360
    return sectors[bisect_right(cum, value)][0]  # 第一个 cum[i] > value 的扇区


# --------------------------------------------------------------------------
# 统计与报告
# --------------------------------------------------------------------------
def run(times, play=play_game):
    """试玩 times 次，返回 {奖项: 中奖次数}。"""
    counts = {}
    for _ in range(times):
        prize = play()
        counts[prize] = counts.get(prize, 0) + 1   # 关键：字典 get() 默认值
    return counts


def report(counts, times, title):
    print("=" * 66)
    print(f'{title}（共试玩 {times} 次）')
    print("=" * 66)
    print(f"{'奖项':<8}{'扇区角度':>10}{'理论概率':>12}{'中奖次数':>12}{'实测频率':>12}{'绝对偏差':>12}")
    print("-" * 66)
    for prize, (lo, hi) in areas.items():
        span = hi - lo
        n = counts.get(prize, 0)
        freq = n / times
        theo = span / 360
        print(f"{prize:<8}{span:>8}°{theo:>11.2%}{n:>12}{freq:>12.2%}{freq - theo:>+12.2%}")
    print("-" * 66)
    print(f"{'合计':<8}{sum(v[1] - v[0] for v in areas.values()):>8}°{1.0:>11.2%}"
          f"{sum(counts.values()):>12}{1.0:>12.2%}")


def main():
    times = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
    if times <= 0:
        print('试玩次数必须是正整数')
        return 1

    seed(20260922)      # 固定种子，结果可复现（Python 默认用 Mersenne Twister）

    print('轮盘扇区模型：')
    for prize, (lo, hi) in areas.items():
        print(f'  {prize}：[{lo:>3}°, {hi:>3}°)  跨度 {hi - lo:>3}°')
    print()

    # 两种实现各自从同一种子重新开始，保证取到的是同一条随机序列
    seed(20260922)
    counts1 = run(times, play_game)
    report(counts1, times, '实现一：字典遍历 + 区间线性匹配')

    print()
    seed(20260922)
    counts2 = run(times, play_game_bisect)
    report(counts2, times, '实现二（拓展）：累积度数 + 二分查找')

    print(f"\n两种实现统计结果一致：{'是 ✔' if counts1 == counts2 else '否 ✘'}"
          f"（同一随机序列下应逐次完全相同——这也验证了两种判区逻辑等价）")
    return 0


if __name__ == '__main__':
    sys.exit(main())
