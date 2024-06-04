// Pattern.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Pattern.hpp"


using namespace arena_controller;
using namespace pattern;

bool Pattern::importFromPat(ExFile & file)
{
  char import_filename[constants::filename_length_max] = "";
  file.getName(import_filename, constants::filename_length_max);
  const char * import_filename_suffix = getFilenameSuffix(import_filename);
  if (strcmp(import_filename_suffix, "pat") == 0)
  {
    memset(filename_stem_, 0, constants::filename_length_max);
    getFilenameStem(filename_stem_, import_filename);
    Serial.print("pat file: ");
    Serial.println(filename_stem_);

    file.rewind();
    PatHeader pat_header;
    file.read(&pat_header, pat_header_size);
    frame_count_x_ = pat_header.frame_count_x;
    frame_count_y_ = pat_header.frame_count_y;
    grayscale_value_ = pat_header.grayscale_value;
    panel_count_row_ = pat_header.panel_count_row;
    panel_count_col_ = pat_header.panel_count_col;

    if ((frame_count_x_ > constants::frame_count_max_x) || (frame_count_y_ > constants::frame_count_max_y))
    {
      file.close();
      return false;
    }
    if ((panel_count_row_ > constants::panel_count_max_per_frame_row) || (panel_count_col_ > constants::panel_count_max_per_frame_col))
    {
      file.close();
      return false;
    }
    if (grayscale_value_ < 16)
    {
      file.close();
      return false;
    }

    Serial.print(" frame_count_x: ");
    Serial.println(frame_count_x_);
    Serial.print(" frame_count_y: ");
    Serial.println(frame_count_y_);
    Serial.print(" grayscale_value: ");
    Serial.println(grayscale_value_);
    Serial.print(" panel_count_row: ");
    Serial.println(panel_count_row_);
    Serial.print(" panel_count_col: ");
    Serial.println(panel_count_col_);

    for (uint16_t frame_index_y = 0; frame_index_y<frame_count_y_; ++frame_index_y)
    {
      for (uint16_t frame_index_x = 0; frame_index_x<frame_count_x_; ++frame_index_x)
      {
        Serial.print(" frame: ");
        Serial.println(frame_index_x);
        uint8_t rows_read = 0;
        for (uint8_t panel_row_index = 0; panel_row_index<panel_count_row_; ++panel_row_index)
        {
          uint16_t bytes_read = 0;
          for (uint8_t quarter_panel_col_index = 0; quarter_panel_col_index<constants::quarter_panel_count_per_panel_col; ++quarter_panel_col_index)
          {
            for (uint8_t quarter_panel_row_index = 0; quarter_panel_row_index<constants::quarter_panel_count_per_panel_row; ++quarter_panel_row_index)
            {
              uint8_t row_signifier;
              file.read(&row_signifier, sizeof row_signifier);

              uint8_t panel_row_index = row_signifier - 1;
              uint8_t stretch;
              for (uint8_t panel_col_index = 0; panel_col_index<panel_count_col_; ++panel_col_index)
              {
                QuarterPanel & quarter_panel = frames_[frame_index_y][frame_index_x].panels[panel_row_index][panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
                file.read(&stretch, sizeof stretch);
                quarter_panel.stretch = stretch;
              }
              for (uint8_t pixel_row_index = 0; pixel_row_index<constants::pixel_count_per_quarter_panel_row; ++pixel_row_index)
              {
                for (uint8_t panel_col_index = 0; panel_col_index<panel_count_col_; ++panel_col_index)
                {
                  QuarterPanel & quarter_panel = frames_[frame_index_y][frame_index_x].panels[panel_row_index][panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
                  for (uint8_t byte_index = 0; byte_index<constants::byte_count_per_quarter_panel_row_grayscale; ++byte_index)
                  {
                    uint8_t data_byte;
                    file.read(&data_byte, sizeof data_byte);
                    ++bytes_read;
                    quarter_panel.data[pixel_row_index][byte_index] = data_byte;
                  }
                }
              }
            }
          }
          Serial.print(" bytes_read: ");
          Serial.println(bytes_read);
          ++rows_read;
        }
        Serial.print(" rows_read: ");
        Serial.println(rows_read);
      }
    }


    // file.printFileSize(&Serial);
    // char stem[10] = "";
    // getFilenameStem(stem, in_name);
    // char out_name[100] = "patterns/tpa/";
    // strcat(out_name, stem);
    // strcat(out_name, ".tpa");
    // sd_.open(out_name, O_WRONLY | O_CREAT);

    file.close();
    Serial.println("");
    return true;
  }
  return false;
}

const char * Pattern::getFilenameSuffix(const char * filename)
{
  const char * dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

void Pattern::getFilenameStem(char * stem, const char * filename)
{
  const char * dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  strncpy(stem, filename, (strlen(filename) - strlen(dot)));
}
