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

  static bool beginSerial();
  static bool pollSerialCommand();
  static uint8_t readSerialByte();
  static void readSerialStringCommand(char * command_str, char first_char);
  static void writeSerialStringResponse(char * response);

  static bool initializeEthernet();
  static void pollEthernet();
  static bool createEthernetServerConnection();
  static void writeEthernetBinaryResponse(void * connection, uint8_t response[AC::constants::byte_count_per_response_max], uint8_t response_byte_count);

  static void initializeFrame();
  static uint8_t * getFrameBuffer();
  static void fillFrameBufferWithAllOn(uint8_t * buffer,
    uint16_t & buffer_byte_count,
    uint8_t & panel_byte_count,
    uint8_t & region_row_panel_count,
    uint8_t & region_col_panel_count);
  static void enablePanelSetSelectPin(uint8_t row_index, uint8_t col_index);
  static void disablePanelSetSelectPin(uint8_t row_index, uint8_t col_index);
  static void transferPanelSet(const uint8_t * buffer, uint16_t & buffer_position, uint8_t panel_byte_count);

};

#endif // BSP_HPP
