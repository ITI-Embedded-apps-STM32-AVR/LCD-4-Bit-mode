/* libs */
#include "STD_TYPES.h"
/* own */
#include "GPIO.h"


/* ############################################### */
/* base address of the module "GPIOC" */
#define GPIOC_BASE_ADDR (0x40011000)

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOC.CRL (Port configuration register low (GPIOn_CRL)), reset value = 0x44444444 */
#define GPIOC_CRL (*((volatile u32*)(GPIOC_BASE_ADDR + 0x0)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOC.IDR (Port input data register (GPIOn_IDR)), reset value = 0x00000000 */
#define GPIOC_IDR (*((volatile u32*)(GPIOC_BASE_ADDR + 0x8)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOC.BSRR (Port bit set/reset register (GPIOn_BSRR)), reset value = 0x00000000 */
#define GPIOC_BSRR (*((volatile u32*)(GPIOC_BASE_ADDR + 0x10)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOC.BRR (Port bit reset register (GPIOn_BRR)), reset value = 0x00000000 */
#define GPIOC_BRR (*((volatile u32*)(GPIOC_BASE_ADDR + 0x14)))
/* === === === === === === === === === === === === */


/* ############################################### */
/* base address of the module "GPIOB" */
#define GPIOB_BASE_ADDR (0x40010C00)

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOB.CRL (Port configuration register low (GPIOn_CRL)), reset value = 0x44444444 */
#define GPIOB_CRL (*((volatile u32*)(GPIOB_BASE_ADDR + 0x0)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOB.IDR (Port input data register (GPIOn_IDR)), reset value = 0x00000000 */
#define GPIOB_IDR (*((volatile u32*)(GPIOB_BASE_ADDR + 0x8)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOB.BSRR (Port bit set/reset register (GPIOn_BSRR)), reset value = 0x00000000 */
#define GPIOB_BSRR (*((volatile u32*)(GPIOB_BASE_ADDR + 0x10)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOB.BRR (Port bit reset register (GPIOn_BRR)), reset value = 0x00000000 */
#define GPIOB_BRR (*((volatile u32*)(GPIOB_BASE_ADDR + 0x14)))
/* === === === === === === === === === === === === */


/* ############################################### */
/* base address of the module "GPIOA" */
#define GPIOA_BASE_ADDR (0x40010800)

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOA.CRL (Port configuration register low (GPIOn_CRL)), reset value = 0x44444444 */
#define GPIOA_CRL (*((volatile u32*)(GPIOA_BASE_ADDR + 0x0)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOA.CRH (Port configuration register high (GPIOn_CRL)), reset value = 0x44444444 */
#define GPIOA_CRH (*((volatile u32*)(GPIOA_BASE_ADDR + 0x4)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOA.IDR (Port input data register (GPIOn_IDR)), reset value = 0x00000000 */
#define GPIOA_IDR (*((volatile u32*)(GPIOA_BASE_ADDR + 0x8)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOA.ODR (Port output data register (GPIOn_ODR)), reset value = 0x00000000 */
#define GPIOA_ODR (*((volatile u32*)(GPIOA_BASE_ADDR + 0xC)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOA.BSRR (Port bit set/reset register (GPIOn_BSRR)), reset value = 0x00000000 */
#define GPIOA_BSRR (*((volatile u32*)(GPIOA_BASE_ADDR + 0x10)))
/* === === === === === === === === === === === === */

/* === === === === === === === === === === === === */
/* base address and masks for register: GPIOA.BRR (Port bit reset register (GPIOn_BRR)), reset value = 0x00000000 */
#define GPIOA_BRR (*((volatile u32*)(GPIOA_BASE_ADDR + 0x14)))
/* === === === === === === === === === === === === */

/* ports masks */
#define PORT_A 1
#define PORT_B 2
#define PORT_C 3

/* mask for pin selection, anded with each pin field: 0xF if pin is selected, 0x0 otherwise
 * also used as a mask for each port used by the pin */
#define PIN_SELECTOR_MASK   0xF
/* mask for pin value in the register IDR, used by readPin API */
#define PIN_VALUE_MASK      0x1
/* mask for pin value: HIGH, LOW, used by writePin API */
#define PIN_USER_VALUE_MASK 0xF
/* masks for HIGH and LOW value, used when comparing each pin state */
#define PIN_VALUE_HIGH 0xF
#define PIN_VALUE_LOW  0

void GPIO_InitPin(const GPIO_t* gpio)
{
   u64 pin_cached = gpio->pin;
   u64 port_cached = gpio->port;
   u64 cr_cached = gpio->mode | gpio->speed;
   register u64 mask_cached;
   u64 cr;

   for (u8 i = 0; i < 16; i++)
   {
      /* if pin is selected */
      if (pin_cached & PIN_SELECTOR_MASK)
      {
         mask_cached = (u64)PIN_SELECTOR_MASK << (i << 2); /* i * 4 */

         switch (port_cached & PIN_SELECTOR_MASK)
         {
         case PORT_A:
            cr = *(volatile u64*)&GPIOA_CRL;
            cr &= ~mask_cached;
            cr |= cr_cached & mask_cached;
            *(volatile u64*)&GPIOA_CRL = cr;
         break;

         case PORT_B:
            cr = *(volatile u64*)&GPIOB_CRL;
            cr &= ~mask_cached;
            cr |= cr_cached & mask_cached;
            *(volatile u64*)&GPIOB_CRL = cr;
         break;

         case PORT_C:
            cr = *(volatile u64*)&GPIOC_CRL;
            cr &= ~mask_cached;
            cr |= cr_cached & mask_cached;
            *(volatile u64*)&GPIOC_CRL = cr;
         break;
         }
      }

      pin_cached >>= 4;
      port_cached >>= 4;
   }
}

void GPIO_WritePin(const GPIO_t* gpio, u64 state)
{
   u64 pin_cached = gpio->pin;
   u64 port_cached = gpio->port;
   register u32 mask_cached;

   for (u8 i = 0; i < 16; i++)
   {
      /* if pin is selected */
      if (pin_cached & PIN_SELECTOR_MASK)
      {
    	 mask_cached = (u32)1 << i;

         switch (state & PIN_USER_VALUE_MASK)
         {
         case PIN_VALUE_HIGH:
             switch (port_cached & PIN_SELECTOR_MASK)
             {
             case PORT_A:
                   GPIOA_BSRR = mask_cached;
             break;

             case PORT_B:
                   GPIOB_BSRR = mask_cached;
			 break;

             case PORT_C:
                   GPIOC_BSRR = mask_cached;
             break;
             }
		 break;

         case PIN_VALUE_LOW:
             switch (port_cached & PIN_SELECTOR_MASK)
             {
             case PORT_A:
                   GPIOA_BRR = mask_cached;
             break;

             case PORT_B:
    			   GPIOB_BRR = mask_cached;
			 break;

             case PORT_C:
                   GPIOC_BRR = mask_cached;
             break;
             }
		 break;
         }
      }

      pin_cached >>= 4;
      port_cached >>= 4;
      state >>= 4;
   }
}

u64 GPIO_ReadPin(const GPIO_t* gpio)
{
   u64 pin_cached = gpio->pin;
   u64 port_cached = gpio->port;
   u32 idrA_cached = GPIOA_IDR;
   u32 idrB_cached = GPIOB_IDR;
   u32 idrC_cached = GPIOC_IDR;
   u64 result = 0;
   register u64 mask_cached;

   for (u8 i = 0; i < 16; i++)
   {
      /* if pin is selected */
      if (pin_cached & PIN_SELECTOR_MASK)
      {
    	 mask_cached = (u64)PIN_VALUE_HIGH << (i << 2); /* i * 4 */

         switch (port_cached & PIN_SELECTOR_MASK)
         {
         case PORT_A:
            if (idrA_cached & PIN_VALUE_MASK)
            {
               result |= mask_cached;
            }
         break;

         case PORT_B:
            if (idrB_cached & PIN_VALUE_MASK)
            {
               result |= mask_cached;
            }
         break;

         case PORT_C:
            if (idrC_cached & PIN_VALUE_MASK)
            {
               result |= mask_cached;
            }
         break;
         }
      }

      pin_cached >>= 4;
      port_cached >>= 4;
      idrA_cached >>= 1;
      idrB_cached >>= 1;
      idrC_cached >>= 1;
   }

   return result;
}

