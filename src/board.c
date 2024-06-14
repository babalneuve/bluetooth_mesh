/* board.c - Generic HW interaction hooks */

/*
 * Copyright (c) 2021 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/drivers/gpio.h>
#include "board.h"

/* Locate led0 as alias or label by that name */
#define LED0 DT_ALIAS(led0)

/* Locate button0 as alias or label by sw0 or button0 */
#define BUTTON0 DT_ALIAS(sw0)

#define LED0_DEV DT_PHANDLE(LED0, gpios)
#define LED0_PIN DT_PHA(LED0, gpios, pin)
#define LED0_FLAGS DT_PHA(LED0, gpios, flags)

static const struct device *const led_dev = DEVICE_DT_GET(LED0_DEV);

#define BUTTON0_DEV DT_PHANDLE(BUTTON0, gpios)
#define BUTTON0_PIN DT_PHA(BUTTON0, gpios, pin)
#define BUTTON0_FLAGS DT_PHA(BUTTON0, gpios, flags)

static const struct device *const button_dev = DEVICE_DT_GET(BUTTON0_DEV);
static struct k_work *button_work;

static void button_cb(const struct device *port, struct gpio_callback *cb,
		      gpio_port_pins_t pins)
{
	k_work_submit(button_work);
}

static int led_init(void)
{
	int err;

	if (!device_is_ready(led_dev)) {
		return -ENODEV;
	}

	err = gpio_pin_configure(led_dev, LED0_PIN,
				 LED0_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (err) {
		return err;
	}

	return 0;
}

static int button_init(struct k_work *button_pressed)
{
	int err;

	err = gpio_pin_configure(button_dev, BUTTON0_PIN,
				 BUTTON0_FLAGS | GPIO_INPUT);
	if (err) {
		return err;
	}

	static struct gpio_callback gpio_cb;

	err = gpio_pin_interrupt_configure(button_dev, BUTTON0_PIN,
					   GPIO_INT_EDGE_TO_ACTIVE);
	if (err) {
		return err;
	}

	button_work = button_pressed;

	gpio_init_callback(&gpio_cb, button_cb, BIT(BUTTON0_PIN));
	gpio_add_callback(button_dev, &gpio_cb);

	return 0;
}

int board_init(struct k_work *button_pressed)
{
	int err;

	err = led_init();
	if (err) {
		return err;
	}

	return button_init(button_pressed);
}

void board_led_set(bool val)
{
	gpio_pin_set(led_dev, LED0_PIN, val);
}

void board_output_number(bt_mesh_output_action_t action, uint32_t number)
{
}

void board_prov_complete(void)
{
}
