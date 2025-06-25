#ifndef ARENA_CONTROLLER_CONSTANTS_HPP
#define ARENA_CONTROLLER_CONSTANTS_HPP


namespace AC
{
namespace constants
{
constexpr uint32_t ticks_per_second = 1000;
constexpr uint8_t pool_event_count = 10;

// duration before callback fires
constexpr uint32_t watchdog_trigger_seconds = 1;
// duration before watchdog expires
constexpr uint32_t watchdog_timeout_seconds = 2;

// Serial Communication Interface
constexpr uint32_t serial_baud_rate = 115200;
constexpr uint16_t serial_timeout = 100;

// Ethernet Communication Interface
constexpr uint16_t ethernet_server_port = 62222;

// Display
constexpr uint32_t display_frequency_hz_default = 200;
constexpr uint32_t display_queue_size = 1;

// Commands
constexpr uint16_t string_command_length_max = 512;
constexpr uint8_t first_command_byte_max_value_binary = 0x32;
constexpr uint8_t byte_count_per_command_max = 16;
constexpr char command_termination_character = '\n';
constexpr uint8_t stream_header_byte_count = 7;

// Response
constexpr uint16_t string_response_length_max = 512;
constexpr uint16_t byte_count_per_response_max = 32;

// Log
constexpr uint16_t string_log_length_max = 512;

// Ethernet Settings
constexpr uint8_t mac_address_size = 6;

// Conversions
constexpr uint32_t microseconds_per_second = 1000000;

// message byte
constexpr uint8_t bit_count_per_byte = 8;

// quarter panel pixels
constexpr uint8_t pixel_count_per_quarter_panel_row = 8;
constexpr uint8_t pixel_count_per_quarter_panel_col = 8;
constexpr uint8_t pixel_count_per_quarter_panel = \
  pixel_count_per_quarter_panel_row * pixel_count_per_quarter_panel_col; // 64

// quarter panel grayscale
constexpr uint8_t bit_count_per_pixel_grayscale = 4;
constexpr uint8_t pixel_count_per_byte_grayscale = \
  bit_count_per_byte / bit_count_per_pixel_grayscale; // 2
constexpr uint8_t byte_count_per_quarter_panel_row_grayscale = \
  pixel_count_per_quarter_panel_row / pixel_count_per_byte_grayscale; // 4

// quarter panel binary
constexpr uint8_t bit_count_per_pixel_binary = 1;
constexpr uint8_t pixel_count_per_byte_binary = \
  bit_count_per_byte / bit_count_per_pixel_binary; // 8
constexpr uint8_t byte_count_per_quarter_panel_row_binary = \
  pixel_count_per_quarter_panel_row / pixel_count_per_byte_binary; // 1

// quarter panel message bytes
constexpr uint8_t byte_count_per_quarter_panel_control = 1;
constexpr uint8_t byte_count_per_quarter_panel_grayscale = \
  byte_count_per_quarter_panel_control + \
  pixel_count_per_quarter_panel / pixel_count_per_byte_grayscale; // 33
constexpr uint8_t byte_count_per_quarter_panel_binary = \
  byte_count_per_quarter_panel_control + \
  pixel_count_per_quarter_panel / pixel_count_per_byte_binary; // 9

// panel
constexpr uint8_t quarter_panel_count_per_panel_row = 2;
constexpr uint8_t quarter_panel_count_per_panel_col = 2;
constexpr uint8_t quarter_panel_count_per_panel = \
  quarter_panel_count_per_panel_row * quarter_panel_count_per_panel_col; // 4

// panel message bytes
constexpr uint8_t byte_count_per_panel_grayscale = \
  byte_count_per_quarter_panel_grayscale * \
  quarter_panel_count_per_panel; // 132
constexpr uint8_t byte_count_per_panel_binary = \
  byte_count_per_quarter_panel_binary * \
  quarter_panel_count_per_panel; // 36
constexpr uint8_t switch_grayscale_command_value_grayscale = 1;
constexpr uint8_t switch_grayscale_command_value_binary = 0;

// region
constexpr uint8_t region_count_per_frame_max = 2;

// pattern
constexpr char base_dir_str[] = "/";
constexpr uint8_t filename_str_len = 16;
constexpr uint8_t pattern_id_str_len = 4;
constexpr uint8_t pattern_header_size = 7;
constexpr uint8_t card_type_str_len = 16;

}
}
#endif
