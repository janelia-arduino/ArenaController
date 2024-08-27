// ----------------------------------------------------------------------------
// Interface.hpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#ifndef ARENA_CONTROLLER_INTERFACE_HPP
#define ARENA_CONTROLLER_INTERFACE_HPP

#include <Arduino.h>
#include <NativeEthernet.h>

#include "ArenaController/Constants.hpp"


class ArenaController;

namespace arena_controller
{
class Interface
{
public:
  Interface();
  void update();
private:
  void setup();
  void setupSerial();
  void updateSerial();
  // void setupEthernet();
  // void getMacAddress(uint8_t * mac_address);
  friend class ::ArenaController;
};
}
#endif
