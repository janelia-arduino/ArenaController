// ----------------------------------------------------------------------------
// PanelsController.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "PanelsController.h"


using namespace panels_controller;

PanelsController::PanelsController() :
spi_settings_(SPISettings(constants::SPI_CLOCK, constants::SPI_BIT_ORDER, constants::SPI_DATA_MODE))
{}

void PanelsController::setup()
{
  Serial.begin(115200);

  setupPanelClockSelectPins();
  setupOutputBuffers();

  SPI.begin();
  SPI1.begin();

  // panel_row_index_ = 0;
  // panel_col_index_ = 0;
}

void PanelsController::update()
{
  // SPI.beginTransaction(spi_settings_);
  // for (uint16_t i = 0; i<constants::BYTE_COUNT_MAX_PER_ARENA_GRAYSCALE; ++i)
  // {
  //   SPI.transfer(1);
  // }
  // SPI.endTransaction();

  // Serial.println(constants::BYTE_COUNT_MAX_PER_ARENA_GRAYSCALE);

  // long panel_spi_address = (long)panels_controller::constants::PANEL_SPI_PTRS[panel_row_index_][panel_col_index_];
  // uint8_t panel_clock_select_pin = panels_controller::constants::PANEL_CLOCK_SELECT_PINS[panel_row_index_][panel_col_index_];

  // Serial.print("panel spi address at row: ");
  // Serial.print(panel_row_index_);
  // Serial.print(", col: ");
  // Serial.print(panel_col_index_);
  // Serial.print(" = ");
  // Serial.println(panel_spi_address);

  // Serial.print("panel clock select pin at row: ");
  // Serial.print(panel_row_index_);
  // Serial.print(", col: ");
  // Serial.print(panel_col_index_);
  // Serial.print(" = ");
  // Serial.println(panel_clock_select_pin);

  // if (++panel_col_index_ == panels_controller::constants::ARENA_PANEL_COUNT_MAX_PER_COL)
  // {
  //   panel_col_index_ = 0;
  //   if (++panel_row_index_ == panels_controller::constants::ARENA_PANEL_COUNT_MAX_PER_ROW)
  //   {
  //     panel_row_index_ = 0;
  //   }
  // }
}

void PanelsController::transferFrameSynchronously()
{
  for (uint8_t s = 0; s<constants::SPI_COUNT_PER_ARENA; ++s)
  {
    SPIClass & spi = *(constants::SPI_PTRS[s]);
    spi.beginTransaction(spi_settings_);
    for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_ARENA_SPI_COL; ++c)
    {
      for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_ARENA_SPI_ROW; ++r)
      {
        const uint8_t & cs_pin = constants::PANEL_CLOCK_SELECT_PINS[s][r][c];
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

void PanelsController::transferFrameAsynchronously()
{
  for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_ARENA_SPI_COL; ++c)
  {
    for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_ARENA_SPI_ROW; ++r)
    {
      for (uint8_t s = 0; s<constants::SPI_COUNT_PER_ARENA; ++s)
      {
        SPIClass & spi = *(constants::SPI_PTRS[s]);
        spi.beginTransaction(spi_settings_);
      }

      const uint8_t & cs_pin = constants::PANEL_CLOCK_SELECT_PINS[0][r][c];
      digitalWriteFast(cs_pin, LOW);

      // bool all_spi_finished = false;
      for (uint8_t s = 0; s<constants::SPI_COUNT_PER_ARENA; ++s)
      {
        // spi_finished[s] = false;
        SPIClass & spi = *(constants::SPI_PTRS[s]);
        spi.transfer(output_buffers_[s], input_buffers_[s], constants::BYTE_COUNT_PER_PANEL_GRAYSCALE, event_responder_);
      }

      delayMicroseconds(300);
      // while (not all_spi_finished)
      // {
      //   all_spi_finished = true;
      //   for (uint8_t s = 0; s<constants::SPI_COUNT_PER_ARENA; ++s)
      //   {
      //     all_spi_finished = (all_spi_not_finished and spi_finished[s]);
      //   }
      // }

      digitalWriteFast(cs_pin, HIGH);

      for (uint8_t s = 0; s<constants::SPI_COUNT_PER_ARENA; ++s)
      {
        SPIClass & spi = *(constants::SPI_PTRS[s]);
        spi.endTransaction();
      }
    }
  }
}



void PanelsController::setupPanelClockSelectPins()
{
  for (uint8_t s = 0; s<constants::SPI_COUNT_PER_ARENA; ++s)
  {
    for (uint8_t c = 0; c<constants::PANEL_COUNT_MAX_PER_ARENA_SPI_COL; ++c)
    {
      for (uint8_t r = 0; r<constants::PANEL_COUNT_MAX_PER_ARENA_SPI_ROW; ++r)
      {
        const uint8_t & cs_pin = constants::PANEL_CLOCK_SELECT_PINS[s][r][c];
        pinMode(cs_pin, OUTPUT);
        digitalWriteFast(cs_pin, HIGH);
      }
    }
  }
}

void PanelsController::setupOutputBuffers()
{
  for (uint8_t s = 0; s<constants::SPI_COUNT_PER_ARENA; ++s)
  {
    for (uint8_t b = 0; b<constants::BYTE_COUNT_PER_PANEL_GRAYSCALE; ++b)
    {
      output_buffers_[s][b] = 1;
    }
  }
}

void PanelsController::transferPanelSynchronously()
{
  for (uint16_t i = 0; i<constants::BYTE_COUNT_MAX_PER_ARENA_GRAYSCALE; ++i)
  {
    SPI.transfer(1);
  }
}
