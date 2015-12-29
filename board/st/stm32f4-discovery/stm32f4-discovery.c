/*
 * (C) Copyright 2011, 2012, 2013
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 * Alexander Potashev, Emcraft Systems, aspotashev@emcraft.com
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 * Pavel Boldin, Emcraft Systems, paboldin@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>
#include <asm/arch/gpio.h>
#include <asm/arch/fmc.h>
#include <dm/platdata.h>
#include <dm/platform_data/serial_stm32.h>
#include <status_led.h>

DECLARE_GLOBAL_DATA_PTR;

const struct stm32_gpio_ctl gpio_ctl_gpout = {
	.mode = STM32_GPIO_MODE_OUT,
	.otype = STM32_GPIO_OTYPE_PP,
	.speed = STM32_GPIO_SPEED_50M,
	.pupd = STM32_GPIO_PUPD_NO,
	.af = STM32_GPIO_AF0
};

const struct stm32_gpio_ctl gpio_ctl_usart = {
	.mode = STM32_GPIO_MODE_AF,
	.otype = STM32_GPIO_OTYPE_PP,
	.speed = STM32_GPIO_SPEED_50M,
	.pupd = STM32_GPIO_PUPD_UP,
	.af = STM32_GPIO_USART
};

static const struct stm32_gpio_dsc usart_gpio[] = {
	{STM32_GPIO_PORT_X, STM32_GPIO_PIN_TX},	/* TX */
	{STM32_GPIO_PORT_X, STM32_GPIO_PIN_RX},	/* RX */
};

int uart_setup_gpio(void)
{
	int i;
	int rv = 0;

	for (i = 0; i < ARRAY_SIZE(usart_gpio); i++) {
		rv = stm32_gpio_config(&usart_gpio[i], &gpio_ctl_usart);
		if (rv)
			goto out;
	}

out:
	return rv;
}


#define NS2CLK(ns) (_ns2clk(ns, freq))

int dram_init(void)
{
	udelay(100);

	/*
	 * Fill in global info with description of SRAM configuration
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_RAM_BASE;
	gd->bd->bi_dram[0].size  = CONFIG_SYS_RAM_SIZE;

	gd->ram_size = CONFIG_SYS_RAM_SIZE;

	return 0;
}

static const struct stm32_serial_platdata serial_platdata = {
	.base = (struct stm32_usart *)STM32_USART1_BASE,
};

U_BOOT_DEVICE(stm32_serials) = {
	.name = "serial_stm32",
	.platdata = &serial_platdata,
};

u32 get_board_rev(void)
{
	return 0;
}

int board_early_init_f(void)
{
	coloured_LED_init();
	red_led_on();
	green_led_on();

	int res;
	res = uart_setup_gpio();
	if (res)
		return res;

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char serialno[25];
	uint32_t u_id_low, u_id_mid, u_id_high;

	if (!getenv("serial#")) {
		u_id_low  = readl(&STM32_U_ID->u_id_low);
		u_id_mid  = readl(&STM32_U_ID->u_id_mid);
		u_id_high = readl(&STM32_U_ID->u_id_high);
		sprintf(serialno, "%08x%08x%08x",
			u_id_high, u_id_mid, u_id_low);
		setenv("serial#", serialno);
	}

	return 0;
}
#endif
