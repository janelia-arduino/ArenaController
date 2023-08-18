// ----------------------------------------------------------------------------
// PanelsController.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_H
#define PANELS_CONTROLLER_H

#include "PanelsController/Constants.h"


class PanelsController
{
public:
  void setup();
  void update();
private:
  const static uint8_t PIXEL_COL_COUNT_QUARTER_PANEL = 8;
  const static uint8_t PIXEL_ROW_COUNT_QUARTER_PANEL = 8;
  uint8_t quarter_panel_message_[PIXEL_COL_COUNT_QUARTER_PANEL];
};

#endif
