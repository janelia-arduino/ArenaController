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

#include "Server.hpp"


class PanelsController
{
public:
  void setup();
  void update();
private:
  Server server_;
};

#endif
