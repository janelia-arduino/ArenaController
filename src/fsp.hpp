#ifndef FSP_HPP
#define FSP_HPP

#include <Arduino.h>
#include "ArenaController.hpp"


struct FSP
{
  static void ArenaController_setup();

  static void Arena_InitialTransition(QP::QActive * const ao);
  static void Arena_ArenaOn_entry(QP::QActive * const ao);
  static void Arena_ArenaOn_exit(QP::QActive * const ao);
  static void Arena_ArenaOn_AllOff_entry(QP::QActive * const ao);
  static void Arena_ArenaOn_AllOn_entry(QP::QActive * const ao);

  static String processStringCommand(String command);
};

#endif // FSP_HPP
