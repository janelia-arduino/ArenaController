#include <ArenaController.hpp>


ArenaController dev;

void setup()
{
  dev.setup();
}

void loop()
{
  dev.storage.printFileSizes();
  Serial.println("--------");
  delay(2000);
}
