#include <ArenaController.hpp>


ArenaController dev;

constexpr uint32_t CLOCK_SPEED = 5000000;
//constexpr uint32_t CLOCK_SPEED = 10000000;
//constexpr uint32_t CLOCK_SPEED = 20000000;

//char filename[20] = "pat0003.tpa";
//char filename[20] = "pat0004.tpa";
//char filename[20] = "pat0007.tpa";
//char filename[20] = "pat0008.tpa";
char filename[20] = "pat0015.tpa";
bool success;

void setup()
{
  dev.setup();

  dev.display.setClockSpeed(CLOCK_SPEED);

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
