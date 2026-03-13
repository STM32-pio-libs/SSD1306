#include "ssd1306.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c;

#define APP_OLED_WIDTH 128U
#define APP_OLED_HEIGHT 64U
#define APP_OLED_BUFFER_SIZE ((APP_OLED_WIDTH * APP_OLED_HEIGHT) / 8U)

static uint8_t demo_bitmap[APP_OLED_BUFFER_SIZE];

static void set_pixel(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t on);
static void build_demo_bitmap(uint8_t *buffer, size_t length);

/*
 * I2C transport callback used by the SSD1306 library.
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
        .width = APP_OLED_WIDTH,
        .height = APP_OLED_HEIGHT,
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

    /*
     * OLED_DrawBitmap expects a full frame buffer:
     * - length must be OLED_BufferSize(&oled) bytes.
     * - memory format is page-oriented (8 vertical pixels per byte).
     */
    build_demo_bitmap(demo_bitmap, sizeof(demo_bitmap));
    if (OLED_DrawBitmap(&oled, demo_bitmap, sizeof(demo_bitmap)) != OLED_OK){
        Error_Handler();
    }

    while(1){
    }
}

static void set_pixel(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t on){
    size_t index;
    uint8_t mask;

    if ((buffer == NULL) || (x >= APP_OLED_WIDTH) || (y >= APP_OLED_HEIGHT)){
        return;
    }

    index = (size_t)x + ((size_t)(y / 8U) * APP_OLED_WIDTH);
    mask = (uint8_t)(1U << (y % 8U));

    if (on != 0U){
        buffer[index] |= mask;
    } else {
        buffer[index] &= (uint8_t)(~mask);
    }
}

static void build_demo_bitmap(uint8_t *buffer, size_t length){
    uint16_t x;
    uint16_t y;

    if ((buffer == NULL) || (length != APP_OLED_BUFFER_SIZE)){
        return;
    }

    memset(buffer, 0, length);

    for (x = 0U; x < APP_OLED_WIDTH; x++){
        set_pixel(buffer, x, 0U, 1U);
        set_pixel(buffer, x, APP_OLED_HEIGHT - 1U, 1U);
    }
    for (y = 0U; y < APP_OLED_HEIGHT; y++){
        set_pixel(buffer, 0U, y, 1U);
        set_pixel(buffer, APP_OLED_WIDTH - 1U, y, 1U);
    }

    for (x = 0U; x < APP_OLED_WIDTH; x++){
        set_pixel(buffer, x, (uint16_t)(x / 2U), 1U);
        set_pixel(buffer, x, (uint16_t)((APP_OLED_HEIGHT - 1U) - (x / 2U)), 1U);
    }
}
