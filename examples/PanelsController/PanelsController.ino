#include <PanelsController.h>


PanelsController dev;

void setup()
{
  dev.setup();
}

void loop()
{
  dev.update();
  delayMicroseconds(1000);
}
