# STM32-SSD1306

Portable SSD1306 OLED driver for STM32 using STM32Cube HAL, with pluggable I2C/SPI transport callbacks.

## Features
- Transport-agnostic architecture (I2C and SPI)
- No hard dependency on a specific STM32 peripheral instance
- Config-driven display setup through `OLED_Config`
- Built-in SSD1306 initialization sequence
- Full-screen operations:
  - `OLED_Clear`
  - `OLED_Fill`
  - `OLED_DrawBitmap`
- Graphics adapter:
  - `OLED_GfxFlushCallback` for external framebuffer pipelines

## Project Layout
- `include/ssd1306.h`: public API
- `src/ssd1306.c`: driver implementation
- `examples/base_init_i2c.c`: I2C transport example
- `examples/base_init_spi.c`: SPI transport example
- `examples/draw_bitmap_i2c.c`: I2C bitmap rendering example
- `documentation.md`: detailed API/usage documentation

## Quick Start
1. Configure your STM32 clock/GPIO/bus peripheral (I2C or SPI).
2. Implement a transport callback (`OLED_SendI2CFn` or `OLED_SendSPIFn`).
3. Fill `OLED_Config` with display size and callback.
4. Call `OLED_Init(&oled)`.
5. Render using `OLED_Clear`, `OLED_Fill`, or `OLED_DrawBitmap`.

### Minimal I2C Flow
```c
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

if (OLED_Init(&oled) != OLED_OK) {
    Error_Handler();
}
OLED_Fill(&oled, 0x00U);
```

### Minimal SPI Flow
```c
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
```

## Return Codes
- `OLED_OK`: success
- `OLED_ERR_INVALID_ARG`: bad arguments/config
- `OLED_ERR_IO`: bus transfer failure

## Notes
- For I2C, use SSD1306 7-bit address (`0x3C` is common).
- For SPI, your callback should drive D/C (command/data select) and optionally CS.
- `OLED_DrawBitmap` requires a full framebuffer of exactly `OLED_BufferSize(cfg)` bytes.

See [documentation.md](documentation.md) for complete API details.
