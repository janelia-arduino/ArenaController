#ifndef PATTERN_HPP
#define PATTERN_HPP

#include <SdFat.h>
#include "constants.hpp"


// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD storage configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS

namespace AC
{
class Pattern
{
public:
  void setup();
  // void listFiles();
  // void printFileInformation();
  // void printPatFileHeaders();
  // void printFileSizes();
  // void convertFiles();

  // void writeDummyFramesToFile(const char * filename, uint16_t frame_count, uint8_t panel_columns_per_frame, uint8_t panel_rows_per_frame);

  // bool openFileForWriting(const char * filename);
  // bool openTpaFileForReading(const char * filename);
  // void rewindTpaFileForReading();
  bool openPatternFileForReading(uint16_t pattern_id);
  void rewindPatternFileForReading();
  void closeFile();
  // void writePanelToFile(const uint8_t * panel_buffer, size_t panel_byte_count);
  // void readPanelFromFile(uint8_t * panel_buffer, size_t panel_byte_count);
  // pattern::TpaHeader tpa_header_;
private:
  SdExFat sd_;
  ExFile pat_dir_;
  // ExFile tpa_dir_;
  ExFile file_;
  uint64_t file_position_;

  // const char * getFilenameSuffix(const char * filename);
  // void getFilenameStem(char * stem, const char * filename);
};
}
#endif
