// ----------------------------------------------------------------------------
// ArenaServer.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "ArenaServer.hpp"


using namespace panels_controller;

void ArenaServer::setup()
{
  setupSerial();
  // setupEthernet();
}

void ArenaServer::update()
{
  updateSerial();
}

void ArenaServer::setupSerial()
{
  Serial.begin(constants::baud_rate);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }
}

void ArenaServer::updateSerial()
{
  if (Serial.available() > 0)
  {
    // read the incoming byte:
    uint8_t incoming_byte = Serial.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incoming_byte, DEC);
  }
}

// void ArenaServer::setupEthernet()
// {
//   uint8_t mac_address[constants::mac_address_size];
//   getMacAddress(mac_address);

//   // start the Ethernet connection:
//   Serial.println("Initialize Ethernet with DHCP:");
//   if (Ethernet.begin(mac_address) == 0) {
//     Serial.println("Failed to configure Ethernet using DHCP");
//     if (Ethernet.hardwareStatus() == EthernetNoHardware) {
//       Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
//     } else if (Ethernet.linkStatus() == LinkOFF) {
//       Serial.println("Ethernet cable is not connected.");
//     }
//     // no point in carrying on, so do nothing forevermore:
//     while (true) {
//       delay(1);
//     }
//   }
//   // print your local IP address:
//   Serial.print("My IP address: ");
//   Serial.println(Ethernet.localIP());
// }

// void ArenaServer::getMacAddress(uint8_t * mac_address)
// {
//   for(uint8_t by=0; by<2; by++) mac_address[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
//   for(uint8_t by=0; by<4; by++) mac_address[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
//   Serial.printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
// }
