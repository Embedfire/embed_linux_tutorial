

#ifndef _SYSTEM_MCIMX6Y2_H_
#define _SYSTEM_MCIMX6Y2_H_                      /**< Symbol preventing repeated inclusion */

#include <stdint.h>
#include "MCIMX6Y2.h"
#include "core_ca7.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*system_irq_handler_t) (uint32_t giccIar, void *param);
/**
 * @brief IRQ handle for specific IRQ
 */
typedef struct _sys_irq_handle
{
    system_irq_handler_t irqHandler; /**< IRQ handler for specific IRQ */
    void *userParam;                 /**< User param for handler callback */
} sys_irq_handle_t;


#define DEFAULT_SYSTEM_CLOCK           528000000u            /* Default System clock value */

/**
 * @brief Install IRQ handler for specific IRQ
 *
 * It can't be called at interrupt context to avoid IRQ table corrupt during interrupt preemption
 *
 * @param irq IRQ number corresponds to the installed handler
 * @param handler IRQ handler for the IRQ number
 * @param userParam User specified parameter for IRQ handler callback
 */
void SystemInstallIrqHandler (IRQn_Type irq, system_irq_handler_t handler, void *userParam);




/**
 * @brief Setup the microcontroller system.
 *
 * Typically this function configures the oscillator (PLL) that is part of the
 * microcontroller device. For systems with variable clock speed it also updates
 * the variable SystemCoreClock. SystemInit is called from startup_device file.
 */
void SystemInit (void);




#ifdef __cplusplus
}
#endif



#endif  /* _SYSTEM_MCIMX6Y2_H_ */
