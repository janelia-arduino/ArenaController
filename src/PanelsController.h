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

#include "Arena.h"


class PanelsController
{
public:
  void setup();
  void update();

  void writeFramesToCard();
private:
  Arena arena_;
};

#endif
