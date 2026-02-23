#ifndef ARENA_CONTROLLER_CONSTANTS_HPP
#define ARENA_CONTROLLER_CONSTANTS_HPP

#include <Arduino.h>

namespace AC
{
namespace constants
{
constexpr uint32_t ticks_per_second = 1000;
constexpr uint8_t set_parameter_event_pool_count = 30;
constexpr uint8_t command_event_pool_count = 20;
constexpr uint8_t frame_event_pool_count = 4;
constexpr uint8_t watchdog_event_queue_count = 2;
constexpr uint8_t serial_command_interface_event_queue_count = 10;
constexpr uint8_t ethernet_command_interface_event_queue_count = 10;
constexpr uint8_t pattern_event_queue_count = 20;
constexpr uint8_t arena_event_queue_count = 20;
constexpr uint8_t display_event_queue_count = 20;
constexpr uint8_t frame_event_queue_count = 20;

// Duration before callback fires
constexpr uint32_t watchdog_trigger_seconds = 1;
// Duration before watchdog expires
constexpr uint32_t watchdog_timeout_seconds = 1;

// QS
constexpr uint32_t qs_serial_baud_rate = 115200;

// Arena
constexpr uint32_t initialize_analog_duration_ms = 500;

// Serial Communication Interface
constexpr uint32_t serial_baud_rate = 115200;
constexpr uint16_t serial_timeout = 100;
constexpr uint32_t serial_timer_frequency_low_speed_hz = 100;
constexpr uint32_t serial_timer_frequency_high_speed_hz = 500;

// Ethernet Communication Interface
constexpr uint8_t mac_address_size = 6;
constexpr uint8_t ethernet_ip_address_length_max = 32;
constexpr uint16_t ethernet_server_port = 62222;
constexpr uint32_t ethernet_timer_frequency_low_speed_hz = 100;
constexpr uint32_t ethernet_timer_frequency_high_speed_hz = 1000;

// Display
constexpr uint32_t refresh_rate_grayscale_default = 200;
constexpr uint32_t refresh_rate_binary_default = 500;
constexpr uint32_t display_refresh_queue_size = 1;

// Commands
constexpr uint16_t string_command_length_max = 512;
constexpr uint8_t first_command_byte_max_value_binary = 0x32;
constexpr uint8_t byte_count_per_command_max = 16;
constexpr char command_termination_character = '\n';
constexpr uint8_t stream_header_byte_count = 7;

// Response
constexpr uint16_t string_response_length_max = 512;
constexpr uint16_t byte_count_per_response_max = 32;
constexpr uint16_t byte_count_per_pattern_finished_response_max = 200;
constexpr uint16_t char_count_runtime_duration_str = 20;

// Log
constexpr uint16_t string_log_length_max = 512;

// Conversions
constexpr uint32_t milliseconds_per_second = 1000;
constexpr uint32_t microseconds_per_second = 1000000;

// Message byte
constexpr uint8_t bit_count_per_byte = 8;

// Quarter panel pixels
constexpr uint8_t pixel_count_per_quarter_panel_row = 8;
constexpr uint8_t pixel_count_per_quarter_panel_col = 8;
constexpr uint8_t pixel_count_per_quarter_panel
    = pixel_count_per_quarter_panel_row
      * pixel_count_per_quarter_panel_col; // 64

// Quarter panel grayscale
constexpr uint8_t bit_count_per_pixel_grayscale = 4;
constexpr uint8_t pixel_count_per_byte_grayscale
    = bit_count_per_byte / bit_count_per_pixel_grayscale; // 2
constexpr uint8_t byte_count_per_quarter_panel_row_grayscale
    = pixel_count_per_quarter_panel_row / pixel_count_per_byte_grayscale; // 4

// Quarter panel binary
constexpr uint8_t bit_count_per_pixel_binary = 1;
constexpr uint8_t pixel_count_per_byte_binary
    = bit_count_per_byte / bit_count_per_pixel_binary; // 8
constexpr uint8_t byte_count_per_quarter_panel_row_binary
    = pixel_count_per_quarter_panel_row / pixel_count_per_byte_binary; // 1

// Quarter panel message bytes
constexpr uint8_t byte_count_per_quarter_panel_control = 1;
constexpr uint8_t byte_count_per_quarter_panel_grayscale
    = byte_count_per_quarter_panel_control
      + pixel_count_per_quarter_panel / pixel_count_per_byte_grayscale; // 33
constexpr uint8_t byte_count_per_quarter_panel_binary
    = byte_count_per_quarter_panel_control
      + pixel_count_per_quarter_panel / pixel_count_per_byte_binary; // 9

// Panel
constexpr uint8_t quarter_panel_count_per_panel_row = 2;
constexpr uint8_t quarter_panel_count_per_panel_col = 2;
constexpr uint8_t quarter_panel_count_per_panel
    = quarter_panel_count_per_panel_row
      * quarter_panel_count_per_panel_col; // 4

// Panel message bytes
constexpr uint8_t byte_count_per_panel_grayscale
    = byte_count_per_quarter_panel_grayscale
      * quarter_panel_count_per_panel; // 132
constexpr uint8_t byte_count_per_panel_binary
    = byte_count_per_quarter_panel_binary
      * quarter_panel_count_per_panel; // 36

// Frame
constexpr uint32_t frame_event_queue_size = 1;
constexpr uint8_t panel_count_per_frame_row_max = 5;
constexpr uint8_t panel_count_per_frame_col_max = 12;
constexpr uint8_t panel_count_per_frame_max
    = panel_count_per_frame_row_max * panel_count_per_frame_col_max; // 60
constexpr uint16_t byte_count_per_frame_max
    = panel_count_per_frame_max
      * byte_count_per_panel_grayscale; // 60*132=7920

constexpr uint8_t set_grayscale_command_value_grayscale = 1;
constexpr uint8_t set_grayscale_command_value_binary = 0;

// Pattern
constexpr uint8_t pattern_grayscale_value = 16;
constexpr uint8_t pattern_binary_value = 2;
constexpr uint8_t pattern_row_signifier_byte_count_per_row
    = quarter_panel_count_per_panel;
constexpr char pattern_dir_str[] = "/patterns/";
constexpr uint8_t pattern_header_size = 7;
constexpr uint8_t card_type_str_len = 16;
constexpr uint32_t pattern_begin_pattern_queue_size = 1;
constexpr uint32_t pattern_frame_rate_queue_size = 1;
constexpr uint32_t milliseconds_per_runtime_duration_unit = 100;
constexpr uint16_t find_card_timeout_duration = 500;
constexpr uint16_t byte_count_per_pattern_frame_max
    = byte_count_per_frame_max
      + pattern_row_signifier_byte_count_per_row
            * panel_count_per_frame_row_max
      + stream_header_byte_count; // 7920 + 4*5 + 7= 7947

// Analog
constexpr uint16_t analog_output_zero = 0;
constexpr uint16_t analog_output_min = 100;
constexpr uint16_t analog_output_max = 4095;
constexpr uint32_t analog_input_frequency_hz = 50;
constexpr uint32_t analog_closed_loop_frequency_hz = 200;
// frame_rate = (gain * (voltage + offset)) / scale_factor
// 20fps = (10 * (1000mv + 0)) / 500
constexpr int32_t analog_closed_loop_offset = 0;
constexpr int32_t analog_closed_loop_scale_factor = 500;

// ---------------------------------------------------------------------------
// Performance instrumentation
//
// These pins are intended as *optional* digital debug outputs that you can
// connect to an oscilloscope / logic analyzer to measure timing.
//
// Defaults are chosen for Teensy 4.1 and match the user's available pins
// (48, 49, 50). If these collide with your wiring, change them here.
//
// Suggested mapping (see perf instrumentation in fsp.cpp):
//   - perf_pin_refresh_tick: toggles every refresh timer ISR tick
//   - perf_pin_frame_transfer: HIGH while a full frame transfer is in-flight
//   - perf_pin_fetch: HIGH while a "frame preparation" stage runs
constexpr uint8_t perf_pin_refresh_tick = 48;
constexpr uint8_t perf_pin_frame_transfer = 49;
constexpr uint8_t perf_pin_fetch = 50;
} // namespace constants
} // namespace AC
#endif
