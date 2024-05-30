#include <ArenaController.hpp>


ArenaController dev;
uint32_t duration;

constexpr uint8_t FRAME_COUNT = 100;
constexpr uint8_t PANEL_COLUMNS_PER_FRAME = 12;
constexpr uint8_t PANEL_ROWS_PER_FRAME = 5;
//constexpr uint8_t PANEL_ROWS_PER_FRAME = 3;
//constexpr uint8_t PANEL_ROWS_PER_FRAME = 2;

char filename[100] = "";

void setup()
{
  dev.setup();


  sprintf(filename, "f%d_c%d_r%d",
    FRAME_COUNT,
    PANEL_COLUMNS_PER_FRAME,
    PANEL_ROWS_PER_FRAME);

  Serial.println("waiting to write...");
  delay(10);
  Serial.println("writing...");

  duration = millis();
  dev.storage.writeDummyFramesToFile(filename, FRAME_COUNT, PANEL_COLUMNS_PER_FRAME, PANEL_ROWS_PER_FRAME);
  duration = millis() - duration;
}

void loop()
{
  Serial.print(FRAME_COUNT);
  Serial.print(" frames written in ");
  Serial.print(duration);
  Serial.print(" milliseconds to filename ");
  Serial.println(filename);
  delay(10000);
}
