#include "ssd1306.h"
#include <stdio.h>

SPI_HandleTypeDef hspi;

/*
 * Example GPIOs for SSD1306 in 4-wire SPI mode.
 * Replace with your actual port/pin mapping.
 */
#define OLED_DC_GPIO_Port GPIOB
#define OLED_DC_Pin GPIO_PIN_0
#define OLED_CS_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_1

/*
 * Transport callback used by the SSD1306 library for every SPI transfer.
 *
 * Purpose:
 * - Keep the driver portable: the library asks your code to send bytes,
 *   while your callback controls MCU-specific SPI and GPIO handling.
 *
 * How to use:
 * - is_data == 0: command phase -> set D/C low.
 * - is_data == 1: data phase -> set D/C high.
 * - Optionally control CS around each transfer (shown below).
 * - Return OLED_OK on success, otherwise OLED_ERR_IO / OLED_ERR_INVALID_ARG.
 */
static int32_t oled_send_spi(void *user_context,
                             uint8_t is_data,
                             const uint8_t *data,
                             size_t length)
{
    SPI_HandleTypeDef *spi = (SPI_HandleTypeDef *)user_context;
    if ((spi == NULL) || (data == NULL) || (length == 0U)){
        return OLED_ERR_INVALID_ARG;
    }

    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin,
                      (is_data != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);

    if (HAL_SPI_Transmit(spi, (uint8_t *)data, (uint16_t)length, HAL_MAX_DELAY) != HAL_OK){
        HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
        return OLED_ERR_IO;
    }

    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);
    return OLED_OK;
}

int main(){
    /*
        HAL INIT code
       */

    /*
     * OLED_Config for SPI mode:
     * - bus_type selects SPI transport.
     * - width/height must match panel resolution.
     * - user_context is passed to oled_send_spi (HAL SPI handle here).
     * - transport.spi.send_fn points to your SPI callback.
     */
    OLED_Config oled = {
        .bus_type = OLED_BUS_SPI,
        .width = 128U,
        .height = 64U,
        .user_context = &hspi,
        .transport = {
            .spi = {
                .send_fn = oled_send_spi
            }
        }
    };

    if (OLED_Init(&oled) != OLED_OK){
        Error_Handler();
    }

    OLED_Fill(&oled, 0xF0);

    while(1){
    }
}
