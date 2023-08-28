#include <PanelsController.h>


PanelsController dev;

void setup()
{
  dev.setup();
}

void loop()
{
  dev.update();
  delay(1000);
}
