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

#include "ArenaServer.hpp"


class PanelsController
{
public:
  void setup();
  void update();
private:
  ArenaServer arena_server_;
};

#endif
