PicoRV32 上的 32 点 Q15 软件 FFT
================================

此目录保留可移至服务器的 FFT 源码及构建配置。
编译与仿真由用户在服务器中执行。

调用链：firmware/start.S -> hello() -> fft32_q15_demo()
FFT 程序：firmware/fft32_q15.c、firmware/fft32_q15.h
源码依赖：third_party/nmsis；固定版本及许可证见其中的 SOURCE.txt、LICENSE。

程序使用普通整数指令实现 FFT，不需要 RVV、Nuclei DSP 扩展或硬件浮点。
FFT 和 NMSIS 的编译目标是 RV32IMC/ILP32，对应官方测试台的乘除法和
压缩指令配置。Makefile 会编译所需库源码，并在链接时删除未用表格；
firmware/sections.lds 保留复位入口。

当前输入共 32 个复数，实部均为 16384（Q1.15 的 0.5），虚部均为 0。
预期 k=0: real=16384, imag=0；其余频点的实部、虚部均为 0。
NMSIS 的 32 点正向 FFT 总缩放为 1/32。
按 Q1.15 解读是 FFT(x)/32；按未归一化 DFT 解读则是 Q6.10。
更换输入后，需要同步修改 fft32_q15_demo() 内的期望值检查。

每次 FFT 调用前后各读取一次 rdcycle，返回后输出 “FFT32 call elapsed cycles: N”。
N 是两次采样的原始周期差，包含函数调用、访存等待、采样边界开销和窗口内发生的中断。
初始化、输入填充、结果打印及校验均位于计时窗口之外。32 位无符号差值适用于
单次耗时小于 2^32 周期的调用。

服务器需要自行提供支持 RV32IMC/ILP32 的 RISC-V 裸机 GCC/Binutils
及标准头文件，以及 Make、Python 3。运行上游仿真目标还需 Icarus Verilog。
通过 TOOLCHAIN_PREFIX 指定服务器工具链；例如在仓库根目录执行：

  make firmware/firmware.hex TOOLCHAIN_PREFIX=riscv64-unknown-elf-
  make test TOOLCHAIN_PREFIX=riscv64-unknown-elf-

以上为供用户自行执行的命令。工具链前缀与路径应按服务器实际安装调整。
不同版本工具链可能需要调整上游计数器指令的汇编参数。
上游正常结束可能打印 EBREAK/TRAP，以 FFT32 PASS、ALL TESTS PASSED.
及实际数值检查为准；整套测试的周期数不能直接当作 FFT 函数耗时。
