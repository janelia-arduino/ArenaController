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
  uint8_t clock_select_pin = panels_controller::constants::PANEL_CLOCK_SELECT_PINS[panel_row_index_][panel_col_index_];

  Serial.print("clock select pin for panel at row: ");
  Serial.print(panel_row_index_);
  Serial.print(", col: ");
  Serial.print(panel_col_index_);
  Serial.print(" = ");
  Serial.println(clock_select_pin);

  if (++panel_col_index_ == panels_controller::constants::PANEL_COUNT_MAX_PER_ARENA_COL)
  {
    panel_col_index_ = 0;
    if (++panel_row_index_ == panels_controller::constants::PANEL_COUNT_MAX_PER_ARENA_ROW)
    {
      panel_row_index_ = 0;
    }
  }
}
