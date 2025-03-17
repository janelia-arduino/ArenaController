#ifndef BSP_HPP
#define BSP_HPP
#include "constants.hpp"


struct BSP {
  static void init();

  static void ledOff();
  static void ledOn();

  static void initializeWatchdog();
  static void feedWatchdog();

  static void initializeArena();
  static void initializeFrame();

  static String processStringCommand(String command);

  static bool beginSerial();
  static bool pollSerialCommand();
  static uint8_t readSerialByte();
  static String readSerialStringCommand(uint8_t first_byte);
  static void writeSerialStringResponse(String response);

  static bool beginEthernet();
  static bool checkForEthernetIPAddress();
  static bool beginEthernetServer();
  static bool checkForEthernetClient();
  static bool pollEthernetCommand();
  static void readEthernetBinaryCommand();
  static void writeEthernetBinaryResponse();

  static void displayFrame();

  static uint8_t getRegionRowPanelCountMax();
  static uint8_t getRegionColPanelCountMax();

  static void enablePanelSetSelectPin(uint8_t row_index, uint8_t col_index);
  static void disablePanelSetSelectPin(uint8_t row_index, uint8_t col_index);
  static void transferPanelSet(const uint8_t (*panel_buffer)[], uint8_t panel_buffer_byte_count);

};

#endif // BSP_HPP
