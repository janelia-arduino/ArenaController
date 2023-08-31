// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.h"


using namespace panels_controller;

void PanelsController::setup()
{
  Serial.begin(115200);
  panel_row_index_ = 0;
  panel_col_index_ = 0;
}

void PanelsController::update()
{
  long panel_spi_address = (long)panels_controller::constants::PANEL_SPI_PTRS[panel_row_index_][panel_col_index_];
  uint8_t panel_clock_select_pin = panels_controller::constants::PANEL_CLOCK_SELECT_PINS[panel_row_index_][panel_col_index_];

  Serial.print("panel spi address at row: ");
  Serial.print(panel_row_index_);
  Serial.print(", col: ");
  Serial.print(panel_col_index_);
  Serial.print(" = ");
  Serial.println(panel_spi_address);

  Serial.print("panel clock select pin at row: ");
  Serial.print(panel_row_index_);
  Serial.print(", col: ");
  Serial.print(panel_col_index_);
  Serial.print(" = ");
  Serial.println(panel_clock_select_pin);

  if (++panel_col_index_ == panels_controller::constants::ARENA_PANEL_COUNT_MAX_PER_COL)
  {
    panel_col_index_ = 0;
    if (++panel_row_index_ == panels_controller::constants::ARENA_PANEL_COUNT_MAX_PER_ROW)
    {
      panel_row_index_ = 0;
    }
  }
}
