#include <ArenaController.hpp>


ArenaController dev;

constexpr uint32_t SPI_CLOCK_SPEED = 5000000;

void setup()
{
  dev.setup();
  dev.display.setSpiClockSpeed(SPI_CLOCK_SPEED);
}

void loop()
{
  dev.display.showAllOnFrame();
}
