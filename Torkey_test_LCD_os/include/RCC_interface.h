#ifndef RCC_INTERFACE_H_
#define RCC_INTERFACE_H_

extern void RCC_Init(void);
extern void RCC_EnablePreipheralClock(u8 Bus, u8 Preiphiral);
extern void RCC_DisablePreipheralClock(u8 Bus, u8 Preiphiral);

#endif /* RCC_INTERFACE_H_ */
