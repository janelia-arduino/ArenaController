#include <PanelsController.h>


PanelsController dev;

void setup()
{
  dev.setup();
}

void loop()
{
  dev.transferFrameSynchronously();
  delay(1);
  dev.transferFrameAsynchronously();
  delay(1000);
}
