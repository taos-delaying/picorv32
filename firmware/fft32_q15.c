#include "firmware.h"
#include "fft32_q15.h"
#include "dsp/transform_functions.h"

/* 本例使用非 RVV 的四参数接口 */
#if defined(RISCV_MATH_VECTOR_ZVE32X)
#error "This example requires the non-RVV CFFT interface."
#endif
#if defined(RISCV_MATH_DSP) || defined(__riscv_vector) || defined(__riscv_flen)
#error "PicoRV32 requires the scalar integer implementation (no DSP, RVV or FPU)."
#endif

#define FFT_LEN 32

/*
 * 32 个复数，共 64 个 q15_t，占 128 字节。
 * 存储顺序：实部0、虚部0、实部1、虚部1……
 * aligned 是 GNU 工具链的对齐语法。
 */
static q15_t fft_data[2 * FFT_LEN]
    __attribute__((aligned(8)));

static inline uint32_t read_cycle(void)
{
    uint32_t cycle;
    __asm__ volatile ("rdcycle %0" : "=r"(cycle) : : "memory");
    return cycle;
}

static void print_q15(q15_t value)
{
    int32_t signed_value = value;
    if (signed_value < 0) {
        print_chr('-');
        signed_value = -signed_value;
    }
    print_dec((unsigned int)signed_value);
}

int fft32_q15_demo(void)
{
    riscv_cfft_instance_q15 fft_instance;
    unsigned int errors = 0;

    /* 1. 初始化 32 点 Q15 FFT */
    riscv_status status =
        riscv_cfft_init_32_q15(&fft_instance);

    if (status != RISCV_MATH_SUCCESS) {
        print_str("FFT initialization failed\n");
        return 1;
    }

    /* 2. 填充输入：x[n] = 0.5 + j0 */
    for (unsigned n = 0; n < FFT_LEN; ++n) {
        fft_data[2 * n]     = 16384; /* Q1.15 中的 0.5 */
        fft_data[2 * n + 1] = 0;
    }

    /*
     * 3. 执行 FFT，结果覆盖 fft_data
     *
     * 第三个参数 0：正向 FFT
     * 第四个参数 1：重排为正常频率顺序
     */
    uint32_t start_cycle = read_cycle();
    riscv_cfft_q15(&fft_instance, fft_data, 0, 1);
    uint32_t end_cycle = read_cycle();

    print_str("FFT32 call elapsed cycles: ");
    print_dec(end_cycle - start_cycle);
    print_chr('\n');

    /*
     * 4. 打印并验证各频点的原始整数。
     * NMSIS 的 32 点正向 FFT 总共缩放 1/32。
     * 按 Q1.15 解释是 FFT(x)/32；按未归一化 DFT 解释是 Q6.10。
     * 因此本例预期 DC=16384，其他实部和所有虚部均为 0。
     */
    print_str("FFT32 Q15: x[n]=0.5+j0, forward, scale=1/32\n");
    for (unsigned k = 0; k < FFT_LEN; ++k) {
        q15_t expected_real = k == 0 ? 16384 : 0;
        print_str("k=");
        print_dec(k);
        print_str(": real=");
        print_q15(fft_data[2 * k]);
        print_str(", imag=");
        print_q15(fft_data[2 * k + 1]);
        print_chr('\n');
        if (fft_data[2 * k] != expected_real || fft_data[2 * k + 1] != 0)
            ++errors;
    }

    if (errors != 0) {
        print_str("FFT32 FAIL: incorrect bins=");
        print_dec(errors);
        print_chr('\n');
        return 1;
    }
    print_str("FFT32 PASS\n");
    return 0;
}
