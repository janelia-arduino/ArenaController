// Pattern.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Pattern.hpp"


using namespace arena_controller;
using namespace pattern;

void Pattern::importFromPat(ExFile & file)
{
  char import_filename[constants::filename_length_max] = "";
  file.getName(import_filename, constants::filename_length_max);
  const char * import_filename_suffix = getFilenameSuffix(import_filename);
  if (strcmp(import_filename_suffix, "pat") == 0)
  {
    memset(filename_stem_, 0, constants::filename_length_max);
    getFilenameStem(filename_stem_, import_filename);
    Serial.print("pat file: ");
    Serial.print(filename_stem_);

    file.rewind();
    PatHeader pat_header;
    file.read(&pat_header, pat_header_size);
    frame_count_x_ = pat_header.frame_count_x;
    frame_count_y_ = pat_header.frame_count_y;
    grayscale_value_ = pat_header.grayscale_value;

    for (uint16_t frame_index_y = 0; frame_index_y<frame_count_y_; ++frame_index_y)
    {
      for (uint16_t frame_index_x = 0; frame_index_x<frame_count_x_; ++frame_index_x)
      {

      }
    }


    Serial.print(" frame_count_x: ");
    Serial.print(frame_count_x_);
    Serial.print(" frame_count_y: ");
    Serial.print(frame_count_y_);
    Serial.print(" grayscale_value: ");
    Serial.print(grayscale_value_);
    Serial.println("");

    // file.printFileSize(&Serial);
    // char stem[10] = "";
    // getFilenameStem(stem, in_name);
    // char out_name[100] = "patterns/tpa/";
    // strcat(out_name, stem);
    // strcat(out_name, ".tpa");
    // sd_.open(out_name, O_WRONLY | O_CREAT);

    file.close();
  }
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
