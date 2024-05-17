// ----------------------------------------------------------------------------
// ArenaController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "ArenaController.hpp"


using namespace arena_controller;

void ArenaController::setup()
{
  Serial.begin(115200);

  display_.setup();
}

void ArenaController::writeFramesToStorage()
{
  display_.writeFramesToStorage();
}

void ArenaController::showFrameFromStorage()
{
  display_.showFrameFromStorage();
}

void ArenaController::showFrameFromRAM()
{
  display_.showFrameFromRAM();
}
