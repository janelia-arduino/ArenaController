// ----------------------------------------------------------------------------
// ArenaController.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_HPP
#define ARENA_CONTROLLER_HPP

#include <Arduino.h>

#include "ArenaController/Display.hpp"


class ArenaController
{
public:
  void setup();
  void setupFileFromStorage();
  void writeFramesToStorage();
  void showFrameFromStorage();
  void showFrameFromRAM();
private:
  Display display_;
};

#endif
