#include <PanelsController.h>


PanelsController dev;

void setup()
{
  dev.setup();
}

void loop()
{
  dev.transferFrameSynchronously();
  delay(1000);
}
