// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.h"


using namespace panels_controller;

void Region::setup()
{
  for (uint8_t b = 0; b<constants::BYTE_COUNT_PER_PANEL_GRAYSCALE; ++b)
  {
    output_buffers[b] = 1;
  }
}

Arena::Arena() :
spi_settings_(SPISettings(constants::SPI_CLOCK, constants::SPI_BIT_ORDER, constants::SPI_DATA_MODE))
{}

void Arena::setup()
{
  setupPanelSelectPins();
  setupOutputBuffers();

  SPI.begin();
  SPI1.begin();
}

void Arena::update()
{
}

void Arena::transferFrameSynchronously()
{
  for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
  {
    SPIClass & spi = *(constants::REGION_SPI_PTRS[g]);
    spi.beginTransaction(spi_settings_);
    for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_REGION_COL; ++c)
    {
      for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_REGION_ROW; ++r)
      {
        const uint8_t & cs_pin = constants::PANEL_SELECT_PINS[r][c];
        digitalWriteFast(cs_pin, LOW);
        for (uint8_t b = 0; b<constants::BYTE_COUNT_PER_PANEL_GRAYSCALE; ++b)
        {
          spi.transfer(1);
        }
        digitalWriteFast(cs_pin, HIGH);
      }
    }
    spi.endTransaction();
  }
}

void Arena::transferFrameAsynchronously()
{
  for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_REGION_COL; ++c)
  {
    for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_REGION_ROW; ++r)
    {
      for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
      {
        SPIClass & spi = *(constants::REGION_SPI_PTRS[g]);
        spi.beginTransaction(spi_settings_);
      }

      const uint8_t & cs_pin = constants::PANEL_SELECT_PINS[r][c];
      digitalWriteFast(cs_pin, LOW);

      // bool all_spi_finished = false;
      for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
      {
        // spi_finished[g] = false;
        SPIClass & spi = *(constants::REGION_SPI_PTRS[g]);
        spi.transfer(output_buffers_[g], NULL, constants::BYTE_COUNT_PER_PANEL_GRAYSCALE, event_);
      }

      delayMicroseconds(300);
      // while (not all_spi_finished)
      // {
      //   all_spi_finished = true;
      //   for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
      //   {
      //     all_spi_finished = (all_spi_not_finished and spi_finished[g]);
      //   }
      // }

      digitalWriteFast(cs_pin, HIGH);

      for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
      {
        SPIClass & spi = *(constants::REGION_SPI_PTRS[g]);
        spi.endTransaction();
      }
    }
  }
}

void Arena::setupPanelSelectPins()
{
  for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_REGION_COL; ++c)
  {
    for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_REGION_ROW; ++r)
    {
      const uint8_t & cs_pin = constants::PANEL_SELECT_PINS[r][c];
      pinMode(cs_pin, OUTPUT);
      digitalWriteFast(cs_pin, HIGH);
    }
  }
}

void Arena::setupRegions()
{
  for (uint8_t g = 0; g<constants::REGION_COUNT_PER_ARENA; ++g)
  {
    regions_[g].setup();
  }
}

void PanelsController::setup()
{
  Serial.begin(115200);

  arena_.setup();
}

void PanelsController::update()
{
  arena_.update();
}

void PanelsController::transferFrameSynchronously()
{
  arena_.transferFrameSynchronously();
}

void PanelsController::transferFrameAsynchronously()
{
  arena_.transferFrameAsynchronously();
}
