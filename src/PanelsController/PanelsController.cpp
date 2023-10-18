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
  server_.setup();
}

void PanelsController::update()
{
  server_.update();
}
