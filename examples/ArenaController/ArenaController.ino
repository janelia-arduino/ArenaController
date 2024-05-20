#include <ArenaController.hpp>


ArenaController dev;

void setup()
{
  dev.setup();
  dev.setupFileFromStorage();
}

void loop()
{
  dev.showFrameFromStorage();
}
