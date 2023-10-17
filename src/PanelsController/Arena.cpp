// ----------------------------------------------------------------------------
// Arena.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Arena.hpp"


using namespace panels_controller;

Arena::Arena() :
spi_settings_(SPISettings(constants::spi_clock, constants::spi_bit_order, constants::spi_data_mode))
{}

void Arena::setup()
{
  setupSerial();
  setupPins();
  setupRegions();
  setupCard();
  setupEthernet();
  TransferTracker::setup();
  frame_index_ = 0;
  display_from_card_ = true;
}

void Arena::writeFramesToCard()
{
  card_.openFileForWriting();

  for (uint8_t frame_index = 0; frame_index<constants::frame_count; ++frame_index)
  {
    for (uint8_t col_index = 0; col_index<constants::panel_count_max_per_region_col; ++col_index)
    {
      for (uint8_t row_index = 0; row_index<constants::panel_count_max_per_region_row; ++row_index)
      {
        for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
        {
          if (frame_index < constants::half_frame_count)
          {
            card_.writePanelToFile(patterns::all_on, constants::byte_count_per_panel_grayscale);
          }
          else
          {
            card_.writePanelToFile(patterns::all_off, constants::byte_count_per_panel_grayscale);
          }
        }
      }
    }
  }
  card_.closeFile();
}

void Arena::displayFrameFromCard()
{
  beginTransferFrame();
  transferFrame();
  endTransferFrame();
}

void Arena::displayFrameFromRAM()
{
  display_from_card_ = false;
  beginTransferFrame();
  transferFrame();
  endTransferFrame();
}

void Arena::setupSerial()
{
  // Open serial communications and wait for port to open:
  Serial.begin(constants::baud_rate);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void Arena::setupPins()
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

void Arena::setupRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].setup(constants::region_spi_ptrs[region_index]);
  }
}

void Arena::setupCard()
{
  card_.setup();
}

void Arena::setupEthernet()
{
  uint8_t mac_address[constants::mac_address_size];
  getMacAddress(mac_address);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac_address) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void Arena::beginTransferFrame()
{
  if (frame_index_ == 0)
  {
    card_.openFileForReading();
  }
}

void Arena::endTransferFrame()
{
  if (++frame_index_ == constants::frame_count)
  {
    frame_index_ = 0;
    card_.closeFile();
  };
}

void Arena::transferFrame()
{
  for (uint8_t col_index = 0; col_index<constants::panel_count_max_per_region_col; ++col_index)
  {
    for (uint8_t row_index = 0; row_index<constants::panel_count_max_per_region_row; ++row_index)
    {
      beginTransferPanelsAcrossRegions();
      transferPanelsAcrossRegions(row_index, col_index);
      endTransferPanelsAcrossRegions();
    }
  }
}

void Arena::beginTransferPanelsAcrossRegions()
{
  TransferTracker::beginTransferPanels();

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].beginTransferPanel(spi_settings_);
  }
}

void Arena::endTransferPanelsAcrossRegions()
{
  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    regions_[region_index].endTransferPanel();
  }

  TransferTracker::endTransferPanels();
}

void Arena::transferPanelsAcrossRegions(uint8_t row_index, uint8_t col_index)
{
  const uint8_t & cs_pin = constants::panel_select_pins[row_index][col_index];
  digitalWriteFast(cs_pin, LOW);

  for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  {
    if (display_from_card_)
    {
      card_.readPanelFromFile(panel_buffer_, constants::byte_count_per_panel_grayscale);
      regions_[region_index].transferPanel(panel_buffer_, constants::byte_count_per_panel_grayscale);
    }
    else
    {
      if (frame_index_ < constants::half_frame_count)
      {
        regions_[region_index].transferPanel(patterns::all_on, constants::byte_count_per_panel_grayscale);
      }
      else
      {
        regions_[region_index].transferPanel(patterns::all_off, constants::byte_count_per_panel_grayscale);
      }
    }
  }

  while (not TransferTracker::allTransferPanelsComplete())
  {
    yield();
  }

  digitalWriteFast(cs_pin, HIGH);
}

void Arena::getMacAddress(uint8_t * mac_address)
{
  for(uint8_t by=0; by<2; by++) mac_address[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
  for(uint8_t by=0; by<4; by++) mac_address[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
  Serial.printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
}
