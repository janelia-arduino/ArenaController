#include <ArenaController.hpp>


ArenaController dev;

constexpr uint8_t FRAME_COUNT = 100;
constexpr uint8_t PANEL_COLUMNS_PER_FRAME = 12;
//constexpr uint8_t PANEL_ROWS_PER_FRAME = 5;
//constexpr uint8_t PANEL_ROWS_PER_FRAME = 3;
constexpr uint8_t PANEL_ROWS_PER_FRAME = 2;
constexpr uint32_t CLOCK_SPEED = 5000000;
//constexpr uint32_t CLOCK_SPEED = 10000000;
//constexpr uint32_t CLOCK_SPEED = 20000000;

char filename[100] = "";
bool success;

void setup()
{
  dev.setup();

  dev.display.setClockSpeed(CLOCK_SPEED);

  sprintf(filename, "f%d_c%d_r%d",
    FRAME_COUNT,
    PANEL_COLUMNS_PER_FRAME,
    PANEL_ROWS_PER_FRAME);
  success = dev.storage.openFileForReading(filename);
}

void loop()
{
  if (not success)
  {
    Serial.println("File not found");
    delay(2000);
  }
  dev.display.showFrame(FRAME_COUNT, PANEL_COLUMNS_PER_FRAME, PANEL_ROWS_PER_FRAME);
}
