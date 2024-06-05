#include <ArenaController.hpp>


ArenaController dev;
uint32_t duration;

void setup()
{
  dev.setup();

  delay(6000);
  Serial.println("Converting files:");
  duration = millis();
  dev.storage.convertFiles();
  duration = millis() - duration;
}

void loop()
{
  Serial.print(duration);
  Serial.println(" milliseconds");
  // dev.storage.convertFiles();
  Serial.println("--------");
  delay(4000);
}
