#include "ui_task.h"
#include "driver/gpio.h"
#include "ssd1306_conf.h"
#include "ssd1306.h"
#include "esp_log.h"
#include <string.h>
#include "shared.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "input_task.h"
#include "neopixel_task.h"

static const char *TAG = "UI";
spi_device_handle_t my_spi_handle;
gpio_config_t oled_dc_cofig;
spi_bus_config_t buscfg;
spi_device_interface_config_t devcfg;

void oled_init(void)
{
    oled_dc_cofig.pin_bit_mask = BIT64(OLED_DC) | BIT64(OLED_RESET);
    oled_dc_cofig.mode = GPIO_MODE_OUTPUT;
    oled_dc_cofig.intr_type = GPIO_INTR_DISABLE;
    oled_dc_cofig.pull_up_en = false;
    oled_dc_cofig.pull_down_en = false;
    ESP_ERROR_CHECK(gpio_config(&oled_dc_cofig));

    buscfg.sclk_io_num = OLED_SCK;
    buscfg.mosi_io_num = OLED_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 128;
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    devcfg.clock_speed_hz = 2000000;
    devcfg.mode = 0;
    devcfg.queue_size = 1;
    devcfg.spics_io_num = OLED_CS;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &my_spi_handle));

    gpio_set_level(OLED_RESET, 0);
    vTaskDelay(pdMS_TO_TICKS(25));
    gpio_set_level(OLED_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(25));

    ssd1306_init();
}

uint8_t center_line(uint8_t line_len, uint8_t char_width_pixels, uint8_t oled_width_pixels)
{
  int16_t start_pixel = (oled_width_pixels - line_len * char_width_pixels) / 2;
  if(start_pixel < 0)
    start_pixel = 0;
  return start_pixel;
}

#define OLED_LINE_BUF_SIZE 32
char oled_line_buf[OLED_LINE_BUF_SIZE];

void draw_nosd(void)
{
    ssd1306_Fill(Black);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "duckyPad Pro");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 10);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "FW V%d.%d.%d", fw_version_major, fw_version_minor, fw_version_patch);
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 25);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "Please Insert");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 55);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "SD card");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 67);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "Instructions:");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 100);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "duckyPad.com");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 112);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    ssd1306_UpdateScreen();
}

void draw_noprofile(void)
{
    ssd1306_Fill(Black);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "No Valid");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 35);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "Profiles");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 47);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "Instructions:");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 80);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
    sprintf(oled_line_buf, "duckyPad.com");
    ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 92);
    ssd1306_WriteString(oled_line_buf, Font_7x10, White);

    ssd1306_UpdateScreen();
}

#define CELL_LONG_SIDE 32
#define CELL_SHORT_SIDE 23
#define TITLE_BAR_HEIGHT 11

void draw_cell_content(uint8_t cell_col, uint8_t cell_row, char* first_line, char* second_line)
{
    if(cell_col >= SW_MATRIX_NUM_COLS || cell_row >= SW_MATRIX_NUM_ROWS)
        return;
    uint8_t cell_pixel_x = cell_col * CELL_LONG_SIDE;
    uint8_t cell_pixel_y = cell_row * CELL_SHORT_SIDE + TITLE_BAR_HEIGHT + 1;
    
    uint8_t first_line_size = strlen(first_line);
    uint8_t second_line_size = strlen(second_line);

    if(first_line_size == 0)
        return;

    uint8_t str_pixel_x, str_pixel_y;
    if(first_line_size <= 4 && second_line_size == 0)
    {
        str_pixel_x = cell_pixel_x + (CELL_LONG_SIDE - (first_line_size * 7)) / 2;
        str_pixel_y = cell_pixel_y + 6;
        ssd1306_SetCursor(str_pixel_x, str_pixel_y);
        ssd1306_WriteString(first_line, Font_7x10, White);
    }
    else if(first_line_size == 5 && second_line_size == 0)
    {
        str_pixel_x = cell_pixel_x + 2;
        str_pixel_y = cell_pixel_y + 7;
        ssd1306_SetCursor(str_pixel_x, str_pixel_y);
        ssd1306_WriteString(first_line, Font_6x8, White);
    }
    else if(first_line_size <= 4 && second_line_size <= 4)
    {
        // first line
        str_pixel_x = cell_pixel_x + (CELL_LONG_SIDE - (first_line_size * 7)) / 2;
        str_pixel_y = cell_pixel_y + 1;
        ssd1306_SetCursor(str_pixel_x, str_pixel_y);
        ssd1306_WriteString(first_line, Font_7x10, White);
        // second line
        str_pixel_x = cell_pixel_x + (CELL_LONG_SIDE - (second_line_size * 7)) / 2;
        str_pixel_y = cell_pixel_y + 1 + 10;
        ssd1306_SetCursor(str_pixel_x, str_pixel_y);
        ssd1306_WriteString(second_line, Font_7x10, White);
    }
    else
    {
        // first line
        str_pixel_x = cell_pixel_x + 1 + (CELL_LONG_SIDE - (first_line_size * 6)) / 2;
        str_pixel_y = cell_pixel_y + 2;
        ssd1306_SetCursor(str_pixel_x, str_pixel_y);
        ssd1306_WriteString(first_line, Font_6x8, White);
        // second line
        str_pixel_x = cell_pixel_x + 1 + (CELL_LONG_SIDE - (second_line_size * 6)) / 2;
        str_pixel_y = cell_pixel_y + 3 + 8;
        ssd1306_SetCursor(str_pixel_x, str_pixel_y);
        ssd1306_WriteString(second_line, Font_6x8, White);
    }
}

void draw_profile(profile_info* this_profile)
{
  ssd1306_Fill(Black);
  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "P%d:%s", this_profile->pf_number, this_profile->pf_name);
	ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 0);
	ssd1306_WriteString(oled_line_buf, Font_7x10, White);
	
  ssd1306_Line(0,10,127,10,White); // title solid line

  for (size_t rrr = 0; rrr < SW_MATRIX_NUM_ROWS; rrr++)
  {
    for (size_t ccc = 0; ccc < SW_MATRIX_NUM_COLS; ccc++)
    {
      uint8_t this_sw_index = rowcol_to_index(rrr, ccc);
      draw_cell_content(ccc, rrr, this_profile->sw_name_firstline[this_sw_index], this_profile->sw_name_secondline[this_sw_index]);
    }
  }
  
  ssd1306_Line(0,33,127,33,White); // horizontal lines
  ssd1306_Line(0,57,127,57,White);
  ssd1306_Line(0,80,127,80,White);
  ssd1306_Line(0,103,127,103,White);
  ssd1306_Line(0,127,127,127,White);

  ssd1306_Line(32,10,32,127,White); // vertical lines
  ssd1306_Line(64,10,64,127,White);
  ssd1306_Line(96,10,96,127,White);

  //ssd1306_dashed_line

  ssd1306_UpdateScreen();
}

void draw_settings(dp_global_settings *dps)
{
  ssd1306_Fill(Black);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "duckyPad Settings");
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 0);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  ssd1306_Line(0,10,128,10,White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "1:Brightness %d%%", brightness_index_to_percent_lookup[dps->brightness_index]);
  ssd1306_SetCursor(0, 15);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "2:Orientation");
  ssd1306_SetCursor(0, 30);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "3:Keyboard Region");
  ssd1306_SetCursor(0, 45);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "%s", dps->current_kb_layout);
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 57);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  ssd1306_UpdateScreen();
}

void draw_nodsb(uint8_t swid)
{
  ssd1306_Fill(Black);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "ERROR");
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 0);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  ssd1306_Line(0,10,128,10,White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "File not found:");
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 20);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "key%d.dsb", swid);
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 35);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "Open & Save");
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 70);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "on PC app");
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 85);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  memset(oled_line_buf, 0, OLED_LINE_BUF_SIZE);
  sprintf(oled_line_buf, "Press Any Key");
  ssd1306_SetCursor(center_line(strlen(oled_line_buf), 7, SSD1306_WIDTH), 115);
  ssd1306_WriteString(oled_line_buf, Font_7x10, White);

  ssd1306_UpdateScreen();
}