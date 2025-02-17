/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/pinctrl.h>

#include <hal/nrf_gpio.h>

BUILD_ASSERT(((NRF_PULL_NONE == NRF_GPIO_PIN_NOPULL) &&
	      (NRF_PULL_DOWN == NRF_GPIO_PIN_PULLDOWN) &&
	      (NRF_PULL_UP == NRF_GPIO_PIN_PULLUP)),
	      "nRF pinctrl pull settings do not match HAL values");

BUILD_ASSERT(((NRF_DRIVE_S0S1 == NRF_GPIO_PIN_S0S1) &&
	      (NRF_DRIVE_H0S1 == NRF_GPIO_PIN_H0S1) &&
	      (NRF_DRIVE_S0H1 == NRF_GPIO_PIN_S0H1) &&
	      (NRF_DRIVE_H0H1 == NRF_GPIO_PIN_H0H1) &&
	      (NRF_DRIVE_D0S1 == NRF_GPIO_PIN_D0S1) &&
	      (NRF_DRIVE_D0H1 == NRF_GPIO_PIN_D0H1) &&
	      (NRF_DRIVE_S0D1 == NRF_GPIO_PIN_S0D1) &&
	      (NRF_DRIVE_H0D1 == NRF_GPIO_PIN_H0D1) &&
#if defined(GPIO_PIN_CNF_DRIVE_E0E1)
	      (NRF_DRIVE_E0E1 == NRF_GPIO_PIN_E0E1) &&
#endif /* defined(GPIO_PIN_CNF_DRIVE_E0E1) */
	      (1U)),
	     "nRF pinctrl drive settings do not match HAL values");

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_uart)
#define NRF_PSEL_UART(reg, line) ((NRF_UART_Type *)reg)->PSEL##line
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_uarte)
#define NRF_PSEL_UART(reg, line) ((NRF_UARTE_Type *)reg)->PSEL.line
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_spi)
#define NRF_PSEL_SPIM(reg, line) ((NRF_SPI_Type *)reg)->PSEL##line
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_spim)
#define NRF_PSEL_SPIM(reg, line) ((NRF_SPIM_Type *)reg)->PSEL.line
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_spis)
#if defined(NRF51)
#define NRF_PSEL_SPIS(reg, line) ((NRF_SPIS_Type *)reg)->PSEL##line
#else
#define NRF_PSEL_SPIS(reg, line) ((NRF_SPIS_Type *)reg)->PSEL.line
#endif
#endif /* DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_spis) */

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_twi)
#if !defined(TWI_PSEL_SCL_CONNECT_Pos)
#define NRF_PSEL_TWIM(reg, line) ((NRF_TWI_Type *)reg)->PSEL##line
#else
#define NRF_PSEL_TWIM(reg, line) ((NRF_TWI_Type *)reg)->PSEL.line
#endif
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_twim)
#define NRF_PSEL_TWIM(reg, line) ((NRF_TWIM_Type *)reg)->PSEL.line
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_i2s)
#define NRF_PSEL_I2S(reg, line) ((NRF_I2S_Type *)reg)->PSEL.line
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_pdm)
#define NRF_PSEL_PDM(reg, line) ((NRF_PDM_Type *)reg)->PSEL.line
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_pwm)
#define NRF_PSEL_PWM(reg, line) ((NRF_PWM_Type *)reg)->PSEL.line
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_qdec)
#define NRF_PSEL_QDEC(reg, line) ((NRF_QDEC_Type *)reg)->PSEL.line
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_qspi)
#define NRF_PSEL_QSPI(reg, line) ((NRF_QSPI_Type *)reg)->PSEL.line
#endif

/**
 * @brief Configure pin settings.
 *
 * @param pin Pin configuration.
 * @param dir Pin direction.
 * @param input Pin input buffer connection.
 */
__unused static void nrf_pin_configure(pinctrl_soc_pin_t pin,
				       nrf_gpio_pin_dir_t dir,
				       nrf_gpio_pin_input_t input)
{
	/* force input direction and disconnected buffer for low power */
	if (NRF_GET_LP(pin) == NRF_LP_ENABLE) {
		dir = NRF_GPIO_PIN_DIR_INPUT;
		input = NRF_GPIO_PIN_INPUT_DISCONNECT;
	}

	nrf_gpio_cfg(NRF_GET_PIN(pin), dir, input, NRF_GET_PULL(pin),
		     NRF_GET_DRIVE(pin), NRF_GPIO_PIN_NOSENSE);
}

int pinctrl_configure_pins(const pinctrl_soc_pin_t *pins, uint8_t pin_cnt,
			   uintptr_t reg)
{
	for (uint8_t i = 0U; i < pin_cnt; i++) {
		switch (NRF_GET_FUN(pins[i])) {
#if defined(NRF_PSEL_UART)
		case NRF_FUN_UART_TX:
			NRF_PSEL_UART(reg, TXD) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 1);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_UART_RX:
			NRF_PSEL_UART(reg, RXD) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_UART_RTS:
			NRF_PSEL_UART(reg, RTS) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 1);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_UART_CTS:
			NRF_PSEL_UART(reg, CTS) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
#endif /* defined(NRF_PSEL_UART) */
#if defined(NRF_PSEL_SPIM)
		case NRF_FUN_SPIM_SCK:
			NRF_PSEL_SPIM(reg, SCK) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 0);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_SPIM_MOSI:
			NRF_PSEL_SPIM(reg, MOSI) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 0);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_SPIM_MISO:
			NRF_PSEL_SPIM(reg, MISO) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
#endif /* defined(NRF_PSEL_SPIM) */
#if defined(NRF_PSEL_SPIS)
		case NRF_FUN_SPIS_SCK:
			NRF_PSEL_SPIS(reg, SCK) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_SPIS_MOSI:
			NRF_PSEL_SPIS(reg, MOSI) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_SPIS_MISO:
			NRF_PSEL_SPIS(reg, MISO) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_SPIS_CSN:
			NRF_PSEL_SPIS(reg, CSN) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
#endif /* defined(NRF_PSEL_SPIS) */
#if defined(NRF_PSEL_TWIM)
		case NRF_FUN_TWIM_SCL:
			NRF_PSEL_TWIM(reg, SCL) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_TWIM_SDA:
			NRF_PSEL_TWIM(reg, SDA) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
#endif /* defined(NRF_PSEL_TWIM) */
#if defined(NRF_PSEL_I2S)
		case NRF_FUN_I2S_SCK_M:
			NRF_PSEL_I2S(reg, SCK) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 0);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_I2S_SCK_S:
			NRF_PSEL_I2S(reg, SCK) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_I2S_LRCK_M:
			NRF_PSEL_I2S(reg, LRCK) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 0);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_I2S_LRCK_S:
			NRF_PSEL_I2S(reg, LRCK) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_I2S_SDIN:
			NRF_PSEL_I2S(reg, SDIN) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_I2S_SDOUT:
			NRF_PSEL_I2S(reg, SDOUT) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 0);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_I2S_MCK:
			NRF_PSEL_I2S(reg, MCK) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 0);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
#endif /* defined(NRF_PSEL_I2S) */
#if defined(NRF_PSEL_PDM)
		case NRF_FUN_PDM_CLK:
			NRF_PSEL_PDM(reg, CLK) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]), 0);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_PDM_DIN:
			NRF_PSEL_PDM(reg, DIN) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
#endif /* defined(NRF_PSEL_PDM) */
#if defined(NRF_PSEL_PWM)
		case NRF_FUN_PWM_OUT0:
			NRF_PSEL_PWM(reg, OUT[0]) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]),
					   NRF_GET_INVERT(pins[i]));
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_PWM_OUT1:
			NRF_PSEL_PWM(reg, OUT[1]) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]),
					   NRF_GET_INVERT(pins[i]));
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_PWM_OUT2:
			NRF_PSEL_PWM(reg, OUT[2]) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]),
					   NRF_GET_INVERT(pins[i]));
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_PWM_OUT3:
			NRF_PSEL_PWM(reg, OUT[3]) = NRF_GET_PIN(pins[i]);
			nrf_gpio_pin_write(NRF_GET_PIN(pins[i]),
					   NRF_GET_INVERT(pins[i]));
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_OUTPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
#endif /* defined(NRF_PSEL_PWM) */
#if defined(NRF_PSEL_QDEC)
		case NRF_FUN_QDEC_A:
			NRF_PSEL_QDEC(reg, A) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
		case NRF_FUN_QDEC_B:
			NRF_PSEL_QDEC(reg, B) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
		case NRF_FUN_QDEC_LED:
			NRF_PSEL_QDEC(reg, LED) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_CONNECT);
			break;
#endif /* defined(NRF_PSEL_QDEC) */
#if defined(NRF_PSEL_QSPI)
		case NRF_FUN_QSPI_SCK:
			NRF_PSEL_QSPI(reg, SCK) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_QSPI_CSN:
			NRF_PSEL_QSPI(reg, CSN) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
		case NRF_FUN_QSPI_IO0:
			NRF_PSEL_QSPI(reg, IO0) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
		case NRF_FUN_QSPI_IO1:
			NRF_PSEL_QSPI(reg, IO1) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
		case NRF_FUN_QSPI_IO2:
			NRF_PSEL_QSPI(reg, IO2) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
		case NRF_FUN_QSPI_IO3:
			NRF_PSEL_QSPI(reg, IO3) = NRF_GET_PIN(pins[i]);
			nrf_pin_configure(pins[i], NRF_GPIO_PIN_DIR_INPUT,
					  NRF_GPIO_PIN_INPUT_DISCONNECT);
			break;
#endif /* defined(NRF_PSEL_QSPI) */
		default:
			return -ENOTSUP;
		}
	}

	return 0;
}
