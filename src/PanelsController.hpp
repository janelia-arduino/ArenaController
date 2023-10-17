// ----------------------------------------------------------------------------
// PanelsController.h
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_H
#define PANELS_CONTROLLER_H

#include <Arduino.h>

#include "Arena.hpp"


class PanelsController
{
public:
  void setup();
  void writeFramesToCard();
  void displayFrameFromCard();
  void displayFrameFromRAM();
private:
  Arena arena_;
};

#endif
