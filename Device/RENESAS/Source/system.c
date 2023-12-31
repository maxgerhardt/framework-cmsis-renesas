/***********************************************************************************************************************
 * Copyright [2020-2023] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics America Inc. and may only be used with products
 * of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.  Renesas products are
 * sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for the selection and use
 * of Renesas products and Renesas assumes no liability.  No license, express or implied, to any intellectual property
 * right is granted by Renesas. This software is protected under all applicable laws, including copyright laws. Renesas
 * reserves the right to change or discontinue this software and/or this documentation. THE SOFTWARE AND DOCUMENTATION
 * IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND TO THE FULLEST EXTENT
 * PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY, INCLUDING WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE SOFTWARE OR
 * DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.  TO THE MAXIMUM
 * EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR DOCUMENTATION
 * (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER, INCLUDING,
 * WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY LOST PROFITS,
 * OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup BSP_MCU
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/
#include <string.h>
#include <stdint.h>
#include <bsp_compiler_support.h>
#include <system.h>
#include <renesas.h>
#if BSP_CFG_EARLY_INIT
 #define BSP_SECTION_EARLY_INIT                 BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT)
#else
 #define BSP_SECTION_EARLY_INIT
#endif
#define BSP_IO_PWPR_PFSWE_OFFSET      (6U)
#define BSP_CFG_C_RUNTIME_INIT ((1))

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Mask to select CP bits( 0xF00000 ) */
#define CP_MASK                                       (0xFU << 20)

/* Startup value for CCR to enable instruction cache, branch prediction and LOB extension */
#define CCR_CACHE_ENABLE                              (0x000E0201)

/* Value to write to OAD register of MPU stack monitor to enable NMI when a stack overflow is detected. */
#define BSP_STACK_POINTER_MONITOR_NMI_ON_DETECTION    (0xA500U)

/* Key code for writing PRCR register. */
#define BSP_PRV_PRCR_KEY                              (0xA500U)
#define BSP_PRV_PRCR_PRC1_UNLOCK                      ((BSP_PRV_PRCR_KEY) | 0x2U)
#define BSP_PRV_PRCR_LOCK                             ((BSP_PRV_PRCR_KEY) | 0x0U)
#define BSP_PRV_STACK_LIMIT                           ((uint32_t) __Vectors[0] - BSP_CFG_STACK_MAIN_BYTES)
#define BSP_PRV_STACK_TOP                             ((uint32_t) __Vectors[0])
#define BSP_TZ_STACK_SEAL_VALUE                       (0xFEF5EDA5)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/

/** System Clock Frequency (Core Clock) */
uint32_t SystemCoreClock BSP_SECTION_EARLY_INIT;

#if defined(__ARMCC_VERSION)
extern uint32_t Image$$BSS$$ZI$$Base;
extern uint32_t Image$$BSS$$ZI$$Length;
extern uint32_t Load$$DATA$$Base;
extern uint32_t Image$$DATA$$Base;
extern uint32_t Image$$DATA$$Length;
extern uint32_t Image$$STACK$$ZI$$Base;
extern uint32_t Image$$STACK$$ZI$$Length;
 #if BSP_FEATURE_BSP_HAS_ITCM
extern uint32_t Load$$ITCM_DATA$$Base;
extern uint32_t Load$$ITCM_PAD$$Limit;
extern uint32_t Image$$ITCM_DATA$$Base;
 #endif
 #if BSP_FEATURE_BSP_HAS_DTCM
extern uint32_t Load$$DTCM_DATA$$Base;
extern uint32_t Load$$DTCM_PAD$$Limit;
extern uint32_t Image$$DTCM_DATA$$Base;
extern uint32_t Image$$DTCM_BSS$$Base;
extern uint32_t Image$$DTCM_BSS_PAD$$ZI$$Limit;
 #endif
#elif defined(__GNUC__)

/* Generated by linker. */
extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackLimit;
extern uint32_t __StackTop;
 #if BSP_FEATURE_BSP_HAS_ITCM
extern uint32_t __itcm_data_init_start;
extern uint32_t __itcm_data_init_end;
extern uint32_t __itcm_data_start;
 #endif
 #if BSP_FEATURE_BSP_HAS_DTCM
extern uint32_t __dtcm_data_init_start;
extern uint32_t __dtcm_data_init_end;
extern uint32_t __dtcm_data_start;
extern uint32_t __dtcm_bss_start;
extern uint32_t __dtcm_bss_end;
 #endif
#elif defined(__ICCARM__)
 #pragma section=".bss"
 #pragma section=".data"
 #pragma section=".data_init"
 #pragma section=".stack"
 #if BSP_FEATURE_BSP_HAS_ITCM
extern uint32_t ITCM_DATA_INIT$$Base;
extern uint32_t ITCM_DATA_INIT$$Limit;
extern uint32_t ITCM_DATA$$Base;
 #endif
 #if BSP_FEATURE_BSP_HAS_DTCM
extern uint32_t DTCM_DATA_INIT$$Base;
extern uint32_t DTCM_DATA_INIT$$Limit;
extern uint32_t DTCM_DATA$$Base;
extern uint32_t DTCM_BSS$$Base;
extern uint32_t DTCM_BSS$$Limit;
 #endif
#endif

/* Initialize static constructors */
#if defined(__ARMCC_VERSION)
extern void (* Image$$INIT_ARRAY$$Base[])(void);
extern void (* Image$$INIT_ARRAY$$Limit[])(void);
#elif defined(__GNUC__)

extern void (* __init_array_start[])(void);

extern void (* __init_array_end[])(void);
#elif defined(__ICCARM__)
extern void __call_ctors(void const *, void const *);

 #pragma section = "SHT$$PREINIT_ARRAY" const
 #pragma section = "SHT$$INIT_ARRAY" const
#endif

extern void * __Vectors[];

extern void R_BSP_SecurityInit(void);

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/

#if BSP_FEATURE_BSP_RESET_TRNG
static void bsp_reset_trng_circuit(void);

#endif

#if BSP_CFG_EARLY_INIT
static void bsp_init_uninitialized_vars(void);

#endif

#if BSP_CFG_C_RUNTIME_INIT
 #if BSP_FEATURE_BSP_HAS_ITCM || BSP_FEATURE_BSP_HAS_DTCM
static void memcpy_64(uint64_t * destination, const uint64_t * source, size_t count);

 #endif
 #if BSP_FEATURE_BSP_HAS_DTCM
static void memset_64(uint64_t * destination, const uint64_t value, size_t count);

 #endif
#endif

#if BSP_CFG_C_RUNTIME_INIT
 #if BSP_FEATURE_BSP_HAS_ITCM
static void bsp_init_itcm(void);

 #endif
 #if BSP_FEATURE_BSP_HAS_DTCM
static void bsp_init_dtcm(void);

 #endif
#endif

/*******************************************************************************************************************//**
 * Initialize the MCU and the runtime environment.
 **********************************************************************************************************************/
void SystemInit (void)
{
#if defined(RENESAS_CORTEX_M85)

    /* Enable the ARM core instruction cache, branch prediction and low-overhead-branch extension.
     * See Section 5.5 of the Cortex-M55 TRM and Section D1.2.9 in the ARMv8-M Architecture Reference Manual */
    SCB->CCR = (uint32_t) CCR_CACHE_ENABLE;
    __DSB();
    __ISB();
#endif

#if __FPU_USED

    /* Enable the FPU only when it is used.
     * Code taken from Section 7.1, Cortex-M4 TRM (DDI0439C) */

    /* Set bits 20-23 (CP10 and CP11) to enable FPU. */
    SCB->CPACR = (uint32_t) CP_MASK;
#endif

#if BSP_TZ_SECURE_BUILD

    /* Seal the main stack for secure projects. Reference:
     * https://developer.arm.com/documentation/100720/0300
     * https://developer.arm.com/support/arm-security-updates/armv8-m-stack-sealing */
    uint32_t * p_main_stack_top = (uint32_t *) __Vectors[0];
    *p_main_stack_top = BSP_TZ_STACK_SEAL_VALUE;
#endif

#if !BSP_TZ_NONSECURE_BUILD

    /* VTOR is in undefined state out of RESET:
     * https://developer.arm.com/documentation/100235/0004/the-cortex-m33-peripherals/system-control-block/system-control-block-registers-summary?lang=en.
     * Set the Secure/Non-Secure VTOR to the vector table address based on the build. This is skipped for non-secure
     * projects because SCB_NS->VTOR is set by the secure project before the non-secure project runs. */
    SCB->VTOR = (uint32_t) &__Vectors;
#endif

#if !BSP_TZ_CFG_SKIP_INIT
 #if BSP_FEATURE_BSP_VBATT_HAS_VBTCR1_BPWSWSTP

    /* Unlock VBTCR1 register. */
    R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_PRC1_UNLOCK;

    /* The VBTCR1.BPWSWSTP must be set after reset on MCUs that have VBTCR1.BPWSWSTP. Reference section 11.2.1
     * "VBATT Control Register 1 (VBTCR1)" and Figure 11.2 "Setting flow of the VBTCR1.BPWSWSTP bit" in the RA4M1 manual
     * R01UM0007EU0110. This must be done before bsp_clock_init because LOCOCR, LOCOUTCR, SOSCCR, and SOMCR cannot
     * be accessed until VBTSR.VBTRVLD is set. */
    R_SYSTEM->VBTCR1 = 1U;
    FSP_HARDWARE_REGISTER_WAIT(R_SYSTEM->VBTSR_b.VBTRVLD, 1U);

    /* Lock VBTCR1 register. */
    R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_LOCK;
 #endif
#endif

#if BSP_FEATURE_TFU_SUPPORTED
    R_BSP_MODULE_START(FSP_IP_TFU, 0U);
#endif

#if BSP_CFG_EARLY_INIT

    /* Initialize uninitialized BSP variables early for use in R_BSP_WarmStart. */
    bsp_init_uninitialized_vars();
#endif

#if BSP_TZ_CFG_SKIP_INIT

    /* Initialize clock variables to be used with R_BSP_SoftwareDelay. */
    bsp_clock_freq_var_init();
#else

 #if BSP_FEATURE_BSP_RESET_TRNG

    /* To prevent an undesired current draw, this MCU requires a reset
     * of the TRNG circuit after the clocks are initialized */

    bsp_reset_trng_circuit();
 #endif
#endif

#if BSP_FEATURE_BSP_HAS_GRAPHICS_DOMAIN

    /* Turn on graphics power domain.
     * This requires MOCO to be enabled, but MOCO is always enabled after bsp_clock_init(). */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_OM_LPC_BATT);
    R_SYSTEM->PDCTRGD = 0;
    (void) R_SYSTEM->PDCTRGD;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_OM_LPC_BATT);
#endif

#if BSP_FEATURE_BSP_HAS_SP_MON

    /* Disable MSP monitoring  */
    R_MPU_SPMON->SP[0].CTL = 0;

    /* Setup NMI interrupt  */
    R_MPU_SPMON->SP[0].OAD = BSP_STACK_POINTER_MONITOR_NMI_ON_DETECTION;

    /* Setup start address  */
    R_MPU_SPMON->SP[0].SA = BSP_PRV_STACK_LIMIT;

    /* Setup end address  */
    R_MPU_SPMON->SP[0].EA = BSP_PRV_STACK_TOP;

    /* Set SPEEN bit to enable NMI on stack monitor exception. NMIER bits cannot be cleared after reset, so no need
     * to read-modify-write. */
    R_ICU->NMIER = R_ICU_NMIER_SPEEN_Msk;

    /* Enable MSP monitoring  */
    R_MPU_SPMON->SP[0].CTL = 1U;
#endif

#if BSP_FEATURE_TZ_HAS_TRUSTZONE
 #if 33U == __CORTEX_M

    /* Use CM33 stack monitor. */
    __set_MSPLIM(BSP_PRV_STACK_LIMIT);
 #else

    /* CM85 stack monitor not yet supported. */
 #endif
#endif

#if BSP_CFG_C_RUNTIME_INIT

    /* Initialize C runtime environment. */
    /* Zero out BSS */
 #if defined(__ARMCC_VERSION)
    memset((uint8_t *) &Image$$BSS$$ZI$$Base, 0U, (uint32_t) &Image$$BSS$$ZI$$Length);
 #elif defined(__GNUC__)
    memset(&__bss_start__, 0U, ((uint32_t) &__bss_end__ - (uint32_t) &__bss_start__));
 #elif defined(__ICCARM__)
    memset((uint32_t *) __section_begin(".bss"), 0U, (uint32_t) __section_size(".bss"));
 #endif

    /* Copy initialized RAM data from ROM to RAM. */
 #if defined(__ARMCC_VERSION)
    memcpy((uint8_t *) &Image$$DATA$$Base, (uint8_t *) &Load$$DATA$$Base, (uint32_t) &Image$$DATA$$Length);
 #elif defined(__GNUC__)
    memcpy(&__data_start__, &__etext, ((uint32_t) &__data_end__ - (uint32_t) &__data_start__));
 #elif defined(__ICCARM__)
    memcpy((uint32_t *) __section_begin(".data"), (uint32_t *) __section_begin(".data_init"),
           (uint32_t) __section_size(".data"));

    /* Copy functions to be executed from RAM. */
  #pragma section=".code_in_ram"
  #pragma section=".code_in_ram_init"
    memcpy((uint32_t *) __section_begin(".code_in_ram"),
           (uint32_t *) __section_begin(".code_in_ram_init"),
           (uint32_t) __section_size(".code_in_ram"));

    /* Copy main thread TLS to RAM. */
  #pragma section="__DLIB_PERTHREAD_init"
  #pragma section="__DLIB_PERTHREAD"
    memcpy((uint32_t *) __section_begin("__DLIB_PERTHREAD"), (uint32_t *) __section_begin("__DLIB_PERTHREAD_init"),
           (uint32_t) __section_size("__DLIB_PERTHREAD_init"));
 #endif

    /* Initialize TCM memory. */
 #if BSP_FEATURE_BSP_HAS_ITCM
    bsp_init_itcm();
 #endif
 #if BSP_FEATURE_BSP_HAS_DTCM
    bsp_init_dtcm();
 #endif

    /* Initialize static constructors */
 #if defined(__ARMCC_VERSION)
    int32_t count = Image$$INIT_ARRAY$$Limit - Image$$INIT_ARRAY$$Base;
    for (int32_t i = 0; i < count; i++)
    {
        void (* p_init_func)(void) =
            (void (*)(void))((uint32_t) &Image$$INIT_ARRAY$$Base + (uint32_t) Image$$INIT_ARRAY$$Base[i]);
        p_init_func();
    }

 #elif defined(__GNUC__)
    int32_t count = __init_array_end - __init_array_start;
    for (int32_t i = 0; i < count; i++)
    {
        __init_array_start[i]();
    }

 #elif defined(__ICCARM__)
    void const * pibase = __section_begin("SHT$$PREINIT_ARRAY");
    void const * ilimit = __section_end("SHT$$INIT_ARRAY");
    __call_ctors(pibase, ilimit);
 #endif
#endif                                 // BSP_CFG_C_RUNTIME_INIT

    /* Initialize SystemCoreClock variable. */
    //SystemCoreClockUpdate();

#if BSP_FEATURE_RTC_IS_AVAILABLE || BSP_FEATURE_RTC_HAS_TCEN || BSP_FEATURE_SYSC_HAS_VBTICTLR

    /* For TZ project, it should be called by the secure application, whether RTC module is to be configured as secure or not. */
 #if !BSP_TZ_NONSECURE_BUILD

    /* Perform RTC reset sequence to avoid unintended operation. */
    R_BSP_Init_RTC();
 #endif
#endif

#if !BSP_CFG_PFS_PROTECT
 #if BSP_TZ_SECURE_BUILD || (BSP_CFG_MCU_PART_SERIES == 8)
    R_PMISC->PWPRS = 0;                              ///< Clear BOWI bit - writing to PFSWE bit enabled
    R_PMISC->PWPRS = 1U << BSP_IO_PWPR_PFSWE_OFFSET; ///< Set PFSWE bit - writing to PFS register enabled
 #else
    R_PMISC->PWPR = 0;                               ///< Clear BOWI bit - writing to PFSWE bit enabled
    R_PMISC->PWPR = 1U << BSP_IO_PWPR_PFSWE_OFFSET;  ///< Set PFSWE bit - writing to PFS register enabled
 #endif
#endif

}

/*******************************************************************************************************************//**
 * Disable TRNG circuit to prevent unnecessary current draw which may otherwise occur when the Crypto module
 * is not in use.
 **********************************************************************************************************************/
#if BSP_FEATURE_BSP_RESET_TRNG
static void bsp_reset_trng_circuit (void)
{
    volatile uint8_t read_port = 0U;
    FSP_PARAMETER_NOT_USED(read_port); /// Prevent compiler 'unused' warning

    /* Release register protection for low power modes (per RA2A1 User's Manual (R01UH0888EJ0100) Figure 11.13 "Example
     * of initial setting flow for an unused circuit") */
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_OM_LPC_BATT);

    /* Enable TRNG function (disable stop function) */
 #if BSP_FEATURE_BSP_HAS_SCE_ON_RA2
    R_BSP_MODULE_START(FSP_IP_TRNG, 0); ///< TRNG Module Stop needs to be started/stopped for RA2 series.
 #elif BSP_FEATURE_BSP_HAS_SCE5
    R_BSP_MODULE_START(FSP_IP_SCE, 0);  ///< TRNG Module Stop needs to be started/stopped for RA4 series.
 #else
  #error "BSP_FEATURE_BSP_RESET_TRNG is defined but not handled."
 #endif

    /* Wait for at least 3 PCLKB cycles */
    read_port = R_PFS->PORT[0].PIN[0].PmnPFS_b.PODR;
    read_port = R_PFS->PORT[0].PIN[0].PmnPFS_b.PODR;
    read_port = R_PFS->PORT[0].PIN[0].PmnPFS_b.PODR;

    /* Disable TRNG function */
 #if BSP_FEATURE_BSP_HAS_SCE_ON_RA2
    R_BSP_MODULE_STOP(FSP_IP_TRNG, 0); ///< TRNG Module Stop needs to be started/stopped for RA2 series.
 #elif BSP_FEATURE_BSP_HAS_SCE5
    R_BSP_MODULE_STOP(FSP_IP_SCE, 0);  ///< TRNG Module Stop needs to be started/stopped for RA4 series.
 #else
  #error "BSP_FEATURE_BSP_RESET_TRNG is defined but not handled."
 #endif

    /* Reapply register protection for low power modes (per RA2A1 User's Manual (R01UH0888EJ0100) Figure 11.13 "Example
     * of initial setting flow for an unused circuit") */
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_OM_LPC_BATT);
}

#endif

#if BSP_CFG_EARLY_INIT

/*******************************************************************************************************************//**
 * Initialize BSP variables not handled by C runtime startup.
 **********************************************************************************************************************/
static void bsp_init_uninitialized_vars (void)
{
    g_protect_pfswe_counter = 0;

    extern volatile uint16_t g_protect_counters[];
    for (uint32_t i = 0; i < 4; i++)
    {
        g_protect_counters[i] = 0;
    }

    extern bsp_grp_irq_cb_t g_bsp_group_irq_sources[];
    for (uint32_t i = 0; i < 16; i++)
    {
        g_bsp_group_irq_sources[i] = 0;
    }

 #if BSP_CFG_EARLY_INIT

    /* Set SystemCoreClock to MOCO */
    SystemCoreClock = BSP_MOCO_HZ;
 #endif
}

#endif

#if BSP_CFG_C_RUNTIME_INIT

 #if (BSP_FEATURE_BSP_HAS_ITCM || BSP_FEATURE_BSP_HAS_DTCM)

/*******************************************************************************************************************//**
 * 64-bit memory copy for Armv8.1-M using low overhead loop instructions.
 *
 * @param[in] destination copy destination start address, word aligned
 * @param[in] source copy source start address, word aligned
 * @param[in] count number of doublewords to copy
 **********************************************************************************************************************/
static void memcpy_64 (uint64_t * destination, const uint64_t * source, size_t count)
{
    uint64_t temp;
    __asm volatile (
        "wls lr, %[count], memcpy_64_loop_end_%=\n"
  #if (defined(__ARMCC_VERSION) || defined(__GNUC__))

        /* Align the branch target to a 64-bit boundary, a CM85 specific optimization. */
        /* IAR does not support alignment control within inline assembly. */
        ".balign 8\n"
  #endif
        "memcpy_64_loop_start_%=:\n"
        "ldrd %Q[temp], %R[temp], [%[source]], #+8\n"
        "strd %Q[temp], %R[temp], [%[destination]], #+8\n"
        "le lr, memcpy_64_loop_start_%=\n"
        "memcpy_64_loop_end_%=:"
        :[destination] "+&r" (destination), [source] "+&r" (source), [temp] "=r" (temp)
        :[count] "r" (count)
        : "lr", "memory"
        );

    /* Suppress IAR warning: "Error[Pe550]: variable "temp" was set but never used" */
    /* "temp" triggers this warning when it lacks an early-clobber modifier, which was removed to allow register reuse with "count". */
    (void) temp;
}

 #endif

 #if BSP_FEATURE_BSP_HAS_DTCM

/*******************************************************************************************************************//**
 * 64-bit memory set for Armv8.1-M using low overhead loop instructions.
 *
 * @param[in] destination set destination start address, word aligned
 * @param[in] value value to set
 * @param[in] count number of doublewords to set
 **********************************************************************************************************************/
static void memset_64 (uint64_t * destination, const uint64_t value, size_t count)
{
    __asm volatile (
        "wls lr, %[count], memset_64_loop_end_%=\n"
  #if (defined(__ARMCC_VERSION) || defined(__GNUC__))

        /* Align the branch target to a 64-bit boundary, a CM85 specific optimization. */
        /* IAR does not support alignment control within inline assembly. */
        ".balign 8\n"
  #endif
        "memset_64_loop_start_%=:\n"
        "strd %Q[value], %R[value], [%[destination]], #+8\n"
        "le lr, memset_64_loop_start_%=\n"
        "memset_64_loop_end_%=:"
        :[destination] "+&r" (destination)
        :[count] "r" (count), [value] "r" (value)
        : "lr", "memory"
        );
}

 #endif

#endif

#if BSP_CFG_C_RUNTIME_INIT

 #if BSP_FEATURE_BSP_HAS_ITCM

/*******************************************************************************************************************//**
 * Initialize ITCM RAM from ROM image.
 **********************************************************************************************************************/
static void bsp_init_itcm (void)
{
    uint64_t       * itcm_destination;
    const uint64_t * itcm_source;
    size_t           count;

  #if defined(__ARMCC_VERSION)
    itcm_destination = (uint64_t *) &Image$$ITCM_DATA$$Base;
    itcm_source      = (uint64_t *) &Load$$ITCM_DATA$$Base;
    count            = ((uint32_t) &Load$$ITCM_PAD$$Limit - (uint32_t) &Load$$ITCM_DATA$$Base) / sizeof(uint64_t);
  #elif defined(__GNUC__)
    itcm_destination = (uint64_t *) &__itcm_data_start;
    itcm_source      = (uint64_t *) &__itcm_data_init_start;
    count            = ((uint32_t) &__itcm_data_init_end - (uint32_t) &__itcm_data_init_start) / sizeof(uint64_t);
  #elif defined(__ICCARM__)
    itcm_destination = (uint64_t *) &ITCM_DATA$$Base;
    itcm_source      = (uint64_t *) &ITCM_DATA_INIT$$Base;
    count            = ((uint32_t) &ITCM_DATA_INIT$$Limit - (uint32_t) &ITCM_DATA_INIT$$Base) / sizeof(uint64_t);
  #endif

    memcpy_64(itcm_destination, itcm_source, count);
}

 #endif

 #if BSP_FEATURE_BSP_HAS_DTCM

/*******************************************************************************************************************//**
 * Initialize DTCM RAM from ROM image and zero initialize DTCM RAM BSS section.
 **********************************************************************************************************************/
static void bsp_init_dtcm (void)
{
    uint64_t       * dtcm_destination;
    const uint64_t * dtcm_source;
    size_t           count;
    uint64_t       * dtcm_zero_destination;
    size_t           count_zero;

  #if defined(__ARMCC_VERSION)
    dtcm_destination      = (uint64_t *) &Image$$DTCM_DATA$$Base;
    dtcm_source           = (uint64_t *) &Load$$DTCM_DATA$$Base;
    count                 = ((uint32_t) &Load$$DTCM_PAD$$Limit - (uint32_t) &Load$$DTCM_DATA$$Base) / sizeof(uint64_t);
    dtcm_zero_destination = (uint64_t *) &Image$$DTCM_BSS$$Base;
    count_zero            = ((uint32_t) &Image$$DTCM_BSS_PAD$$ZI$$Limit - (uint32_t) &Image$$DTCM_BSS$$Base) /
                            sizeof(uint64_t);
  #elif defined(__GNUC__)
    dtcm_destination      = (uint64_t *) &__dtcm_data_start;
    dtcm_source           = (uint64_t *) &__dtcm_data_init_start;
    count                 = ((uint32_t) &__dtcm_data_init_end - (uint32_t) &__dtcm_data_init_start) / sizeof(uint64_t);
    dtcm_zero_destination = (uint64_t *) &__dtcm_bss_start;
    count_zero            = ((uint32_t) &__dtcm_bss_end - (uint32_t) &__dtcm_bss_start) / sizeof(uint64_t);
  #elif defined(__ICCARM__)
    dtcm_destination      = (uint64_t *) &DTCM_DATA$$Base;
    dtcm_source           = (uint64_t *) &DTCM_DATA_INIT$$Base;
    count                 = ((uint32_t) &DTCM_DATA_INIT$$Limit - (uint32_t) &DTCM_DATA_INIT$$Base) / sizeof(uint64_t);
    dtcm_zero_destination = (uint64_t *) &DTCM_BSS$$Base;
    count_zero            = ((uint32_t) &DTCM_BSS$$Limit - (uint32_t) &DTCM_BSS$$Base) / sizeof(uint64_t);
  #endif

    memcpy_64(dtcm_destination, dtcm_source, count);
    memset_64(dtcm_zero_destination, 0, count_zero);
}

 #endif

#endif

/** @} (end addtogroup BSP_MCU) */
