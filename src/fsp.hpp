#ifndef FSP_HPP
#define FSP_HPP

#include <Arduino.h>
#include "ArenaController.hpp"
#include "Arena.hpp"
#include "SerialCommandInterface.hpp"
#include "EthernetCommandInterface.hpp"
#include "Frame.hpp"
#include "Watchdog.hpp"

#include "constants.hpp"


struct FSP
{
  static void ArenaController_setup();

  static void Arena_InitialTransition(QP::QActive * const ao);
  static void Arena_ArenaOn_entry(QP::QActive * const ao);
  static void Arena_ArenaOn_exit(QP::QActive * const ao);
  static void Arena_AllOff_entry(QP::QActive * const ao);
  static void Arena_AllOn_entry(QP::QActive * const ao);

  static void SerialCommandInterface_InitialTransition(QP::QActive * const ao);
  static void SerialCommandInterface_Active_entry(QP::QActive * const ao);
  static void SerialCommandInterface_Active_exit(QP::QActive * const ao);
  static void SerialCommandInterface_NotReady_entry(QP::QActive * const ao);
  static void SerialCommandInterface_PollingForNewCommand_SERIAL_TIMEOUT(QP::QActive * const ao);
  static void SerialCommandInterface_PollingForNewCommand_SERIAL_COMMAND_AVAILABLE(QP::QActive * const ao);
  static bool SerialCommandInterface_PollingForNewCommand_SERIAL_COMMAND_AVAILABLE_if_guard(QP::QActive * const ao);
  static void SerialCommandInterface_PollingForNewCommand_SERIAL_COMMAND_AVAILABLE_else_action(QP::QActive * const ao);
  static void SerialCommandInterface_ProcessingStringCommand_entry(QP::QActive * const ao);
  static void SerialCommandInterface_ProcessingStringCommand_COMMAND_PROCESSED(QP::QActive * const ao);
  static void SerialCommandInterface_ProcessingBinaryCommand_COMMAND_PROCESSED(QP::QActive * const ao);

  static void EthernetCommandInterface_InitialTransition(QP::QActive * const ao);
  static void EthernetCommandInterface_Active_entry(QP::QActive * const ao);
  static void EthernetCommandInterface_Active_exit(QP::QActive * const ao);
  static void EthernetCommandInterface_Uninitialized_SERIAL_TIMEOUT(QP::QActive * const ao);
  static void EthernetCommandInterface_WaitingForIPAddress_SERIAL_TIMEOUT(QP::QActive * const ao);
  static void EthernetCommandInterface_IPAddressFound_SERIAL_TIMEOUT(QP::QActive * const ao);
  static void EthernetCommandInterface_PollingForNewCommand_SERIAL_TIMEOUT(QP::QActive * const ao);

  static void Frame_InitialTransition(QP::QActive * const ao);
  static void Frame_Active_entry(QP::QActive * const ao);
  static void Frame_TransferringPanelSet_entry(QP::QActive * const ao);
  static void Frame_TransferringPanelSet_exit(QP::QActive * const ao);
  static bool Frame_TransferringPanelSet_PANEL_SET_TRANSFERRED_if_guard(QP::QActive * const ao);
  static void Frame_TransferringPanelSet_PANEL_SET_TRANSFERRED_else_action(QP::QActive * const ao);

  static void Watchdog_InitialTransition(QP::QActive * const ao);
  static void Watchdog_Feeding_entry(QP::QActive * const ao);
  static void Watchdog_Feeding_exit(QP::QActive * const ao);
  static void Watchdog_Initialized_WATCHDOG_TIMEOUT(QP::QActive * const ao);

  static String processStringCommand(String command);
};

#endif // FSP_HPP
