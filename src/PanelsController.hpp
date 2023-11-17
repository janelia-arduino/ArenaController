// ----------------------------------------------------------------------------
// PanelsController.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_HPP
#define PANELS_CONTROLLER_HPP

#include <Arduino.h>

#include "Display.hpp"


class PanelsController
{
public:
  void setup();
  void writeFramesToStorage();
  void showFrameFromStorage();
  void showFrameFromRAM();
private:
  Display display_;
};

#endif
