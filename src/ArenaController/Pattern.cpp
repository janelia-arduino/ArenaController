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
  file.rewind();
  PatHeader pat_header;
  file.read(&pat_header, pat_header_size);
  frame_count_x_ = pat_header.frame_count_x;
  frame_count_y_ = pat_header.frame_count_y;
  grayscale_value_ = pat_header.grayscale_value;
  panel_count_per_frame_row_ = pat_header.panel_count_per_frame_row;
  panel_count_per_frame_col_ = pat_header.panel_count_per_frame_col;

  if (frame_count_x_ > constants::frame_count_max_x)
  {
    frame_count_x_ = constants::frame_count_max_x;
    // file.printName();
    // Serial.print("Changed frame count to: ");
    // Serial.println(frame_count_x_);
  }

  if (frame_count_y_ > constants::frame_count_max_y)
  {
    frame_count_y_ = constants::frame_count_max_y;
  }

  // if ((frame_count_x_ > constants::frame_count_max_x) || (frame_count_y_ > constants::frame_count_max_y))
  // {
  //   file.printName();
  //   Serial.print("Invalid frame count: ");
  //   Serial.println(frame_count_x_);
  //   return false;
  // }
  if ((panel_count_per_frame_row_ > constants::panel_count_max_per_frame_row) || (panel_count_per_frame_col_ > constants::panel_count_max_per_frame_col))
  {
    // file.printName();
    // Serial.println("Invalid panel count.");
    return false;
  }
  if (panel_count_per_frame_col_ != constants::panel_count_max_per_frame_col)
  {
    // file.printName();
    // Serial.println("Invalid panel count.");
    return false;
  }
  if (grayscale_value_ < 16)
  {
    // file.printName();
    // Serial.println("Invalid grayscale value.");
    return false;
  }

  // file.printName();
  // Serial.print(" frame_count_x: ");
  // Serial.println(frame_count_x_);
  // Serial.print(" frame_count_y: ");
  // Serial.println(frame_count_y_);
  // Serial.print(" grayscale_value: ");
  // Serial.println(grayscale_value_);
  // Serial.print(" panel_count_per_frame_row: ");
  // Serial.println(panel_count_per_frame_row_);
  // Serial.print(" panel_count_per_frame_col: ");
  // Serial.println(panel_count_per_frame_col_);

  for (uint16_t frame_index_y = 0; frame_index_y<frame_count_y_; ++frame_index_y)
  {
    for (uint16_t frame_index_x = 0; frame_index_x<frame_count_x_; ++frame_index_x)
    {
      // Serial.print(" frame: ");
      // Serial.println(frame_index_x);
      uint8_t row_signifier_check = 1;
      for (int8_t panel_row_index = (panel_count_per_frame_row_ - 1); panel_row_index>=0; --panel_row_index)
      {
        // uint16_t bytes_read = 0;
        for (uint8_t quarter_panel_col_index = 0; quarter_panel_col_index<constants::quarter_panel_count_per_panel_col; ++quarter_panel_col_index)
        {
          for (uint8_t quarter_panel_row_index = 0; quarter_panel_row_index<constants::quarter_panel_count_per_panel_row; ++quarter_panel_row_index)
          {
            uint8_t row_signifier;
            file.read(&row_signifier, sizeof row_signifier);

            // Serial.print("row_signifier_check ");
            // Serial.print(row_signifier_check);
            // Serial.print("row_signifier ");
            // Serial.println(row_signifier);
            // if (row_signifier != row_signifier_check)
            // {
              // file.printName();
              // Serial.println("row_signifier != row_signifier_check!");
            // }
            uint8_t stretch;
            for (uint8_t panel_col_index = 0; panel_col_index<panel_count_per_frame_col_; ++panel_col_index)
            {
              QuarterPanel & quarter_panel = frames_[frame_index_y][frame_index_x].panels[panel_row_index][panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
              file.read(&stretch, sizeof stretch);
              quarter_panel.stretch = stretch;
            }
            for (uint8_t pixel_row_index = 0; pixel_row_index<constants::pixel_count_per_quarter_panel_row; ++pixel_row_index)
            {
              for (uint8_t panel_col_index = 0; panel_col_index<panel_count_per_frame_col_; ++panel_col_index)
              {
                QuarterPanel & quarter_panel = frames_[frame_index_y][frame_index_x].panels[panel_row_index][panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
                for (uint8_t byte_index = 0; byte_index<constants::byte_count_per_quarter_panel_row_grayscale; ++byte_index)
                {
                  uint8_t data_byte;
                  file.read(&data_byte, sizeof data_byte);
                  // ++bytes_read;
                  quarter_panel.data[pixel_row_index][byte_index] = data_byte;
                }
              }
            }
          }
        }
        // Serial.print(" bytes_read: ");
        // Serial.println(bytes_read);
        // Serial.print(" rows_read: ");
        // Serial.println(rows_read);
        ++row_signifier_check;
        // ++rows_read;
      }
    }
  }
  // Serial.println("");
  return true;
}

bool Pattern::exportToTpa(ExFile & file)
{
  // char opened_filename[constants::filename_length_max] = "";
  // file.getName(opened_filename, constants::filename_length_max);
  // Serial.print("successfully opened: ");
  // Serial.println(opened_filename);

  file.rewind();
  TpaHeader tpa_header;
  tpa_header.frame_count_x = frame_count_x_;
  tpa_header.frame_count_y = frame_count_y_;
  tpa_header.grayscale_value = grayscale_value_;
  tpa_header.panel_count_per_frame_row = panel_count_per_frame_row_;
  tpa_header.panel_count_per_frame_col = panel_count_per_frame_col_;
  file.write(&tpa_header, tpa_header_size);

  uint8_t panel_count_per_region_col = panel_count_per_frame_col_ / constants::region_count_per_frame;
  uint8_t panel_count_per_region_row = panel_count_per_frame_row_;

  for (uint16_t frame_index_y = 0; frame_index_y<frame_count_y_; ++frame_index_y)
  {
    for (uint16_t frame_index_x = 0; frame_index_x<frame_count_x_; ++frame_index_x)
    {
      for (uint8_t panel_col_index = 0; panel_col_index<panel_count_per_region_col; ++panel_col_index)
      {
        for (uint8_t panel_row_index = 0; panel_row_index<panel_count_per_region_row; ++panel_row_index)
        {
          for (uint8_t quarter_panel_col_index = 0; quarter_panel_col_index<constants::quarter_panel_count_per_panel_col; ++quarter_panel_col_index)
          {
            for (uint8_t quarter_panel_row_index = 0; quarter_panel_row_index<constants::quarter_panel_count_per_panel_row; ++quarter_panel_row_index)
            {
              for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
              {
                uint8_t region_panel_col_index = panel_col_index + region_index * panel_count_per_region_col;
                QuarterPanel & quarter_panel = frames_[frame_index_y][frame_index_x].panels[panel_row_index][region_panel_col_index].quarter_panels[quarter_panel_row_index][quarter_panel_col_index];
                file.write(quarter_panel.stretch);
                for (uint8_t pixel_row_index = 0; pixel_row_index<constants::pixel_count_per_quarter_panel_row; ++pixel_row_index)
                {
                  for (uint8_t byte_index = 0; byte_index<constants::byte_count_per_quarter_panel_row_grayscale; ++byte_index)
                  {
                    uint8_t data_byte = quarter_panel.data[pixel_row_index][byte_index];
                    file.write(data_byte);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // for (uint8_t frame_index = 0; frame_index<frame_count; ++frame_index)
  // {
  //   for (uint8_t col_index = 0; col_index<panel_count_per_region_col; ++col_index)
  //   {
  //     for (uint8_t row_index = 0; row_index<panel_count_per_region_row; ++row_index)
  //     {
  //       for (uint8_t region_index = 0; region_index<constants::region_count_per_frame; ++region_index)
  //       {
  //         if (frame_index < half_frame_count)
  //         {
  //           writePanelToFile(pattern::all_on_grayscale_panel, constants::byte_count_per_panel_grayscale);
  //         }
  //         else
  //         {
  //           writePanelToFile(pattern::all_off_grayscale_panel, constants::byte_count_per_panel_grayscale);
  //         }
  //       }
  //     }
  //   }
  // }
  return true;
}
