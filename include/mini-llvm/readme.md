# include 目录介绍

codegen 代码核心生成

作用 ： 实现与目标平台无关的通用代码生成逻辑

关键内容：

registerAllocator.h：定义寄存器分配接口，支持线性扫描（linearScanAllocator）

和简单分配（NaiveAllocator）等策略

寄存器分配算法： 将虚拟寄存器映射到物理寄存器，优化指令执行效率


common  公共基础设施

作用：存放垮模块共享的通用工具和基础定义

关键内容：

    诊断工具： Diagnostic.h提供错误/警告的生成机制

    类型系统： Precision.h 定义树脂精度控制逻辑

   运算符库 ops/目录包含算术/逻辑/类型转换等操作的抽象接口


ir  中间表示层

作用： 实现LLVM IR的核心数据结构和操作

关键内容：

    核心类： Argument （函数参数）/ BasicBlock基本块 Instruction 指令构成IR的三级结构


ir _reader  IR解析器

作用： 将文本格式的IR转换为内存对象

关键内容

 词法分析 

符号管理


mc   机器码抽象层

作用： 处理机器码的生成和反汇编

    指令定义

段管理

  目标拓展


mir   机器中间表示

作用：描述物理寄存器分配后的低级指令序列

指令系统

寄存器管理

优化接口


opt   优化层

作用： 实现不同阶段的代码优化

IR优化

机器码优化

Pass管理


target 目标后端

作用：针对特定的硬件架构的后端实现


utils 通用工具库

作用： 提供跨模块的基础工具


ai推荐实现的目录先后顺序

ir -> ir_reader -> opt -> codegen/target-> utils/common

 核心层 ： IR- > IR 解析-> 优化

后端层 ： 通用代码生成 -> 目标架构拓展 -> 机器码生成

工具层 ： 基础设施->  测试验证 -> 用户工具
