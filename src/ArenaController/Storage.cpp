// Storage.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Storage.hpp"


using namespace arena_controller;

void Storage::listFiles()
{
  sd_.ls(LS_R);
}

void Storage::printFileInformation()
{
  ExFile file;
  pat_dir_.rewind();
  while (file.openNext(&pat_dir_, O_RDONLY))
  {
    file.printFileSize(&Serial);
    Serial.write(' ');
    file.printModifyDateTime(&Serial);
    Serial.write(' ');
    file.printName(&Serial);
    if (file.isDir())
    {
      // Indicate a directory.
      Serial.write('/');
    }
    Serial.println();
    file.close();
  }
  if (pat_dir_.getError())
  {
    Serial.println("openNext failed");
  }
  else
  {
    Serial.println("Done!");
  }
}

void Storage::printFileHeaders()
{
  ExFile file;
  pat_dir_.rewind();
  while (file.openNext(&pat_dir_, O_RDONLY))
  {
    file.printName(&Serial);
    Serial.println("");
    file.rewind();
    pattern::PatHeader pat_header;
    file.read(&pat_header, pattern::pat_header_size);
    Serial.print("frame_count_x: ");
    Serial.println(pat_header.frame_count_x);
    Serial.print("frame_count_y: ");
    Serial.println(pat_header.frame_count_y);
    Serial.print("grayscale_value: ");
    Serial.println(pat_header.grayscale_value);
    Serial.print("panel_count_row: ");
    Serial.println(pat_header.panel_count_row);
    Serial.print("panel_count_col: ");
    Serial.println(pat_header.panel_count_col);
    file.close();
    Serial.println("--");
  }
}

void Storage::printFileSizes()
{
  ExFile file;
  pat_dir_.rewind();
  while (file.openNext(&pat_dir_, O_RDONLY))
  {
    file.printName(&Serial);
    Serial.print(' ');
    file.printFileSize(&Serial);
    Serial.println("");
    file.rewind();
    pattern::PatHeader pat_header;
    file.read(&pat_header, pattern::pat_header_size);

    long file_size = 0;
    file_size += pattern::pat_header_size;
    file_size += (1 + pat_header.panel_count_col + 32*pat_header.panel_count_col)*4*pat_header.panel_count_row*pat_header.frame_count_x*pat_header.frame_count_y;

    Serial.print("calculated file size: ");
    Serial.println(file_size);
    file.close();
    Serial.println("--");
  }
}

void Storage::convertFiles()
{
  ExFile file;

  tpa_dir_.rewind();
  while (file.openNext(&tpa_dir_, O_WRONLY))
  {
    file.remove();
  }

  pat_dir_.rewind();
  while (file.openNext(&pat_dir_, O_RDONLY))
  {
    pattern::Pattern pattern;
    pattern.importFromPat(file);
  }

  tpa_dir_.rewind();
  while (file.openNext(&tpa_dir_, O_RDONLY))
  {
    char in_name[constants::filename_length_max] = "";
    file.getName(in_name, constants::filename_length_max);
    Serial.print(in_name);
    Serial.print(' ');
    file.printFileSize(&Serial);
    file.close();
    Serial.println("");
  }
}

void Storage::writeDummyFramesToFile(const char * filename, uint16_t frame_count, uint8_t panel_columns_per_frame, uint8_t panel_rows_per_frame)
{
  if (not openFileForWriting(filename))
  {
    return;
  }
  uint16_t half_frame_count = frame_count / 2;
  uint8_t panel_count_per_region_col = panel_columns_per_frame / constants::region_count_per_frame;
  uint8_t panel_count_per_region_row = panel_rows_per_frame;

  for (uint8_t frame_index = 0; frame_index<frame_count; ++frame_index)
  {
    for (uint8_t col_index = 0; col_index<panel_count_per_region_col; ++col_index)
    {
      for (uint8_t row_index = 0; row_index<panel_count_per_region_row; ++row_index)
      {
        for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
        {
          if (frame_index < half_frame_count)
          {
            writePanelToFile(pattern::all_on_grayscale_panel, constants::byte_count_per_panel_grayscale);
          }
          else
          {
            writePanelToFile(pattern::all_off_grayscale_panel, constants::byte_count_per_panel_grayscale);
          }
        }
      }
    }
  }
  closeFile();
}

bool Storage::openFileForWriting(const char * filename)
{
  return file_.open(filename, O_WRITE | O_CREAT | O_EXCL);
}

bool Storage::openFileForReading(const char * filename)
{
  return file_.open(filename);
}

void Storage::rewindFileForReading()
{
  file_.rewind();
  file_position_ = 0;
}

void Storage::closeFile()
{
  file_.close();
}

void Storage::writePanelToFile(const uint8_t * panel_buffer, size_t panel_byte_count)
{
  file_.write(panel_buffer, panel_byte_count);
  file_.sync();
}

void Storage::readPanelFromFile(uint8_t * panel_buffer, size_t panel_byte_count)
{
  file_.read(panel_buffer, panel_byte_count);
  file_position_ = file_position_ + panel_byte_count;
  file_.seekSet(file_position_);
}

void Storage::setup()
{
  sd_.begin(SD_CONFIG);

  pat_dir_.open("patterns/pat");
  tpa_dir_.open("patterns/tpa", O_CREAT);

  sd_.chdir(constants::base_dir_str);
}
