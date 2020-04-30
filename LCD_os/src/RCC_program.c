#include "STD_TYPES.H"
#include "BIT_MATH.h"
#include "RCC_interface.h"
#include "RCC_register.h"

extern void RCC_Init(void)
{
	/*HSI >> off*
	PLL_>OFF*
	HSC>>ON
	CSS>off*/
	RCC_CR = 0x00010000;

	//SET_BIT(RCC_CR,16);// HSE enable

	/*
	sysclk >>HSE
	AHB,APB!,APb2> no division
	MCO > NOt connected
	*/
	RCC_CFGR = 0x00000001;
	//SET_BIT(RCC_CFGR,0);//change system source sw
	///CLEAR_BIT(RCC_CFGR,1);//change sw sourc  :D
	//CLEAR_BIT(RCC_CR,24);//diable pll
	//pll mmultplication factor by 9 3shan hse
	//SET_BIT(RCC_CFGR,18);
	//SET_BIT(RCC_CFGR,19);
	//SET_BIT(RCC_CFGR,20);
	//CLEAR_BIT(RCC_CFGR,21);
	//pll source ==HSE without division factor
	//SET_BIT(RCC_CFGR,16);
	//divsion of  hse divider =0
	//CLEAR_BIT(RCC_CFGR,17);
	//enable pll
	//SET_BIT(RCC_CR,24);

	//SET_BIT(RCC_CFGR,1);//change system source sw
	//CLEAR_BIT(RCC_CFGR,0);//change sw source  :D
	//nazbot el mco b2a system clock selected
	//CLEAR_BIT(RCC_CFGR,24);
	//CLEAR_BIT(RCC_CFGR,25);
	//SET_BIT(RCC_CFGR,26);
}

//enable leh clck elly da5la leh
extern void RCC_EnablePreipheralClock(u8 Bus, u8 Preiphiral)
{
	switch(Bus)
	{
		case 0:
			SET_BIT(RCC_AHBENR,Preiphiral);
		break;

		case 1:
			SET_BIT(RCC_APB1ENR,Preiphiral);
		break;

		case 2:
			SET_BIT(RCC_APB2ENR,Preiphiral);
		break;

	}
}

extern void RCC_DisablePreipheralClock(u8 Bus, u8 Preiphiral)
{
	switch(Bus)
	{
		case 0:
			CLEAR_BIT(RCC_AHBENR,Preiphiral);
		break;

		case 1:
			CLEAR_BIT(RCC_APB1ENR,Preiphiral);
		break;

		case 2:
			CLEAR_BIT(RCC_APB2ENR,Preiphiral);
		break;

	}
}

