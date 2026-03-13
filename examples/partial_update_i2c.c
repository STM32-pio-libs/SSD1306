#include "ssd1306.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c;

#define DEMO_WIDTH 128U
#define DEMO_HEIGHT 64U
#define RECT_WIDTH 32U
#define RECT_HEIGHT 32U
#define RECT_PAGES (RECT_HEIGHT / 8U)
#define RECT_BUFFER_SIZE (RECT_WIDTH * RECT_PAGES)

static uint8_t rect_on[RECT_BUFFER_SIZE];
static uint8_t rect_off[RECT_BUFFER_SIZE];

static void set_pixel_rect(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t on);
static void build_rect_pattern(uint8_t *buffer, size_t length, uint32_t frame);

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
    uint8_t has_prev = 0U;
    uint8_t prev_col = 0U;
    uint8_t prev_page = 0U;
    uint8_t col_start;
    uint8_t page_start;
    uint32_t frame = 0U;

    /*
        HAL clock + GPIO + I2C init code
       */

    OLED_Config oled = {
        .bus_type = OLED_BUS_I2C,
        .width = DEMO_WIDTH,
        .height = DEMO_HEIGHT,
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

    OLED_Clear(&oled);
    memset(rect_off, 0, sizeof(rect_off));

    while(1){
        col_start = (uint8_t)(frame % (DEMO_WIDTH - RECT_WIDTH + 1U));
        page_start = (uint8_t)((frame / 6U) % ((DEMO_HEIGHT / 8U) - RECT_PAGES + 1U));

        if (has_prev != 0U){
            OLED_DrawBitmapRect(&oled,
                                rect_off,
                                sizeof(rect_off),
                                prev_col,
                                (uint8_t)(prev_col + RECT_WIDTH - 1U),
                                prev_page,
                                (uint8_t)(prev_page + RECT_PAGES - 1U));
        }

        build_rect_pattern(rect_on, sizeof(rect_on), frame);
        OLED_DrawBitmapRect(&oled,
                            rect_on,
                            sizeof(rect_on),
                            col_start,
                            (uint8_t)(col_start + RECT_WIDTH - 1U),
                            page_start,
                            (uint8_t)(page_start + RECT_PAGES - 1U));

        prev_col = col_start;
        prev_page = page_start;
        has_prev = 1U;
        frame++;
        HAL_Delay(45);
    }
}

static void set_pixel_rect(uint8_t *buffer, uint16_t x, uint16_t y, uint8_t on){
    size_t index;
    uint8_t mask;

    if ((buffer == NULL) || (x >= RECT_WIDTH) || (y >= RECT_HEIGHT)){
        return;
    }

    index = (size_t)x + ((size_t)(y / 8U) * RECT_WIDTH);
    mask = (uint8_t)(1U << (y % 8U));
    if (on != 0U){
        buffer[index] |= mask;
    } else {
        buffer[index] &= (uint8_t)(~mask);
    }
}

static void build_rect_pattern(uint8_t *buffer, size_t length, uint32_t frame){
    uint16_t x;
    uint16_t y;
    uint16_t wave;

    if ((buffer == NULL) || (length != RECT_BUFFER_SIZE)){
        return;
    }

    memset(buffer, 0, length);

    for (x = 0U; x < RECT_WIDTH; x++){
        set_pixel_rect(buffer, x, 0U, 1U);
        set_pixel_rect(buffer, x, RECT_HEIGHT - 1U, 1U);
    }

    for (y = 0U; y < RECT_HEIGHT; y++){
        set_pixel_rect(buffer, 0U, y, 1U);
        set_pixel_rect(buffer, RECT_WIDTH - 1U, y, 1U);
    }

    for (x = 2U; x < (RECT_WIDTH - 2U); x++){
        wave = (uint16_t)((x + frame) % RECT_HEIGHT);
        set_pixel_rect(buffer, x, wave, 1U);
    }
}
