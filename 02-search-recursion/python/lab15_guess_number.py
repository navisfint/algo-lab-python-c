# -*- coding: utf-8 -*-
"""
实验 15  无界面版猜数游戏设计与实现（Python 版）
------------------------------------------------
教材参考代码复现：
    def guessNumber(maxValue=10, maxTimes=3)

对应 C 代码：lab15_guess_number.c
"""

from random import randint


def guessNumber(maxValue=10, maxTimes=3):
    """随机生成一个 [1, maxValue] 的整数，最多允许猜 maxTimes 次"""
    # 随机生成一个整数
    value = randint(1, maxValue)

    for i in range(maxTimes):
        # 第一次提示 Start to GUESS:，之后提示 Guess again:
        prompt = 'Start to GUESS:' if i == 0 else 'Guess again:'

        # 使用异常处理结构防止输入不是数字的情况
        try:
            x = int(input(prompt))
        except:
            print('Must input an integer between 1 and', maxValue)
        else:
            # 猜对 / 偏大 / 偏小 三种情形
            if x == value:
                print('Congratulations!')
                break
            elif x > value:
                print('Too big')
            else:
                print('Too little')
    else:
        # 循环未被 break（次数用完仍没猜中）时执行
        print('Game over. FAIL.')
        print('The value is', value)


if __name__ == '__main__':
    guessNumber()
