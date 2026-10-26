# -*- coding: utf-8 -*-
"""
实验一 · 约瑟夫环（Josephus Problem）
===========================================================================
问题描述
    n 个人围成一圈，从 1 开始依次编号。从 1 号开始 1、2、…、k 报数，
    报到 k 的人退出圈子；圈子缩小后，从其下一个人重新从 1 报数，
    如此反复，直到圈中只剩 1 人。问最后留下的是原来的第几号。

教材要求的两种方法（都打印游戏过程）：
    方法一  josephus_slice()     列表切片模拟
    方法二  josephus_iterator()  itertools.cycle 无限迭代器 + 内置 next() 模拟

编程拓展：
    方法三  josephus_recur()     O(n) 数学递推，用于"对拍"验证与性能对比

运行：
    python exp1_josephus.py              默认 n=10, k=3
    python exp1_josephus.py 41 3         自定义 n、k
    python exp1_josephus.py 20000 3 bench  只跑性能对比
===========================================================================
"""
import sys
import time
from itertools import cycle


# --------------------------------------------------------------------------
# 公共小工具
# --------------------------------------------------------------------------
def _preview(seq, limit=15):
    """列表过长时只显示前 limit 个元素——顺带演示列表切片。"""
    if len(seq) <= limit:
        return seq
    return list(seq[:limit]) + ['…']


# --------------------------------------------------------------------------
# 方法一：列表切片模拟
# --------------------------------------------------------------------------
def josephus_slice(n, k, verbose=True):
    """列表切片模拟：把"圆圈"摊平成列表，出圈者用切片拼接删掉。

    数组/列表法核心：出圈位置 i = (i + k - 1) % len(people)。
    删除用 people[:i] + people[i+1:]，一行切片等价于"抹掉第 i 个元素"。

    代价：每次切片都要复制整个列表 → O(n)；共 n-1 轮 → 时间 O(n^2)。
    """
    assert n >= 1 and k >= 1, "n 与 k 必须是正整数"

    people = list(range(1, n + 1))   # 初始队列 [1, 2, ..., n]
    i = 0                            # 当前报数起点在列表中的下标
    rounds = 0

    while len(people) > 1:
        i = (i + k - 1) % len(people)          # 报数 k 次，落在谁头上
        out = people[i]
        people = people[:i] + people[i + 1:]   # 切片拼接删除
        rounds += 1
        if verbose:
            print(f"  第 {rounds:>4} 轮：{out:>4} 号出圈 | 剩余 {len(people):>4} 人 "
                  f"| {_preview(people)}")

    return people[0]


# --------------------------------------------------------------------------
# 方法二：itertools.cycle 迭代器模拟
# --------------------------------------------------------------------------
def josephus_iterator(n, k, verbose=True):
    """迭代器模拟：cycle(range(n)) 是"取完自动回到开头"的无限迭代器，
    天然就是"围成一圈"；用内置 next() 一个一个取人，另用一张布尔表记谁还在圈里。

    代价：已出圈的人仍会被迭代器取到（只能跳过），
          期望空转 O(n·ln n) 次 next()，但每次都是纯下标操作，没有列表搬移。
    """
    assert n >= 1 and k >= 1, "n 与 k 必须是正整数"

    alive = [True] * n      # alive[i] 表示 i 号还在圈里
    it = cycle(range(n))    # 环形无限迭代器
    left = n                # 圈内剩余人数
    count = 0               # 当前累计报数
    rounds = 0

    while left > 1:
        idx = next(it)          # 用 next() 从迭代器取下一个候选
        if not alive[idx]:
            continue            # 已出圈的跳过
        count += 1
        if count == k:          # 报到 k，出圈
            alive[idx] = False
            count = 0
            left -= 1
            rounds += 1
            if verbose:
                print(f"  第 {rounds:>4} 轮：{idx + 1:>4} 号出圈 | 剩余 {left:>4} 人")

    return alive.index(True) + 1


# --------------------------------------------------------------------------
# 方法三（拓展）：O(n) 数学递推
# --------------------------------------------------------------------------
def josephus_recur(n, k):
    """数学递推：记 f(i) 为 i 个人时最后留下者的 0 基下标，则

        f(1) = 0
        f(i) = (f(i-1) + k) % i        i = 2..n

    递推 n 次即得答案（返回 1 基编号）。它不做任何"模拟"，是解析解，
    时间复杂度 O(n)、空间 O(1)，但无法输出游戏过程。
    """
    assert n >= 1 and k >= 1, "n 与 k 必须是正整数"

    pos = 0
    for i in range(2, n + 1):
        pos = (pos + k) % i
    return pos + 1


# --------------------------------------------------------------------------
# 演示 / 对拍 / 性能对比
# --------------------------------------------------------------------------
def demo(n, k):
    print("=" * 68)
    print(f"【方法一】列表切片模拟          n = {n}, k = {k}")
    print("=" * 68)
    ans1 = josephus_slice(n, k)
    print(f"→ 最后留下的是原来的 {ans1} 号\n")

    print("=" * 68)
    print(f"【方法二】itertools.cycle 迭代器模拟   n = {n}, k = {k}")
    print("=" * 68)
    ans2 = josephus_iterator(n, k)
    print(f"→ 最后留下的是原来的 {ans2} 号\n")

    ans3 = josephus_recur(n, k)
    print(f"【方法三·拓展】O(n) 数学递推（无过程输出）：{ans3} 号")
    same = (ans1 == ans2 == ans3)
    print(f"【一致性校验】三种方法结果 {'完全一致 ✔' if same else '不一致 ✘'}")
    return same


def cross_check(limit=200, ks=(1, 2, 3, 5, 7)):
    """对拍：枚举小规模 (n, k)，校验三种实现结果完全一致。"""
    bad = 0
    total = 0
    for n in range(1, limit + 1):
        for k in ks:
            total += 1
            a = josephus_slice(n, k, False)
            b = josephus_iterator(n, k, False)
            c = josephus_recur(n, k)
            if not (a == b == c):
                bad += 1
                print(f"  ✘ n={n}, k={k}: {a} / {b} / {c}")
    print(f"【对拍结果】共 {total} 组 (n≤{limit}, k∈{ks})，"
          f"{'全部通过 ✔' if bad == 0 else f'{bad} 组不一致 ✘'}")
    return bad == 0


def benchmark(n, k):
    print("\n" + "=" * 68)
    print(f"【性能对比】n = {n}, k = {k}")
    print("=" * 68)
    cases = (
        ('列表切片  ', lambda: josephus_slice(n, k, False)),
        ('迭代器    ', lambda: josephus_iterator(n, k, False)),
        ('数学递推  ', lambda: josephus_recur(n, k)),
    )
    for name, fn in cases:
        t0 = time.perf_counter()
        r = fn()
        t1 = time.perf_counter()
        print(f"  {name}：结果 {r:>7}   耗时 {t1 - t0:>8.4f} s")


def main():
    args = sys.argv[1:]
    bench = 'bench' in args
    nums = [int(a) for a in args if a.isdigit()]
    n = nums[0] if len(nums) >= 1 else 10
    k = nums[1] if len(nums) >= 2 else 3

    if n < 1 or k < 1:
        print("n 与 k 必须是正整数")
        return 1

    if bench:
        benchmark(n, k)
        return 0

    demo(n, k)
    print()
    cross_check()
    print()
    benchmark(max(n, 20000), k)   # 小规模看不出差距，性能对比统一用大 n
    return 0


if __name__ == '__main__':
    sys.exit(main())
