// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.h"


using namespace panels_controller;

void PanelsController::setup()
{
  Serial.begin(115200);
  inc_ = 0;
}

void PanelsController::update()
{
  Serial.println(inc_++);
}
