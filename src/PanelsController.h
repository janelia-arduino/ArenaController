// ----------------------------------------------------------------------------
// PanelsController.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_H
#define PANELS_CONTROLLER_H

#include "Arduino.h"

#include "PanelsController/Constants.h"


class PanelsController
{
public:
  void setup();
  void update();
private:
  uint8_t quarter_panel_message_[panels_controller::constants::COL_PIXEL_COUNT_PER_QUARTER_PANEL];
  long inc_;
};

#endif
