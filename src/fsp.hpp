#ifndef FSP_HPP
#define FSP_HPP

#include <Arduino.h>

#include "bsp.hpp"
#include "ArenaController.hpp"


struct FSP
{
  static void setupArenaController();

  static String processStringCommand(String command);
};

#endif // FSP_HPP
