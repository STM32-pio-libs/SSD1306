#include "ssd1306.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c;

/*
 * Transport callback used by the SSD1306 library for every I2C transfer.
 *
 * Purpose:
 * - The library is hardware-agnostic and does not call HAL directly.
 * - It calls this function whenever it needs to send command/data bytes.
 *
 * How to use:
 * - Put your MCU-specific I2C transmit code here.
 * - Return OLED_OK on success, otherwise OLED_ERR_IO / OLED_ERR_INVALID_ARG.
 * - Pass this function pointer in OLED_Config.transport.i2c.send_fn.
 */
static int32_t oled_send_i2c(void *user_context,
                             uint8_t i2c_address_7bit,
                             const uint8_t *data,
                             size_t length)
{

    I2C_HandleTypeDef *i2c = (I2C_HandleTypeDef *)user_context;
    if ((i2c == NULL) || (data == NULL) || (length == 0U)){
        return OLED_ERR_INVALID_ARG;
    }

    if (HAL_I2C_Master_Transmit(i2c,
                                (uint16_t)(i2c_address_7bit << 1),
                                (uint8_t *)data,
                                (uint16_t)length,
                                HAL_MAX_DELAY) != HAL_OK)
    {
        return OLED_ERR_IO;
    }

    return OLED_OK;
}


int main(){
    /*
        HAL INIT code
    */



    /*
     * OLED_Config tells the library how to talk to your display.
     *
     * Field meaning:
     * - bus_type: transport mode (I2C in this example).
     * - width/height: panel resolution in pixels.
     * - user_context: opaque pointer passed back to your callback
     *   (here it is the HAL I2C handle).
     * - transport.i2c.i2c_address_7bit: SSD1306 7-bit address (often 0x3C).
     * - transport.i2c.send_fn: your low-level send function.
     */
    OLED_Config oled = {
        .bus_type = OLED_BUS_I2C,
        .width = 128U,
        .height = 64U,
        .user_context = &hi2c,
        .transport = {
            .i2c = {
                .i2c_address_7bit = 0x3CU,
                .send_fn = oled_send_i2c
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
