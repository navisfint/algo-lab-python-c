# 算法实验：Python / C 双实现对照

六个经典实验，每个都有 Python 和 C 两份独立实现，外加两份设计思想对比分析。
写这个不是为了交作业，是为了把「同一套逻辑换一种语言重写」时**成本到底转移到了哪里**讲清楚。

## 目录

```
algo-lab-python-c/
├── 01-simulation-probability/     模拟与概率
│   ├── python/  exp1_josephus.py  exp2_roulette.py  exp3_montyhall.py
│   └── c/       exp1_josephus.c   exp2_roulette.c   exp3_montyhall.c
├── 02-search-recursion/           搜索与递归
│   ├── python/  lab15_guess_number.py  lab16_catch_fox.py  lab17_hanoi.py
│   └── c/       lab15_guess_number.c   lab16_catch_fox.c   lab17_hanoi.c
└── docs/
    ├── 设计思想对比_模拟与概率.md
    └── 设计思想对比_搜索与递归.md
```

| 实验 | 问题 | 关键点 |
|---|---|---|
| exp1 约瑟夫环 | n 个人报数出列 | 四种实现复杂度从 O(n²) 到 O(n) |
| exp2 轮盘抽奖 | 概率分布模拟 | 大样本下频率收敛验证 |
| exp3 蒙提霍尔 | 换门是否更优 | 用模拟证明反直觉结论 |
| lab15 猜数 | 二分搜索 | 猜数次数上界 |
| lab16 抓狐狸 | 环形搜索 | 搜索路径与边界处理 |
| lab17 汉诺塔 | 递归分解 | 递归深度与栈开销 |

## 最值钱的一条结论

实验一实测（n = 20000，k = 3）：

| 实现 | 复杂度 | 耗时 |
|---|---|---|
| Python 列表切片 | O(n²) | 0.8955 s |
| C 数组 memmove | O(n²) | 0.1080 s |
| **Python 迭代器法** | **O(n·log n)** | **0.0228 s** |
| C 数组环形标记 | O(n·log n) | 0.0020 s |
| C 循环链表 | O(n·k) | 0.0010 s |
| Python 数学递推 | O(n) | 0.0007 s |
| C 数学递推 | O(n) | ≈0.0000 s |

> **Python 用对算法（0.0228 s）比 C 用错算法（0.1080 s）还快 5 倍。**
> 换语言只拿常数级提速，换算法才降复杂度。

## 另一个值得说的坑

C 版约瑟夫环写到过一个**数组越界却「碰巧算对」**的版本：越界读到的是恰好为 0 的内存，
在小样本上结果正确，跑 1000 组交叉对拍才暴露。同样的写法在 Python 里会直接抛 `IndexError`。

这就是 C 的真实成本——**正确性全靠自己守**，编译器不报、小数据不显、只有对拍能抓。

## 运行

Python 版无第三方依赖：

```bash
python 01-simulation-probability/python/exp1_josephus.py
```

C 版在 MSVC 2022（`/W3`）下零告警编译通过，GCC / MinGW 同样可编：

```bash
gcc -O2 -o exp1 01-simulation-probability/c/exp1_josephus.c
./exp1
```

## 两份对比文档读什么

- **`docs/设计思想对比_模拟与概率.md`** —— 数据结构怎么装（内置容器 vs 手写结构体）、
  内存谁管、错误怎么报，附上表那份实测数据与完整的性能对照。
- **`docs/设计思想对比_搜索与递归.md`** —— 继承与函数、变量作用域、控制结构映射表、
  输入约束与异常处理的差异，末附 Python ↔ C 语法速查。
