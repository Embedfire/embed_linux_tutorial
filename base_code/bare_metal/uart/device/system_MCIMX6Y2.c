/*!
 * @file MCIMX6Y2
 * @version 3.0
 * @date 2017-02-28
 * @brief Device specific configuration file for MCIMX6Y2 (implementation file)
 *
 * Provides a system configuration function and a global variable that contains
 * the system frequency. It configures the device and initializes the oscillator
 * (PLL) that is part of the microcontroller device.
 */

#include <stdint.h>
#include "led.h"
#include "system_MCIMX6Y2.h"

uint32_t __VECTOR_TABLE = 0x80002000;


/* Local irq table and nesting level value */
static sys_irq_handle_t irqTable[NUMBER_OF_INT_VECTORS];
static uint32_t irqNesting;



/* ----------------------------------------------------------------------------
   -- Core clock
   ---------------------------------------------------------------------------- */

uint32_t SystemCoreClock = DEFAULT_SYSTEM_CLOCK;

/* ----------------------------------------------------------------------------
   -- SystemInit()
   ---------------------------------------------------------------------------- */

void SystemInit(void)
{
  uint32_t sctlr;
  uint32_t actlr;
 #if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
  uint32_t cpacr;
  uint32_t fpexc;
 #endif

  L1C_InvalidateInstructionCacheAll();
  L1C_InvalidateDataCacheAll();

  actlr = __get_ACTLR();
  actlr = (actlr | ACTLR_SMP_Msk); /* Change to SMP mode before enable DCache */
  __set_ACTLR(actlr);

  sctlr = __get_SCTLR();
  sctlr = (sctlr & ~(SCTLR_V_Msk | /* Use low vector */
                     SCTLR_A_Msk | /* Disable alignment fault checking */
                     SCTLR_M_Msk)) /* Disable MMU */
          | (SCTLR_I_Msk |         /* Enable ICache */
             SCTLR_Z_Msk |         /* Enable Prediction */
             SCTLR_CP15BEN_Msk |   /* Enable CP15 barrier operations */
             SCTLR_C_Msk);         /* Enable DCache */
  __set_SCTLR(sctlr);

  /* Set vector base address */
  GIC_Init();
  __set_VBAR((uint32_t)__VECTOR_TABLE);

  // rgb_led_init();
  // blue_led_on;
 #if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
  cpacr = __get_CPACR();
  /* Enable NEON and FPU */
  cpacr = (cpacr & ~(CPACR_ASEDIS_Msk | CPACR_D32DIS_Msk)) | (3UL << CPACR_cp10_Pos) | (3UL << CPACR_cp11_Pos);
  __set_CPACR(cpacr);

  fpexc = __get_FPEXC();
  fpexc |= 0x40000000UL; /* Enable NEON and FPU */
  __set_FPEXC(fpexc);
 #endif /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */
}



// /* ----------------------------------------------------------------------------
//    -- SystemCoreClockUpdate()
//    ---------------------------------------------------------------------------- */

// void SystemCoreClockUpdate (void) {
//   /* i.MX6ULL systemCoreClockUpdate */
//   uint32_t PLL1SWClock;
//   uint32_t PLL2MainClock;
//   if (CCM->CCSR & CCM_CCSR_PLL1_SW_CLK_SEL_MASK)
//   {
//     if (CCM->CCSR & CCM_CCSR_STEP_SEL_MASK)
//     {
//         /* Get SYS PLL clock*/
//         if (CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_DIV_SELECT_MASK)
//         {
//           PLL2MainClock = (24000000UL * 22UL + (uint64_t)(24000000UL) * (uint64_t)(CCM_ANALOG->PLL_SYS_NUM) / (uint64_t)(CCM_ANALOG->PLL_SYS_DENOM));
//         }
//         else
//         {
//           PLL2MainClock = (24000000UL * 20UL + (uint64_t)(24000000UL) * (uint64_t)(CCM_ANALOG->PLL_SYS_NUM) / (uint64_t)(CCM_ANALOG->PLL_SYS_DENOM));
//         }

//         if (CCM->CCSR & CCM_CCSR_SECONDARY_CLK_SEL_MASK)
//         {
//             /* PLL2 ---> Secondary_clk ---> Step Clock ---> CPU Clock */
//             PLL1SWClock = PLL2MainClock;
//         }
//         else
//         {
//             /* PLL2 PFD2 ---> Secondary_clk ---> Step Clock ---> CPU Clock */
//             PLL1SWClock = ((uint64_t)PLL2MainClock * 18) / ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD2_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD2_FRAC_SHIFT);
//         }
//     }
//     else
//     {
//       /* Osc_clk (24M) ---> Step Clock ---> CPU Clock */
//       PLL1SWClock = 24000000UL;
//     }
//   }
//   else
//   {
//     /* ARM PLL ---> CPU Clock */
//     PLL1SWClock = 24000000UL;
//     PLL1SWClock = ( PLL1SWClock * (CCM_ANALOG->PLL_ARM & CCM_ANALOG_PLL_ARM_DIV_SELECT_MASK) >> CCM_ANALOG_PLL_ARM_DIV_SELECT_SHIFT) >> 1UL;
//    }

//   SystemCoreClock = PLL1SWClock / (((CCM->CACRR & CCM_CACRR_ARM_PODF_MASK) >> CCM_CACRR_ARM_PODF_SHIFT) + 1UL);
// }




/* ----------------------------------------------------------------------------
   -- SystemInstallIrqHandler()
   ---------------------------------------------------------------------------- */

void SystemInstallIrqHandler(IRQn_Type irq, system_irq_handler_t handler, void *userParam)
{
  irqTable[irq].irqHandler = handler;
  irqTable[irq].userParam = userParam;
}

/* ----------------------------------------------------------------------------
   -- SystemIrqHandler()
   ---------------------------------------------------------------------------- */

__attribute__((weak)) void SystemIrqHandler(uint32_t giccIar)
{

  uint32_t intNum = giccIar & 0x3FFUL;

  // rgb_led_init();
  // blue_led_on;

  /* Spurious interrupt ID or Wrong interrupt number */
  if ((intNum == 1023) || (intNum >= NUMBER_OF_INT_VECTORS))
  {
    return;
  }

  irqNesting++;

  // __enable_irq();      /* Support nesting interrupt */

  /* Now call the real irq handler for intNum */
  irqTable[intNum].irqHandler(giccIar, irqTable[intNum].userParam);

  // __disable_irq();

  irqNesting--;
}





// uint32_t SystemGetIRQNestingLevel(void)
// {
//   return irqNesting;
// }

/* Leverage GPT1 to provide Systick */
// void SystemSetupSystick(uint32_t tickRateHz, void *tickHandler, uint32_t intPriority)
// {
//     uint32_t clockFreq;
//     uint32_t spllTmp;

//     /* Install IRQ handler for GPT1 */
//     SystemInstallIrqHandler(GPT1_IRQn, (system_irq_handler_t)(uint32_t)tickHandler, NULL);

//     /* Enable Systick all the time */
//     CCM->CCGR1 |= CCM_CCGR1_CG10_MASK | CCM_CCGR1_CG11_MASK;

//     GPT1->CR = GPT_CR_SWR_MASK;
//     /* Wait reset finished. */
//     while (GPT1->CR == GPT_CR_SWR_MASK)
//     {
//     }
//     /* Use peripheral clock source IPG */
//     GPT1->CR = GPT_CR_WAITEN_MASK | GPT_CR_STOPEN_MASK | GPT_CR_DOZEEN_MASK |
//                GPT_CR_DBGEN_MASK | GPT_CR_ENMOD_MASK | GPT_CR_CLKSRC(1UL);
//     /* Set clock divider to 1 */
//     GPT1->PR = 0;

//     /* Get IPG clock*/
//     /* Periph_clk2_clk ---> Periph_clk */
//     if (CCM->CBCDR & CCM_CBCDR_PERIPH_CLK_SEL_MASK)
//     {
//         switch (CCM->CBCMR & CCM_CBCMR_PERIPH_CLK2_SEL_MASK)
//         {
//             /* Pll3_sw_clk ---> Periph_clk2_clk ---> Periph_clk */
//             case CCM_CBCMR_PERIPH_CLK2_SEL(0U):
//                 clockFreq = (24000000UL * ((CCM_ANALOG->PLL_USB1 & CCM_ANALOG_PLL_USB1_DIV_SELECT_MASK) ? 22U : 20U));
//                 break;

//             /* Osc_clk ---> Periph_clk2_clk ---> Periph_clk */
//             case CCM_CBCMR_PERIPH_CLK2_SEL(1U):
//                 clockFreq = 24000000UL;
//                 break;

//             case CCM_CBCMR_PERIPH_CLK2_SEL(2U):
//             case CCM_CBCMR_PERIPH_CLK2_SEL(3U):
//             default:
//                 clockFreq = 0U;
//                 break;
//         }

//         clockFreq /= (((CCM->CBCDR & CCM_CBCDR_PERIPH_CLK2_PODF_MASK) >> CCM_CBCDR_PERIPH_CLK2_PODF_SHIFT) + 1U);
//     }
//     /* Pll2_main_clk ---> Periph_clk */
//     else
//     {
//         /* Get SYS PLL clock*/
//         if (CCM_ANALOG->PLL_SYS & CCM_ANALOG_PLL_SYS_DIV_SELECT_MASK)
//         {
//           spllTmp = (24000000UL * 22UL + (uint64_t)(24000000UL) * (uint64_t)(CCM_ANALOG->PLL_SYS_NUM) / (uint64_t)(CCM_ANALOG->PLL_SYS_DENOM));
//         }
//         else
//         {
//           spllTmp = (24000000UL * 20UL + (uint64_t)(24000000UL) * (uint64_t)(CCM_ANALOG->PLL_SYS_NUM) / (uint64_t)(CCM_ANALOG->PLL_SYS_DENOM));
//         }

//         switch (CCM->CBCMR & CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK)
//         {
//             /* PLL2 ---> Pll2_main_clk ---> Periph_clk */
//             case CCM_CBCMR_PRE_PERIPH_CLK_SEL(0U):
//                 clockFreq = spllTmp;
//                 break;

//             /* PLL2 PFD2 ---> Pll2_main_clk ---> Periph_clk */
//             case CCM_CBCMR_PRE_PERIPH_CLK_SEL(1U):
//                 clockFreq = ((uint64_t)spllTmp * 18) / ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD2_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD2_FRAC_SHIFT);
//                 break;

//             /* PLL2 PFD0 ---> Pll2_main_clk ---> Periph_clk */
//             case CCM_CBCMR_PRE_PERIPH_CLK_SEL(2U):
//                 clockFreq = ((uint64_t)spllTmp * 18) / ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD0_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD0_FRAC_SHIFT);
//                 break;

//             /* PLL2 PFD2 divided(/2) ---> Pll2_main_clk ---> Periph_clk */
//             case CCM_CBCMR_PRE_PERIPH_CLK_SEL(3U):
//                 clockFreq = ((((uint64_t)spllTmp * 18) / ((CCM_ANALOG->PFD_528 & CCM_ANALOG_PFD_528_PFD2_FRAC_MASK) >> CCM_ANALOG_PFD_528_PFD2_FRAC_SHIFT)) >> 1U);
//                 break;

//             default:
//                 clockFreq = 0U;
//                 break;
//         }
//     }
//     clockFreq /= (((CCM->CBCDR & CCM_CBCDR_AHB_PODF_MASK) >> CCM_CBCDR_AHB_PODF_SHIFT) + 1U);
//     clockFreq /= (((CCM->CBCDR & CCM_CBCDR_IPG_PODF_MASK) >> CCM_CBCDR_IPG_PODF_SHIFT) + 1U);

//     /* Set timeout value and enable interrupt */
//     GPT1->OCR[0] = clockFreq / tickRateHz - 1UL;
//     GPT1->IR = GPT_IR_OF1IE_MASK;

//     /* Set interrupt priority */
//     GIC_SetPriority(GPT1_IRQn, intPriority);
//     /* Enable IRQ */
//     GIC_EnableIRQ(GPT1_IRQn);

//     /* Start GPT counter */
//     GPT1->CR |= GPT_CR_EN_MASK;
// }

// void SystemClearSystickFlag(void)
// {
//     GPT1->SR = GPT_SR_OF1_MASK;
// }

