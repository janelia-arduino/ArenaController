// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.hpp"


using namespace panels_controller;

void PanelsController::setup()
{
  arena_server_.setup();
}

void PanelsController::update()
{
  arena_server_.update();
}
