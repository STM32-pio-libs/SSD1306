# SSD1306 Driver Documentation

## Overview
This library provides a hardware-agnostic SSD1306 OLED driver for STM32 projects.  
It supports:
- I2C transport (through a user callback)
- SPI transport (through a user callback)
- Full-screen clear/fill
- Full-frame bitmap drawing
- Adapter callback for external graphics buffers

The driver does not configure MCU peripherals itself. You provide low-level send functions.

## Status Codes
- `OLED_OK` (`0`): Success
- `OLED_ERR_INVALID_ARG` (`-1`): Invalid config/arguments
- `OLED_ERR_IO` (`-2`): Transport callback returned error

## Structs and Types

### `OLED_BusType`
```c
typedef enum{
    OLED_BUS_I2C = 0,
    OLED_BUS_SPI = 1
} OLED_BusType;
```
Selects which transport path the driver uses.

### `OLED_SendI2CFn`
```c
typedef int32_t (*OLED_SendI2CFn)(void *user_context,
                                  uint8_t i2c_address_7bit,
                                  const uint8_t *data,
                                  size_t length);
```
I2C transmit callback contract:
- Called by the driver whenever commands/data must be sent
- `i2c_address_7bit` is the configured SSD1306 7-bit address
- Return `OLED_OK` on success, error code otherwise

### `OLED_SendSPIFn`
```c
typedef int32_t (*OLED_SendSPIFn)(void *user_context,
                                  uint8_t is_data,
                                  const uint8_t *data,
                                  size_t length);
```
SPI transmit callback contract:
- `is_data == 0`: bytes are commands
- `is_data == 1`: bytes are display data
- Typical implementation toggles D/C and optionally CS pins

### `OLED_Config`
```c
typedef struct{
    OLED_BusType bus_type;
    uint16_t width;
    uint16_t height;
    void *user_context;
    union{
        struct{
            uint8_t i2c_address_7bit;
            OLED_SendI2CFn send_fn;
        } i2c;
        struct{
            OLED_SendSPIFn send_fn;
        } spi;
    } transport;
} OLED_Config;
```

Fields:
- `bus_type`: `OLED_BUS_I2C` or `OLED_BUS_SPI`
- `width`, `height`: panel size
- `user_context`: opaque pointer passed into callbacks (HAL handle, bus wrapper, etc.)
- `transport.i2c.i2c_address_7bit`: display address (commonly `0x3C`)
- `transport.i2c.send_fn`: your I2C callback
- `transport.spi.send_fn`: your SPI callback

Validation rules (from driver):
- `cfg` must be non-null
- `width > 0`, `height > 0`
- `height % 8 == 0`
- required callback for selected bus must be non-null

## Public Functions

### `size_t OLED_BufferSize(const OLED_Config *cfg)`
Returns the required full-frame buffer size in bytes:
`cfg->width * (cfg->height / 8)`.

Arguments:
- `cfg`: Pointer to display configuration.

Returns:
- Buffer size in bytes.
- `0` if `cfg == NULL`.

### `int32_t OLED_Init(const OLED_Config *cfg)`
Initializes SSD1306 register sequence and clears display.

Arguments:
- `cfg`: Pointer to a valid `OLED_Config`.

Returns:
- `OLED_OK` on success
- `OLED_ERR_INVALID_ARG` if config invalid
- `OLED_ERR_IO` if transport fails

### `int32_t OLED_Clear(const OLED_Config *cfg)`
Clears entire display (`0x00` pattern).

Arguments:
- `cfg`: Pointer to a valid `OLED_Config`.

Returns:
- `OLED_OK`, `OLED_ERR_INVALID_ARG`, or `OLED_ERR_IO`.

### `int32_t OLED_Fill(const OLED_Config *cfg, uint8_t pattern)`
Fills entire display with one byte pattern (for example, `0xFF` all on).

Arguments:
- `cfg`: Pointer to a valid `OLED_Config`.
- `pattern`: Byte written across whole GRAM.
  - `0x00`: all pixels off
  - `0xFF`: all pixels on
  - Any other value: repeated bit pattern

Returns:
- `OLED_OK`, `OLED_ERR_INVALID_ARG`, or `OLED_ERR_IO`.

### `int32_t OLED_DrawBitmap(const OLED_Config *cfg, const uint8_t *bitmap, size_t length)`
Draws a full frame buffer to display memory.

Arguments:
- `cfg`: Pointer to a valid `OLED_Config`.
- `bitmap`: Pointer to full framebuffer data in SSD1306 page format.
- `length`: Number of bytes in `bitmap`.

Requirements:
- `bitmap != NULL`
- `length == OLED_BufferSize(cfg)`
- Page-oriented layout (8 vertical pixels per byte, LSB at top)

Returns:
- `OLED_OK`, `OLED_ERR_INVALID_ARG`, or `OLED_ERR_IO`.

### `int32_t OLED_GfxFlushCallback(void *context, const uint8_t *buffer, size_t buffer_size, uint16_t width, uint16_t height)`
Adapter for external graphics pipelines:
- Casts `context` to `OLED_Config*` and forwards to `OLED_DrawBitmap`.

Arguments:
- `context`: Must be a valid `OLED_Config*`.
- `buffer`: Full framebuffer pointer.
- `buffer_size`: Must equal `OLED_BufferSize(cfg)`.
- `width`: Must match `cfg->width`.
- `height`: Must match `cfg->height`.

Returns:
- `OLED_OK`, `OLED_ERR_INVALID_ARG`, or `OLED_ERR_IO`.

## Usage

1. Initialize your MCU clocks, GPIO, and I2C or SPI peripheral.
2. Implement the transport callback:
- I2C: `OLED_SendI2CFn`
- SPI: `OLED_SendSPIFn`
3. Fill `OLED_Config` with bus type, resolution, callback, and context pointer.
4. Call `OLED_Init(&oled)` once after peripheral initialization.
5. Use drawing APIs:
- `OLED_Clear(&oled)`
- `OLED_Fill(&oled, pattern)`
- `OLED_DrawBitmap(&oled, framebuffer, framebuffer_size)`

For concrete code, use:
- `examples/base_init_i2c.c`
- `examples/base_init_spi.c`
- `examples/draw_bitmap_i2c.c`

## Example Files
- `examples/base_init_i2c.c`: base I2C initialization and fill
- `examples/base_init_spi.c`: base SPI initialization and fill
- `examples/draw_bitmap_i2c.c`: I2C + generated bitmap drawing
