// ----------------------------------------------------------------------------
// Display.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Display.hpp"


using namespace arena_controller;

Display::Display() :
spi_settings_(SPISettings(constants::spi_clock_speed, constants::spi_bit_order, constants::spi_data_mode))
{}

void Display::setSpiClockSpeed(uint32_t spi_clock_speed)
{
  spi_settings_ = SPISettings(clock_speed, constants::spi_bit_order, constants::spi_data_mode);
}

void Display::showFrame()
{
  // delay(1000);
  // Serial.print("beginTransferFrame();");
  beginTransferFrame();
  // Serial.print("transferFrame(");
  // Serial.print(storage_ptr_->tpa_header_.panel_count_per_frame_col/constants::region_count_per_frame);
  // Serial.print(",");
  // Serial.print(storage_ptr_->tpa_header_.panel_count_per_frame_row);
  // Serial.println(");");
  transferFrame(storage_ptr_->tpa_header_.panel_count_per_frame_col/constants::region_count_per_frame, storage_ptr_->tpa_header_.panel_count_per_frame_row);
  // Serial.print("endTransferFrame(");
  // Serial.print(storage_ptr_->tpa_header_.frame_count_x);
  // Serial.println(");");
  endTransferFrame(storage_ptr_->tpa_header_.frame_count_x);
}

void Display::setup(Storage & storage)
{
  storage_ptr_ = &storage;
  // setupSerial();
  setupPins();
  setupRegions();
  // setupEthernet();
  TransferTracker::setup();
  frame_index_ = 0;
}

// void Display::setupSerial()
// {
//   // Open serial communications and wait for port to open:
//   Serial.begin(constants::baud_rate);
//   // while (!Serial) {
//   //   ; // wait for serial port to connect. Needed for native USB port only
//   // }
// }

void Display::setupPins()
{
  pinMode(constants::reset_pin, OUTPUT);
  digitalWriteFast(constants::reset_pin, LOW);

  for (uint8_t col_index = 0; col_index<constants::panel_count_max_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<constants::panel_count_max_per_region_row; ++row_index)
    {
      const uint8_t & cs_pin = constants::panel_select_pins[row_index][col_index];
      pinMode(cs_pin, OUTPUT);
      digitalWriteFast(cs_pin, HIGH);
    }
  }
}

void Display::setupRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].setup(constants::region_spi_ptrs[region_index]);
  }
}

// void Display::setupEthernet()
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

void Display::beginTransferFrame()
{
  // if (frame_index_ == 0)
  // {
    storage_ptr_->rewindTpaFileForReading();
  // }
}

void Display::endTransferFrame(uint16_t frame_count)
{
  if (++frame_index_ == frame_count)
  {
    frame_index_ = 0;
  };
}

void Display::transferFrame(uint8_t panel_count_per_region_col, uint8_t panel_count_per_region_row)
{
  for (uint8_t col_index = 0; col_index<panel_count_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<panel_count_per_region_row; ++row_index)
    {
      beginTransferPanelsAcrossRegions();
      transferPanelsAcrossRegions(row_index, col_index);
      endTransferPanelsAcrossRegions();
    }
  }
}

void Display::beginTransferPanelsAcrossRegions()
{
  TransferTracker::beginTransferPanels();

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].beginTransferPanel(spi_settings_);
  }
}

void Display::endTransferPanelsAcrossRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].endTransferPanel();
  }

  TransferTracker::endTransferPanels();
}

void Display::transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index)
{
  const uint8_t & cs_pin = constants::panel_select_pins[row_index][col_index];
  digitalWriteFast(cs_pin, LOW);

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    storage_ptr_->readPanelFromFile(panel_buffer_[region_index], constants::byte_count_per_panel_grayscale);
    regions_[region_index].transferPanel(panel_buffer_[region_index], constants::byte_count_per_panel_grayscale);
  }

  while (not TransferTracker::allTransferPanelsComplete())
  {
    yield();
  }

  digitalWriteFast(cs_pin, HIGH);
}

// void Display::getMacAddress(uint8_t * mac_address)
// {
//   for(uint8_t by=0; by<2; by++) mac_address[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
//   for(uint8_t by=0; by<4; by++) mac_address[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
//   Serial.printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
// }
