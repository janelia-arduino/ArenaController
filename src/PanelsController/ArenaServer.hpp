// ----------------------------------------------------------------------------
// ArenaServer.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef PANELS_CONTROLLER_ARENA_SERVER_HPP
#define PANELS_CONTROLLER_ARENA_SERVER_HPP

#include <Arduino.h>
#include <NativeEthernet.h>

#include "Constants.hpp"


class ArenaServer
{
public:
  void setup();
  void update();
private:
  void setupSerial();
  void updateSerial();
  // void setupEthernet();
  // void getMacAddress(uint8_t * mac_address);
};

#endif
