# -*- coding: utf-8 -*-
"""
实验 17  模拟汉诺塔问题（Python 版）
------------------------------------------------
教材参考代码复现：
    def hannoi(num, src, dst, temp=None)     # 默认参数 + 递归

对应 C 代码：lab17_hanoi.c
"""

import sys


def hannoi(num, src, dst, temp=None):
    """递归算法：把 num 个盘子从 src 借助 temp 移到 dst"""
    global times                      # 声明用来记录移动次数的变量为全局变量

    # 递归调用函数自身，先把除最后一个盘子之外的所有盘子移动到临时柱子上
    if num < 1:
        return                        # 递归出口
    hannoi(num - 1, src, temp, dst)

    # 移动最后一个盘子
    print('The {0} Times move:{1}=>{2}'.format(times, src, dst))
    towers[dst].append(towers[src].pop())
    for tower in 'ABC':               # 输出 3 个底座上的盘子
        print(tower, ':', towers[tower])
    times += 1

    # 把除最后一个盘子之外的其他盘子从临时底座上移动到目标底座上
    hannoi(num - 1, temp, dst, src)


# 用来记录移动次数的变量
times = 1

# 盘子数量
n = 3

# 初始状态，所有盘子都在 A 座上
towers = {'A': list(range(n, 0, -1)), 'B': [], 'C': []}

# A 表示最初放置盘子的底座，C 是目标底座，B 是临时底座
hannoi(n, 'A', 'C', 'B')
