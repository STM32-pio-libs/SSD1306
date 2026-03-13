#include "ssd1306.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c;

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
                                HAL_MAX_DELAY) != HAL_OK){
        return OLED_ERR_IO;
    }

    return OLED_OK;
}

int main(){
    /*
        HAL clock + GPIO + I2C init code
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

    if (OLED_Fill(&oled, 0xAAU) != OLED_OK){
        Error_Handler();
    }
    HAL_Delay(600);

    /* Invert mode ON/OFF */
    OLED_SetInvert(&oled, 1U);
    HAL_Delay(500);
    OLED_SetInvert(&oled, 0U);
    HAL_Delay(300);

    /* Entire display ON ignores GRAM contents (A5), then resume (A4) */
    OLED_SetEntireDisplayOn(&oled, 1U);
    HAL_Delay(500);
    OLED_SetEntireDisplayOn(&oled, 0U);
    HAL_Delay(300);

    /* Contrast control */
    OLED_SetContrast(&oled, 0x10U);
    HAL_Delay(400);
    OLED_SetContrast(&oled, 0xFFU);
    HAL_Delay(400);
    OLED_SetContrast(&oled, 0x7FU);

    /* Panel OFF/ON */
    OLED_DisplayOn(&oled, 0U);
    HAL_Delay(300);
    OLED_DisplayOn(&oled, 1U);

    while(1){
    }
}
