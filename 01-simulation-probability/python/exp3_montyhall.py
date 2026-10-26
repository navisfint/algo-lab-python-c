# -*- coding: utf-8 -*-
"""
实验三 · 蒙提霍尔悖论（Monty Hall Problem）
===========================================================================
游戏规则
    有 3 道门，其中 1 道门后是汽车，另 2 道门后是山羊。
    玩家先选 1 道门 → 主持人知道每道门后是什么，故意打开另外两扇门中
    一扇后面是山羊的门（例如 3 号门，后面是一只山羊）→
    主持人问："你想改选 2 号门吗？" → 玩家做出最终选择 → 揭晓输赢。

反直觉之处（悖论所在）
    主持人的开门不是随机的，他"必然"开一扇羊门，这个动作携带了信息：
    坚持原选择胜率 = 1/3，换门胜率 = 2/3，换门严格更优。

知识点对应（实验目的）
    (1)(2) 字典方法与集合运算：doors 字典 + set 差集求"可开的门"
    (3)(7) 异常处理结构：try/except 拦住非法输入
    (4)    assert 断言：把"输入必须合法"写成断言
    (5)    for 循环遍历序列元素
    (6)    while 循环与退出条件设计
    (8)    多函数定义与调用

运行：
    python exp3_montyhall.py
===========================================================================
"""
import random
import sys

GOAT, CAR = '山羊', '汽车'
DOORS = (1, 2, 3)
THEORY_SWITCH = 2 / 3        # 换门理论胜率
THEORY_STAY = 1 / 3          # 坚持理论胜率


# --------------------------------------------------------------------------
# 游戏建模
# --------------------------------------------------------------------------
def init_doors():
    """构造三扇门：{门号: 奖品}，随机把汽车放到其中一扇门后。"""
    doors = {d: GOAT for d in DOORS}     # 先让每扇门后都是羊
    doors[random.choice(DOORS)] = CAR    # 再把车塞进随机一扇门
    return doors


def get_choice(doors):
    """读取玩家选择的门号，异常处理拦住一切非法输入。"""
    while True:
        raw = input(f'请选择一扇门 {list(doors)} > ').strip()
        try:
            choice = int(raw)                       # 非数字 → ValueError
            assert choice in doors, f'不存在 {choice} 号门'   # 越界 → AssertionError
        except (ValueError, AssertionError) as exc:
            print(f'  ✘ 非法输入（{exc}），请输入 1 / 2 / 3。')
            continue
        return choice


def host_open(doors, choice):
    """主持人打开一扇"不是玩家所选、且后面是山羊"的门（集合运算求候选）。"""
    candidates = set(doors) - {choice} - {d for d, v in doors.items() if v == CAR}
    # 玩家选到羊 → 候选只剩另一扇羊门（唯一解，主持人没得挑）
    # 玩家选到车 → 候选是剩下两扇羊门（主持人可任选其一）
    assert candidates, '候选集合不应为空'
    return random.choice(sorted(candidates))


def other_closed_door(doors, choice, opened):
    """剩下那扇"既没被玩家选、也没被打开"的门——换门就改选它。"""
    remaining = set(doors) - {choice} - {opened}
    assert len(remaining) == 1, '只剩两扇未开的门，差集应恰好一个元素'
    return remaining.pop()


def ask_switch(doors, choice, opened):
    """把主持人的话说完，并读取玩家是否换门。"""
    target = other_closed_door(doors, choice, opened)
    while True:
        raw = input(f'主持人打开了 {opened} 号门，后面是{doors[opened]}。'
                    f'你想改选 {target} 号门吗？(y/n) > ').strip().lower()
        try:
            assert raw in ('y', 'n', 'yes', 'no'), '只接受 y / n'
        except AssertionError as exc:
            print(f'  ✘ {exc}，请重新输入。')
            continue
        return raw.startswith('y')


# --------------------------------------------------------------------------
# 人工玩一局 / 自动模拟
# --------------------------------------------------------------------------
def play_manual():
    """人工玩一局，完整打印过程与结果。"""
    doors = init_doors()
    print(f'\n三扇门已就绪 {list(doors)}（门后是什么，你看不到）')
    choice = get_choice(doors)
    print(f'你的选择：{choice} 号门')
    opened = host_open(doors, choice)
    switch = ask_switch(doors, choice, opened)
    final = other_closed_door(doors, choice, opened) if switch else choice
    won = doors[final] == CAR

    print(f'\n最终打开 {final} 号门，后面是【{doors[final]}】')
    print(f'{"🎉 恭喜，你赢得了汽车！" if won else "😅 很遗憾，你得到的是一只山羊。"}')
    print(f'（本局复盘：玩家{"换门" if switch else "坚持原选择"}｜'
          f'汽车原本在 {[d for d, v in doors.items() if v == CAR][0]} 号门后）')
    return won


def simulate(times):
    """自动模拟：同一批局面下，分别统计"坚持"与"换门"的胜局数。"""
    win_stay = win_switch = 0
    for _ in range(times):
        doors = init_doors()
        choice = random.choice(DOORS)                        # 玩家随机选
        opened = host_open(doors, choice)                    # 主持人必开一扇羊门
        if doors[choice] == CAR:
            win_stay += 1
        if doors[other_closed_door(doors, choice, opened)] == CAR:
            win_switch += 1
    return win_stay, win_switch


def report(times, win_stay, win_switch):
    print('\n' + '=' * 66)
    print(f'自动模拟结果（共 {times} 局，玩家随机选门，主持人必开羊门）')
    print('=' * 66)
    print(f"{'策略':<12}{'胜局数':>10}{'实测胜率':>12}{'理论胜率':>12}{'偏差':>12}")
    print('-' * 66)
    p_stay = win_stay / times
    p_switch = win_switch / times
    print(f"{'坚持原选择':<12}{win_stay:>10}{p_stay:>12.2%}{THEORY_STAY:>12.2%}"
          f"{p_stay - THEORY_STAY:>+12.2%}")
    print(f"{'换门':<12}{win_switch:>10}{p_switch:>12.2%}{THEORY_SWITCH:>12.2%}"
          f"{p_switch - THEORY_SWITCH:>+12.2%}")
    print('-' * 66)
    print(f'两策略胜局数之和 = {win_stay + win_switch} = 总局数 → '
          f'每局必有一种策略赢（换门必等于"坚持"的相反结果）')
    print(f'结论：换门胜率 {p_switch:.2%} 明显高于坚持 {p_stay:.2%}，'
          f'换门是最优策略 ✔')


def main():
    random.seed(20260922)      # 固定种子，模拟结果可复现
    print('=' * 66)
    print('实验三 · 蒙提霍尔悖论（Monty Hall Problem）模拟')
    print('=' * 66)

    while True:
        print('\n请选择功能：')
        print('  1) 人工玩一局（体验主持人的提问）')
        print('  2) 自动模拟，对比「坚持」与「换门」的胜率')
        print('  0) 退出')
        try:
            opt = input('输入编号 > ').strip()
            assert opt in ('0', '1', '2'), f'没有 {opt} 这个选项'
        except (AssertionError, EOFError):
            print('  ✘ 非法输入，请输入 0 / 1 / 2。')
            continue
        except KeyboardInterrupt:
            break

        if opt == '0':
            break
        if opt == '1':
            play_manual()
            continue

        try:
            times = int(input('模拟局数（回车默认 10000）> ').strip() or 10000)
            assert times > 0, '局数必须为正'
        except (ValueError, AssertionError) as exc:
            print(f'  ✘ 局数非法（{exc}），按 10000 处理。')
            times = 10000
        except EOFError:
            break
        report(times, *simulate(times))

    print('\n再见。')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except EOFError:
        sys.exit(0)
