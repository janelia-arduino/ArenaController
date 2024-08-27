#include <ArenaController.hpp>


ArenaController dev;

constexpr uint32_t SPI_CLOCK_SPEED = 5000000;
//constexpr uint32_t SPI_CLOCK_SPEED = 5000000;
//constexpr uint32_t SPI_CLOCK_SPEED = 10000000;
//constexpr uint32_t SPI_CLOCK_SPEED = 20000000;

//char filename[20] = "pat0016.tpa";
//char filename[20] = "pat0017.tpa";
//char filename[20] = "pat0018.tpa";
//char filename[20] = "pat0019.tpa";
char filename[20] = "pat0020.tpa";
bool success;

void setup()
{
  dev.setup();

  dev.display.setSpiClockSpeed(SPI_CLOCK_SPEED);

  delay(6000);
  success = dev.storage.openTpaFileForReading(filename);
}

void loop()
{
  if (not success)
  {
    Serial.println("File not found");
    dev.storage.listFiles();
    delay(2000);
  }
  dev.display.showFrame();
}
