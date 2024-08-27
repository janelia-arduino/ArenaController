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
  display.setup(storage);
  storage.setup();
  interface.setup();
}
