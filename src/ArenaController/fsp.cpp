#include "fsp.hpp"

#include "commands.hpp"
#include "modes.hpp"

using namespace QP;

using namespace AC;

namespace AC
{
namespace constants
{
static QSpyId const fsp_id = { 0U }; // QSpy source ID

static QEvt const reset_evt = { RESET_SIG, 0U, 0U };

static QEvt const deactivate_display_evt = { DEACTIVATE_DISPLAY_SIG, 0U, 0U };
static QEvt const display_frame_evt = { DISPLAY_FRAME_SIG, 0U, 0U };
static QEvt const transfer_frame_evt = { TRANSFER_FRAME_SIG, 0U, 0U };
static QEvt const frame_transferred_evt = { FRAME_TRANSFERRED_SIG, 0U, 0U };

static QEvt const process_binary_command_evt
    = { PROCESS_BINARY_COMMAND_SIG, 0U, 0U };
static QEvt const process_string_command_evt
    = { PROCESS_STRING_COMMAND_SIG, 0U, 0U };
static QEvt const process_stream_command_evt
    = { PROCESS_STREAM_COMMAND_SIG, 0U, 0U };
static QEvt const command_processed_evt = { COMMAND_PROCESSED_SIG, 0U, 0U };
static QEvt const pattern_finished_playing_evt
    = { PATTERN_FINISHED_PLAYING_SIG, 0U, 0U };
static QEvt const play_pattern_error_evt = { PLAY_PATTERN_ERROR_SIG, 0U, 0U };

static QEvt const all_on_evt = { ALL_ON_SIG, 0U, 0U };
static QEvt const all_off_evt = { ALL_OFF_SIG, 0U, 0U };
static QEvt const stream_frame_evt = { STREAM_FRAME_SIG, 0U, 0U };

static QEvt const activate_serial_command_interface_evt
    = { ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U };
static QEvt const deactivate_serial_command_interface_evt
    = { DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, 0U, 0U };
static QEvt const serial_initialized_evt = { SERIAL_INITIALIZED_SIG, 0U, 0U };
static QEvt const serial_command_available_evt
    = { SERIAL_COMMAND_AVAILABLE_SIG, 0U, 0U };

static QEvt const activate_ethernet_command_interface_evt
    = { ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U };
static QEvt const deactivate_ethernet_command_interface_evt
    = { DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, 0U, 0U };
static QEvt const ethernet_initialized_evt
    = { ETHERNET_INITIALIZED_SIG, 0U, 0U };
static QEvt const ethernet_server_connected_evt
    = { ETHERNET_SERVER_CONNECTED_SIG, 0U, 0U };

static QEvt const refresh_timeout_evt = { REFRESH_TIMEOUT_SIG, 0U, 0U };

static QEvt const fill_frame_buffer_with_all_on_evt
    = { FILL_FRAME_BUFFER_WITH_ALL_ON_SIG, 0U, 0U };
static QEvt const fill_frame_buffer_with_decoded_frame_evt
    = { FILL_FRAME_BUFFER_WITH_DECODED_FRAME_SIG, 0U, 0U };

static QEvt const begin_playing_pattern_evt
    = { BEGIN_PLAYING_PATTERN_SIG, 0U, 0U };
static QEvt const end_playing_pattern_evt
    = { END_PLAYING_PATTERN_SIG, 0U, 0U };
static QEvt const begin_showing_pattern_frame_evt
    = { BEGIN_SHOWING_PATTERN_FRAME_SIG, 0U, 0U };
static QEvt const end_showing_pattern_frame_evt
    = { END_SHOWING_PATTERN_FRAME_SIG, 0U, 0U };
static QEvt const card_found_evt = { CARD_FOUND_SIG, 0U, 0U };
static QEvt const card_not_found_evt = { CARD_NOT_FOUND_SIG, 0U, 0U };
static QEvt const directory_open_success_evt
    = { DIRECTORY_OPEN_SUCCESS_SIG, 0U, 0U };
static QEvt const directory_open_failure_evt
    = { DIRECTORY_OPEN_FAILURE_SIG, 0U, 0U };
static QEvt const find_pattern_evt = { FIND_PATTERN_SIG, 0U, 0U };
static QEvt const file_valid_evt = { FILE_VALID_SIG, 0U, 0U };
static QEvt const file_not_valid_evt = { FILE_NOT_VALID_SIG, 0U, 0U };
static QEvt const pattern_not_valid_evt = { PATTERN_NOT_VALID_SIG, 0U, 0U };
static QEvt const frame_decoded_evt = { FRAME_DECODED_SIG, 0U, 0U };

static QEvt const initialize_analog_output_evt
    = { INITIALIZE_ANALOG_OUTPUT_SIG, 0U, 0U };
static QEvt const analog_output_initialized_evt
    = { ANALOG_OUTPUT_INITIALIZED_SIG, 0U, 0U };

static QEvt const initialize_analog_input_evt
    = { INITIALIZE_ANALOG_INPUT_SIG, 0U, 0U };
static QEvt const analog_input_initialized_evt
    = { ANALOG_INPUT_INITIALIZED_SIG, 0U, 0U };
} // namespace constants
} // namespace AC
//----------------------------------------------------------------------------
// Local functions

void
FSP::ArenaController_setup ()
{
  static QF_MPOOL_EL (UnsignedValueEvt)
      smlPoolSto[constants::set_parameter_event_pool_count];
  static QF_MPOOL_EL (CommandEvt)
      medPoolSto[constants::command_event_pool_count];
  static QF_MPOOL_EL (FrameEvt) lrgPoolSto[constants::frame_event_pool_count];

  QF::init (); // initialize the framework

  QS_INIT (nullptr);

  BSP::init (); // initialize the BSP

  // initialize the event pools...
  QP::QF::poolInit (smlPoolSto, sizeof (smlPoolSto), sizeof (smlPoolSto[0]));
  QP::QF::poolInit (medPoolSto, sizeof (medPoolSto), sizeof (medPoolSto[0]));
  QP::QF::poolInit (lrgPoolSto, sizeof (lrgPoolSto), sizeof (lrgPoolSto[0]));

  // object dictionaries for AOs...
  QS_OBJ_DICTIONARY (AO_Arena);
  QS_OBJ_DICTIONARY (AO_SerialCommandInterface);
  QS_OBJ_DICTIONARY (AO_EthernetCommandInterface);
  QS_OBJ_DICTIONARY (AO_Display);
  QS_OBJ_DICTIONARY (AO_Frame);
  // Watchdog disabled
  // QS_OBJ_DICTIONARY (AO_Watchdog);
  QS_OBJ_DICTIONARY (AO_Pattern);

  QS_OBJ_DICTIONARY (&constants::fsp_id);

  // signal dictionaries for globally published events...
  QS_SIG_DICTIONARY (DEACTIVATE_DISPLAY_SIG, nullptr);
  QS_SIG_DICTIONARY (FRAME_FILLED_SIG, nullptr);
  QS_SIG_DICTIONARY (FRAME_TRANSFERRED_SIG, nullptr);
  QS_SIG_DICTIONARY (PLAY_PATTERN_SIG, nullptr);
  QS_SIG_DICTIONARY (SHOW_PATTERN_FRAME_SIG, nullptr);
  QS_SIG_DICTIONARY (ANALOG_CLOSED_LOOP_SIG, nullptr);

  QS_SIG_DICTIONARY (SERIAL_COMMAND_AVAILABLE_SIG, nullptr);
  QS_SIG_DICTIONARY (ETHERNET_COMMAND_AVAILABLE_SIG, nullptr);
  QS_SIG_DICTIONARY (PROCESS_BINARY_COMMAND_SIG, nullptr);
  QS_SIG_DICTIONARY (PROCESS_STRING_COMMAND_SIG, nullptr);
  QS_SIG_DICTIONARY (PROCESS_STREAM_COMMAND_SIG, nullptr);
  QS_SIG_DICTIONARY (COMMAND_PROCESSED_SIG, nullptr);
  QS_SIG_DICTIONARY (PATTERN_FINISHED_PLAYING_SIG, nullptr);
  QS_SIG_DICTIONARY (PLAY_PATTERN_ERROR_SIG, nullptr);

  // user record dictionaries
  QS_USR_DICTIONARY (ETHERNET_LOG);
  QS_USR_DICTIONARY (USER_COMMENT);

  // setup the QS filters...
  // QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records ON
  // QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records ON
  // QS_GLB_FILTER(QP::QS_QEP_STATE_ENTRY);
  // QS_GLB_FILTER(QP::QS_QEP_STATE_EXIT);
  // QS_GLB_FILTER(QP::QS_QEP_TRAN);
  // QS_GLB_FILTER(QP::QS_QEP_INTERN_TRAN);
  // QS_GLB_FILTER(QP::QS_QF_NEW);QS_QF_EQUEUE_POST
  // QS_GLB_FILTER(QP::QS_QF_EQUEUE_POST);
  QS_GLB_FILTER (QP::QS_UA_RECORDS); // all user records ON
  // QS_GLB_FILTER (-QP::QS_U0_RECORDS); // ethernet records OFF
  // QS_GLB_FILTER(QP::QS_U1_RECORDS); // user records ON

  // init publish-subscribe
  static QSubscrList subscrSto[MAX_PUB_SIG];
  QF::psInit (subscrSto, Q_DIM (subscrSto));

  // statically allocate event queues for the AOs and start them...
  // Watchdog disabled
  // static QEvt const *watchdog_queueSto[constants::watchdog_event_queue_count];
  // AO_Watchdog->start (1U, // priority
  //                     watchdog_queueSto, Q_DIM (watchdog_queueSto), (void *)0,
  //                     0U); // no stack

  static QEvt const *serial_command_interface_queueSto
      [constants::serial_command_interface_event_queue_count];
  AO_SerialCommandInterface->start (2U, // priority
                                    serial_command_interface_queueSto,
                                    Q_DIM (serial_command_interface_queueSto),
                                    (void *)0, 0U); // no stack

  static QEvt const *ethernet_command_interface_queueSto
      [constants::serial_command_interface_event_queue_count];
  AO_EthernetCommandInterface->start (
      3U, // priority
      ethernet_command_interface_queueSto,
      Q_DIM (ethernet_command_interface_queueSto), (void *)0, 0U); // no stack

  static QEvt const *pattern_queueSto[constants::pattern_event_queue_count];
  AO_Pattern->start (4U, // priority
                     pattern_queueSto, Q_DIM (pattern_queueSto), (void *)0,
                     0U); // no stack

  static QEvt const *arena_queueSto[constants::arena_event_queue_count];
  AO_Arena->start (5U, // priority
                   arena_queueSto, Q_DIM (arena_queueSto), (void *)0,
                   0U); // no stack

  static QEvt const *display_queueSto[constants::display_event_queue_count];
  AO_Display->start (6U, // priority
                     display_queueSto, Q_DIM (display_queueSto), (void *)0,
                     0U); // no stack

  static QEvt const *frame_queueSto[constants::frame_event_queue_count];
  AO_Frame->start (7U, // priority
                   frame_queueSto, Q_DIM (frame_queueSto), (void *)0,
                   0U); // no stack

  // Watchdog disabled
  // QS_LOC_FILTER (-AO_Watchdog->m_prio);
  // QS_LOC_FILTER(-AO_Display->m_prio);
  // QS_LOC_FILTER(-AO_Frame->m_prio);
  // QS_LOC_FILTER(-AO_EthernetCommandInterface->m_prio);
  // QS_LOC_FILTER(-AO_SerialCommandInterface->m_prio);

  QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
  QS_STR ("sizeof(smlPoolSto[0])");
  QS_U16 (5, sizeof (smlPoolSto[0]));
  QS_STR ("sizeof(medPoolSto[0])");
  QS_U16 (5, sizeof (medPoolSto[0]));
  QS_STR ("sizeof(lrgPoolSto[0])");
  QS_U16 (5, sizeof (lrgPoolSto[0]));
  QS_END ()
}

void
FSP::Arena_initializeAndSubscribe (QActive *const ao, QEvt const *e)
{
  BSP::initializeArena ();

  ao->subscribe (PLAY_PATTERN_SIG);
  ao->subscribe (FRAME_FILLED_SIG);
  ao->subscribe (SHOW_PATTERN_FRAME_SIG);
  ao->subscribe (ANALOG_CLOSED_LOOP_SIG);

  QS_SIG_DICTIONARY (ALL_ON_SIG, ao);
  QS_SIG_DICTIONARY (ALL_OFF_SIG, ao);
  QS_SIG_DICTIONARY (STREAM_FRAME_SIG, ao);
  QS_SIG_DICTIONARY (INITIALIZE_ANALOG_TIMEOUT_SIG, ao);

  QS_SIG_DICTIONARY (INITIALIZE_ANALOG_OUTPUT_SIG, ao);
  QS_SIG_DICTIONARY (ANALOG_OUTPUT_INITIALIZED_SIG, ao);
  QS_SIG_DICTIONARY (SET_ANALOG_OUTPUT_SIG, ao);

  QS_SIG_DICTIONARY (INITIALIZE_ANALOG_INPUT_SIG, ao);
  QS_SIG_DICTIONARY (ANALOG_INPUT_INITIALIZED_SIG, ao);
  QS_SIG_DICTIONARY (GET_ANALOG_INPUT_TIMEOUT_SIG, ao);

  Arena *const arena = static_cast<Arena *const> (ao);
  arena->initialize_analog_time_evt_.armX (
      constants::ticks_per_second / constants::milliseconds_per_second
      * constants::initialize_analog_duration_ms);

  arena->analog_output_->init (ao->m_prio);
  arena->analog_input_->init (ao->m_prio);
}

void
FSP::Arena_initializeAnalog (QP::QActive *const ao, QP::QEvt const *e)
{
  QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
  QS_STR ("Arena_initializeAnalog");
  QS_END ()
  Arena *const arena = static_cast<Arena *const> (ao);
  arena->analog_output_->dispatch (&constants::initialize_analog_output_evt,
                                   ao->m_prio);
  arena->analog_input_->dispatch (&constants::initialize_analog_input_evt,
                                  ao->m_prio);
}

void
FSP::Arena_activateCommandInterfaces (QActive *const ao, QEvt const *e)
{
  // AO_SerialCommandInterface->POST (
  //     &constants::activate_serial_command_interface_evt,
  //     &constants::fsp_id);
  AO_EthernetCommandInterface->POST (
      &constants::activate_ethernet_command_interface_evt, ao);
}

void
FSP::Arena_deactivateCommandInterfaces (QActive *const ao, QEvt const *e)
{
  // AO_SerialCommandInterface->POST (
  //     &constants::deactivate_serial_command_interface_evt,
  //     &constants::fsp_id);
  AO_EthernetCommandInterface->POST (
      &constants::deactivate_ethernet_command_interface_evt, ao);
}

void
FSP::Arena_deactivateDisplay (QActive *const ao, QEvt const *e)
{
  QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
  QS_STR ("deactivating display");
  QS_END ()
  QF::PUBLISH (&constants::deactivate_display_evt, ao);
}

void
FSP::Arena_displayFrame (QActive *const ao, QEvt const *e)
{
  AO_Display->POST (&constants::display_frame_evt, ao);
}

void
FSP::Arena_fillFrameBufferWithAllOn (QActive *const ao, QEvt const *e)
{
  AO_Frame->POST (&constants::fill_frame_buffer_with_all_on_evt, ao);
}

void
FSP::Arena_fillFrameBufferWithDecodedFrame (QActive *const ao, QEvt const *e)
{
  AO_Frame->POST (&constants::fill_frame_buffer_with_decoded_frame_evt, ao);
}

void
FSP::Arena_allOffTransition (QP::QActive *const ao, QP::QEvt const *e)
{
  UnsignedValueEvt *set_analog_output_ev
      = Q_NEW (UnsignedValueEvt, SET_ANALOG_OUTPUT_SIG);
  set_analog_output_ev->value = constants::analog_output_zero;
  AO_Arena->POST (set_analog_output_ev, ao);
}

void
FSP::Arena_allOnTransition (QP::QActive *const ao, QP::QEvt const *e)
{
  Arena_deactivateDisplay (ao, e);
  UnsignedValueEvt *set_analog_output_ev
      = Q_NEW (UnsignedValueEvt, SET_ANALOG_OUTPUT_SIG);
  set_analog_output_ev->value = constants::analog_output_zero;
  AO_Arena->POST (set_analog_output_ev, ao);
}

void
FSP::Arena_streamFrameTransition (QP::QActive *const ao, QP::QEvt const *e)
{
  Arena_deactivateDisplay (ao, e);
}

void
FSP::Arena_playPatternTransition (QP::QActive *const ao, QP::QEvt const *e)
{
  Arena_deactivateDisplay (ao, e);
}

void
FSP::Arena_endPlayingPattern (QActive *const ao, QEvt const *e)
{
  AO_Pattern->POST (&constants::end_playing_pattern_evt, ao);
}

void
FSP::Arena_showPatternFrameTransition (QP::QActive *const ao,
                                       QP::QEvt const *e)
{
  Arena_deactivateDisplay (ao, e);
}

void
FSP::Arena_endShowPatternFrame (QActive *const ao, QEvt const *e)
{
  AO_Pattern->POST (&constants::end_showing_pattern_frame_evt, ao);
}

void
FSP::Arena_analogClosedLoopTransition (QP::QActive *const ao,
                                       QP::QEvt const *e)
{
  Arena_deactivateDisplay (ao, e);
}

void
FSP::Arena_beginAnalogClosedLoop (QP::QActive *const ao, QP::QEvt const *e)
{
  Arena *const arena = static_cast<Arena *const> (ao);
  arena->analog_input_time_evt_.armX (
      constants::ticks_per_second / constants::analog_input_frequency_hz,
      constants::ticks_per_second / constants::analog_input_frequency_hz);
}

void
FSP::Arena_endAnalogClosedLoop (QActive *const ao, QEvt const *e)
{
  Arena *const arena = static_cast<Arena *const> (ao);
  AO_Pattern->POST (&constants::end_playing_pattern_evt, ao);
  arena->analog_input_time_evt_.disarm ();
}

void
FSP::AnalogOutput_initialize (QHsm *const hsm, QEvt const *e)
{
  QS_SIG_DICTIONARY (INITIALIZE_ANALOG_OUTPUT_SIG, hsm);
  QS_SIG_DICTIONARY (ANALOG_OUTPUT_INITIALIZED_SIG, hsm);
  QS_SIG_DICTIONARY (SET_ANALOG_OUTPUT_SIG, hsm);
}

void
FSP::AnalogOutput_initializeOutput (QHsm *const hsm, QEvt const *e)
{
  QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
  QS_STR ("initializing analog output");
  QS_END ()
  bool analog_output_initialized = BSP::initializeAnalogOutput ();
  if (analog_output_initialized)
    {
      AO_Arena->POST (&constants::analog_output_initialized_evt, hsm);
      QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
      QS_STR ("analog output initialized");
      QS_END ()
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
      QS_STR ("analog output not initialized!");
      QS_END ()
    }
}

void
FSP::AnalogOutput_enterInitialized (QHsm *const hsm, QEvt const *e)
{
  UnsignedValueEvt *uvev = Q_NEW (UnsignedValueEvt, SET_ANALOG_OUTPUT_SIG);
  uvev->value = constants::analog_output_zero;
  AO_Arena->POST (uvev, hsm);
}

void
FSP::AnalogOutput_setOutput (QHsm *const hsm, QEvt const *e)
{
  UnsignedValueEvt const *uvev = static_cast<UnsignedValueEvt const *> (e);
  BSP::setAnalogOutput (uvev->value);
  // QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
  //   QS_STR("AnalogOutput_setOutput");
  //   QS_U16(5, uvev->value);
  // QS_END()
}

void
FSP::AnalogInput_initialize (QHsm *const hsm, QEvt const *e)
{
  QS_SIG_DICTIONARY (INITIALIZE_ANALOG_INPUT_SIG, hsm);
  QS_SIG_DICTIONARY (ANALOG_INPUT_INITIALIZED_SIG, hsm);
  QS_SIG_DICTIONARY (GET_ANALOG_INPUT_TIMEOUT_SIG, hsm);
}

void
FSP::AnalogInput_initializeInput (QHsm *const hsm, QEvt const *e)
{
  QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
  QS_STR ("initializing analog input");
  QS_END ()
  bool analog_input_initialized = BSP::initializeAnalogInput ();
  if (analog_input_initialized)
    {
      AO_Arena->POST (&constants::analog_input_initialized_evt, hsm);
      QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
      QS_STR ("analog input initialized");
      QS_END ()
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
      QS_STR ("analog input not initialized!");
      QS_END ()
    }
}

void
FSP::AnalogInput_enterInitialized (QHsm *const hsm, QEvt const *e)
{
  BSP::setAnalogInputGainAndStartReading ();
}

void
FSP::AnalogInput_getInput (QHsm *const hsm, QEvt const *e)
{
  if (!BSP::analogInputDataAvailable ())
    {
      return;
    }
  int16_t analog_input_millivolts = BSP::getAnalogInputMillivolts ();

  SignedValueEvt *svev = Q_NEW (SignedValueEvt, GOT_ANALOG_INPUT_SIG);
  svev->value = analog_input_millivolts;
  AO_Pattern->POST (svev, hsm);
}

void
FSP::Display_initializeAndSubscribe (QActive *const ao, QEvt const *e)
{
  Display *const display = static_cast<Display *const> (ao);
  ao->subscribe (DEACTIVATE_DISPLAY_SIG);
  ao->subscribe (FRAME_TRANSFERRED_SIG);
  display->refresh_rate_hz_ = constants::refresh_rate_grayscale_default;

  static QEvt const
      *display_refresh_queue_store[constants::display_refresh_queue_size];
  display->refresh_queue_.init (display_refresh_queue_store,
                                Q_DIM (display_refresh_queue_store));

  QS_SIG_DICTIONARY (DISPLAY_FRAME_SIG, ao);
  QS_SIG_DICTIONARY (REFRESH_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY (SET_REFRESH_RATE_SIG, ao);
}

void
FSP::Display_setRefreshRate (QActive *const ao, QEvt const *e)
{
  Display *const display = static_cast<Display *const> (ao);
  UnsignedValueEvt const *uvev = static_cast<UnsignedValueEvt const *> (e);
  display->refresh_rate_hz_ = uvev->value;
  QS_BEGIN_ID (USER_COMMENT, AO_Display->m_prio)
  QS_STR ("set refresh rate");
  QS_U16 (5, display->refresh_rate_hz_);
  QS_END ()
}

void
postRefreshTimeout ()
{
  AO_Display->POST (&constants::refresh_timeout_evt, &constants::fsp_id);
}

void
FSP::Display_armRefreshTimer (QActive *const ao, QEvt const *e)
{
  Display *const display = static_cast<Display *const> (ao);
  BSP::armRefreshTimer (display->refresh_rate_hz_, postRefreshTimeout);
  // QS_BEGIN_ID(USER_COMMENT, AO_Display->m_prio)
  //   QS_STR("armRefreshTimer");
  //   QS_U16(5, display->refresh_rate_hz_);
  // QS_END()
}

void
FSP::Display_disarmRefreshTimer (QActive *const ao, QEvt const *e)
{
  BSP::disarmRefreshTimer ();
  // QS_BEGIN_ID(USER_COMMENT, AO_Display->m_prio)
  //   QS_STR("disarmRefreshTimer");
  // QS_END()
}

void
FSP::Display_transferFrame (QActive *const ao, QEvt const *e)
{
  AO_Frame->POST (&constants::transfer_frame_evt, ao);
}

void
FSP::Display_defer (QP::QActive *const ao, QP::QEvt const *e)
{
  Display *const display = static_cast<Display *const> (ao);
  if (display->refresh_queue_.getNFree () > 0)
    {
      display->defer (&display->refresh_queue_, e);
    }
}

void
FSP::Display_recall (QP::QActive *const ao, QP::QEvt const *e)
{
  Display *const display = static_cast<Display *const> (ao);
  display->recall (&display->refresh_queue_);
}

void
FSP::SerialCommandInterface_initializeAndSubscribe (QActive *const ao,
                                                    QEvt const *e)
{
  ao->subscribe (SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe (ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe (PROCESS_BINARY_COMMAND_SIG);
  ao->subscribe (PROCESS_STRING_COMMAND_SIG);
  ao->subscribe (PROCESS_STREAM_COMMAND_SIG);
  ao->subscribe (COMMAND_PROCESSED_SIG);
  ao->subscribe (PLAY_PATTERN_SIG);
  ao->subscribe (ANALOG_CLOSED_LOOP_SIG);
  ao->subscribe (PATTERN_FINISHED_PLAYING_SIG);
  ao->subscribe (PLAY_PATTERN_ERROR_SIG);

  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  QS_OBJ_DICTIONARY (&(sci->serial_time_evt_));
  QS_SIG_DICTIONARY (SERIAL_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY (ACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY (DEACTIVATE_SERIAL_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY (SERIAL_INITIALIZED_SIG, ao);
}

void
FSP::SerialCommandInterface_armSerialTimerLowSpeed (QActive *const ao,
                                                    QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  sci->serial_time_evt_.disarm ();
  sci->serial_time_evt_.armX (
      constants::ticks_per_second
          / constants::serial_timer_frequency_low_speed_hz,
      constants::ticks_per_second
          / constants::serial_timer_frequency_low_speed_hz);
}

void
FSP::SerialCommandInterface_armSerialTimerHighSpeed (QActive *const ao,
                                                     QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  sci->serial_time_evt_.disarm ();
  sci->serial_time_evt_.armX (
      constants::ticks_per_second
          / constants::serial_timer_frequency_high_speed_hz,
      constants::ticks_per_second
          / constants::serial_timer_frequency_high_speed_hz);
}

void
FSP::SerialCommandInterface_disarmSerialTimer (QActive *const ao,
                                               QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  sci->serial_time_evt_.disarm ();
}

void
FSP::SerialCommandInterface_initializeSerial (QActive *const ao, QEvt const *e)
{
  bool serial_ready = BSP::initializeSerial ();
  if (serial_ready)
    {
      AO_SerialCommandInterface->POST (&constants::serial_initialized_evt, ao);
    }
}

void
FSP::SerialCommandInterface_pollSerial (QActive *const ao, QEvt const *e)
{
  bool bytes_available = BSP::pollSerial ();
  if (bytes_available)
    {
      QF::PUBLISH (&constants::serial_command_available_evt, ao);
    }
}

void
FSP::SerialCommandInterface_analyzeCommand (QActive *const ao, QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);

  QS_BEGIN_ID (USER_COMMENT, AO_SerialCommandInterface->m_prio)
  QS_STR ("------------------------------------------------");
  QS_END ()

  uint8_t first_command_byte = BSP::readSerialByte ();
  uint8_t command_buffer_position = 0;
  command_buffer_position += sizeof (first_command_byte);
  sci->binary_command_byte_count_ = 0;
  sci->binary_command_[sci->binary_command_byte_count_++] = first_command_byte;
  if (first_command_byte > constants::first_command_byte_max_value_binary)
    {
      QS_BEGIN_ID (USER_COMMENT, AO_SerialCommandInterface->m_prio)
      QS_STR ("string command");
      QS_END ()
      QF::PUBLISH (&constants::process_string_command_evt, ao);
    }
  else if (first_command_byte == STREAM_FRAME_CMD)
    {
      uint16_t binary_command_byte_count_claim;
      for (uint8_t position = 0;
           position < sizeof (binary_command_byte_count_claim); ++position)
        {
          sci->binary_command_[sci->binary_command_byte_count_++]
              = BSP::readSerialByte ();
        }
      memcpy (&binary_command_byte_count_claim,
              sci->binary_command_ + command_buffer_position,
              sizeof (binary_command_byte_count_claim));
      sci->binary_command_byte_count_claim_ = binary_command_byte_count_claim;
      QS_BEGIN_ID (USER_COMMENT, AO_SerialCommandInterface->m_prio)
      QS_STR ("stream command");
      QS_U32 (8, sci->binary_command_byte_count_claim_);
      QS_END ()
      QF::PUBLISH (&constants::process_stream_command_evt, ao);
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_SerialCommandInterface->m_prio)
      QS_STR ("binary command");
      QS_END ()
      QF::PUBLISH (&constants::process_binary_command_evt, ao);
    }
}

void
FSP::SerialCommandInterface_processBinaryCommand (QActive *const ao,
                                                  QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  uint8_t binary_command_byte_count = sci->binary_command_[0];
  for (uint8_t position = 0; position < binary_command_byte_count; ++position)
    {
      sci->binary_command_[sci->binary_command_byte_count_++]
          = BSP::readSerialByte ();
    }
  sci->binary_response_byte_count_ = FSP::processBinaryCommand (
      sci->binary_command_, sci->binary_command_byte_count_,
      sci->binary_response_);
  QF::PUBLISH (&constants::command_processed_evt, ao);
}

void
FSP::SerialCommandInterface_writeBinaryResponse (QActive *const ao,
                                                 QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  BSP::writeSerialBinaryResponse (sci->binary_response_,
                                  sci->binary_response_byte_count_);
}

void
FSP::SerialCommandInterface_updateStreamCommand (QActive *const ao,
                                                 QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  uint32_t byte_count = 0;
  while (BSP::pollSerial ())
    {
      sci->binary_command_[sci->binary_command_byte_count_++]
          = BSP::readSerialByte ();
      ++byte_count;
    }
  QS_BEGIN_ID (USER_COMMENT, AO_SerialCommandInterface->m_prio)
  QS_STR ("update stream command");
  QS_U32 (8, byte_count);
  QS_END ()
}

bool
FSP::SerialCommandInterface_ifStreamCommandComplete (QActive *const ao,
                                                     QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  return (sci->binary_command_byte_count_
          >= sci->binary_command_byte_count_claim_);
}

void
FSP::SerialCommandInterface_processStreamCommand (QActive *const ao,
                                                  QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  sci->binary_response_byte_count_ = 3;
  sci->binary_response_[0] = 2;
  sci->binary_response_[1] = 0;
  sci->binary_response_[2] = STREAM_FRAME_CMD;
  FSP::processStreamCommand (sci->binary_command_,
                             sci->binary_command_byte_count_);
  QF::PUBLISH (&constants::command_processed_evt, ao);
}

void
FSP::SerialCommandInterface_storePlayPatternParameters (QActive *const ao,
                                                        QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  PlayPatternEvt const *ppev = static_cast<PlayPatternEvt const *> (e);

  sci->runtime_duration_ms_
      = ppev->runtime_duration
        * constants::milliseconds_per_runtime_duration_unit;
}

void
FSP::SerialCommandInterface_storeAnalogClosedLoopParameters (QActive *const ao,
                                                             QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  AnalogClosedLoopEvt const *aclev
      = static_cast<AnalogClosedLoopEvt const *> (e);

  sci->runtime_duration_ms_
      = aclev->runtime_duration
        * constants::milliseconds_per_runtime_duration_unit;
}

void
FSP::SerialCommandInterface_writePatternFinishedResponse (QActive *const ao,
                                                          QEvt const *e)
{
  SerialCommandInterface *const sci
      = static_cast<SerialCommandInterface *const> (ao);
  uint8_t response[constants::byte_count_per_pattern_finished_response_max];
  uint8_t response_byte_count = 0;
  response[response_byte_count++] = 2;
  response[response_byte_count++] = 0;
  response[response_byte_count++] = TRIAL_PARAMS_CMD;
  appendMessage (response, response_byte_count, "Sequence completed in ");
  char runtime_duration_str[constants::char_count_runtime_duration_str];
  itoa (sci->runtime_duration_ms_, runtime_duration_str, 10);
  appendMessage (response, response_byte_count, runtime_duration_str);
  appendMessage (response, response_byte_count, " ms");

  BSP::writeSerialBinaryResponse (response, response_byte_count);
  QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
  QS_STR ("wrote pattern finished response over serial");
  QS_END ()
}

void
FSP::SerialCommandInterface_writePatternErrorResponse (QActive *const ao,
                                                       QEvt const *e)
{
  uint8_t response[constants::byte_count_per_pattern_finished_response_max];
  uint8_t response_byte_count = 0;
  response[response_byte_count++] = 2;
  response[response_byte_count++] = 0;
  response[response_byte_count++] = TRIAL_PARAMS_CMD;
  appendMessage (response, response_byte_count, "Play pattern error!");

  BSP::writeSerialBinaryResponse (response, response_byte_count);
  QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
  QS_STR ("wrote pattern error response over serial");
  QS_END ()
}

void
FSP::EthernetCommandInterface_initializeAndSubscribe (QActive *const ao,
                                                      QEvt const *e)
{
  ao->subscribe (SERIAL_COMMAND_AVAILABLE_SIG);
  ao->subscribe (ETHERNET_COMMAND_AVAILABLE_SIG);
  ao->subscribe (PROCESS_BINARY_COMMAND_SIG);
  ao->subscribe (PROCESS_STRING_COMMAND_SIG);
  ao->subscribe (PROCESS_STREAM_COMMAND_SIG);
  ao->subscribe (COMMAND_PROCESSED_SIG);
  ao->subscribe (PLAY_PATTERN_SIG);
  ao->subscribe (ANALOG_CLOSED_LOOP_SIG);
  ao->subscribe (PATTERN_FINISHED_PLAYING_SIG);
  ao->subscribe (PLAY_PATTERN_ERROR_SIG);

  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  QS_OBJ_DICTIONARY (&(eci->ethernet_time_evt_));
  QS_SIG_DICTIONARY (ETHERNET_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY (ACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY (DEACTIVATE_ETHERNET_COMMAND_INTERFACE_SIG, ao);
  QS_SIG_DICTIONARY (ETHERNET_INITIALIZED_SIG, ao);
  QS_SIG_DICTIONARY (ETHERNET_SERVER_CONNECTED_SIG, ao);
}

void
FSP::EthernetCommandInterface_armEthernetTimerLowSpeed (QActive *const ao,
                                                        QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  eci->ethernet_time_evt_.disarm ();
  eci->ethernet_time_evt_.armX (
      constants::ticks_per_second
          / constants::ethernet_timer_frequency_low_speed_hz,
      constants::ticks_per_second
          / constants::ethernet_timer_frequency_low_speed_hz);
}

void
FSP::EthernetCommandInterface_armEthernetTimerHighSpeed (QActive *const ao,
                                                         QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  eci->ethernet_time_evt_.disarm ();
  eci->ethernet_time_evt_.armX (
      constants::ticks_per_second
          / constants::ethernet_timer_frequency_high_speed_hz,
      constants::ticks_per_second
          / constants::ethernet_timer_frequency_high_speed_hz);
}

void
FSP::EthernetCommandInterface_disarmEthernetTimer (QActive *const ao,
                                                   QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  eci->ethernet_time_evt_.disarm ();
}

void
FSP::EthernetCommandInterface_initializeEthernet (QActive *const ao,
                                                  QEvt const *e)
{
  bool ethernet_initialized = BSP::initializeEthernet ();
  if (ethernet_initialized)
    {
      AO_EthernetCommandInterface->POST (&constants::ethernet_initialized_evt,
                                         ao);
    }
}

void
FSP::EthernetCommandInterface_pollEthernet (QActive *const ao, QEvt const *e)
{
  BSP::pollEthernet ();
}

void
FSP::EthernetCommandInterface_createServerConnection (QActive *const ao,
                                                      QEvt const *e)
{
  bool server_connected = BSP::createEthernetServerConnection ();
  if (server_connected)
    {
      AO_EthernetCommandInterface->POST (
          &constants::ethernet_server_connected_evt, ao);
    }
}

void
FSP::EthernetCommandInterface_analyzeCommand (QActive *const ao, QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  CommandEvt const *cev = static_cast<CommandEvt const *> (e);
  eci->connection_ = cev->connection;
  eci->binary_command_ = cev->binary_command;
  eci->binary_command_byte_count_ = cev->binary_command_byte_count;

  QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
  QS_STR ("------------------------------------------------");
  QS_END ()

  uint8_t command_buffer_position = 0;
  uint8_t first_command_byte;
  memcpy (&first_command_byte, eci->binary_command_ + command_buffer_position,
          sizeof (first_command_byte));
  command_buffer_position += sizeof (first_command_byte);
  if (first_command_byte > constants::first_command_byte_max_value_binary)
    {
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("string command");
      QS_END ()
      QF::PUBLISH (&constants::process_string_command_evt, ao);
    }
  else if (first_command_byte == STREAM_FRAME_CMD)
    {
      uint16_t binary_command_byte_count_claim;
      memcpy (&binary_command_byte_count_claim,
              eci->binary_command_ + command_buffer_position,
              sizeof (binary_command_byte_count_claim));
      eci->binary_command_byte_count_claim_ = binary_command_byte_count_claim;
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("stream command");
      QS_U32 (8, eci->binary_command_byte_count_claim_);
      QS_END ()
      QF::PUBLISH (&constants::process_stream_command_evt, ao);
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("binary command");
      QS_END ()
      QF::PUBLISH (&constants::process_binary_command_evt, ao);
    }
}

void
FSP::EthernetCommandInterface_processBinaryCommand (QActive *const ao,
                                                    QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  eci->binary_response_byte_count_ = FSP::processBinaryCommand (
      eci->binary_command_, eci->binary_command_byte_count_,
      eci->binary_response_);
  QF::PUBLISH (&constants::command_processed_evt, ao);
}

void
FSP::EthernetCommandInterface_writeBinaryResponse (QActive *const ao,
                                                   QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  BSP::writeEthernetBinaryResponse (eci->connection_, eci->binary_response_,
                                    eci->binary_response_byte_count_);
}

void
FSP::EthernetCommandInterface_updateStreamCommand (QActive *const ao,
                                                   QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  CommandEvt const *cev = static_cast<CommandEvt const *> (e);
  eci->binary_command_ = cev->binary_command;
  eci->binary_command_byte_count_ = cev->binary_command_byte_count;
}

bool
FSP::EthernetCommandInterface_ifStreamCommandComplete (QActive *const ao,
                                                       QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  return (eci->binary_command_byte_count_
          >= eci->binary_command_byte_count_claim_);
}

void
FSP::EthernetCommandInterface_processStreamCommand (QActive *const ao,
                                                    QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  eci->binary_response_byte_count_ = 3;
  eci->binary_response_[0] = 2;
  eci->binary_response_[1] = 0;
  eci->binary_response_[2] = STREAM_FRAME_CMD;
  FSP::processStreamCommand (eci->binary_command_,
                             eci->binary_command_byte_count_);
  QF::PUBLISH (&constants::command_processed_evt, ao);
}

void
FSP::EthernetCommandInterface_storePlayPatternParameters (QActive *const ao,
                                                          QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  PlayPatternEvt const *ppev = static_cast<PlayPatternEvt const *> (e);

  eci->runtime_duration_ms_
      = ppev->runtime_duration
        * constants::milliseconds_per_runtime_duration_unit;
}

void
FSP::EthernetCommandInterface_storeAnalogClosedLoopParameters (
    QActive *const ao, QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  AnalogClosedLoopEvt const *aclev
      = static_cast<AnalogClosedLoopEvt const *> (e);

  eci->runtime_duration_ms_
      = aclev->runtime_duration
        * constants::milliseconds_per_runtime_duration_unit;
}

void
FSP::EthernetCommandInterface_writePatternFinishedResponse (QActive *const ao,
                                                            QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  uint8_t response[constants::byte_count_per_pattern_finished_response_max];
  uint8_t response_byte_count = 0;
  response[response_byte_count++] = 2;
  response[response_byte_count++] = 0;
  response[response_byte_count++] = TRIAL_PARAMS_CMD;
  appendMessage (response, response_byte_count, "Sequence completed in ");
  char runtime_duration_str[constants::char_count_runtime_duration_str];
  itoa (eci->runtime_duration_ms_, runtime_duration_str, 10);
  appendMessage (response, response_byte_count, runtime_duration_str);
  appendMessage (response, response_byte_count, " ms");

  BSP::writeEthernetBinaryResponse (eci->connection_, response,
                                    response_byte_count);
  QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
  QS_STR ("wrote pattern finished response over ethernet");
  QS_END ()
}

void
FSP::EthernetCommandInterface_writePatternErrorResponse (QActive *const ao,
                                                         QEvt const *e)
{
  EthernetCommandInterface *const eci
      = static_cast<EthernetCommandInterface *const> (ao);
  uint8_t response[constants::byte_count_per_pattern_finished_response_max];
  uint8_t response_byte_count = 0;
  response[response_byte_count++] = 2;
  response[response_byte_count++] = 0;
  response[response_byte_count++] = TRIAL_PARAMS_CMD;
  appendMessage (response, response_byte_count, "Play pattern error!");

  BSP::writeEthernetBinaryResponse (eci->connection_, response,
                                    response_byte_count);
  QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
  QS_STR ("wrote pattern error response over ethernet");
  QS_END ()
}

void
FSP::Frame_initializeAndSubscribe (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  BSP::initializeFrame ();
  frame->frame_ = nullptr;
  frame->grayscale_ = true;

  static QEvt const
      *frame_event_queue_store[constants::frame_event_queue_size];
  frame->event_queue_.init (frame_event_queue_store,
                            Q_DIM (frame_event_queue_store));

  ao->subscribe (FRAME_FILLED_SIG);
  ao->subscribe (DEACTIVATE_DISPLAY_SIG);
  ao->subscribe (FRAME_TRANSFERRED_SIG);

  QS_SIG_DICTIONARY (TRANSFER_FRAME_SIG, ao);
  QS_SIG_DICTIONARY (PANEL_SET_TRANSFERRED_SIG, ao);
  QS_SIG_DICTIONARY (FILL_FRAME_BUFFER_WITH_ALL_ON_SIG, ao);
  QS_SIG_DICTIONARY (FILL_FRAME_BUFFER_WITH_DECODED_FRAME_SIG, ao);
  QS_SIG_DICTIONARY (SET_GRAYSCALE_SIG, ao);
}

void
FSP::Frame_fillFrameBufferWithAllOn (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  FrameEvt *fe = Q_NEW (FrameEvt, FRAME_FILLED_SIG);

  BSP::fillFrameBufferWithAllOn (fe->buffer, frame->grayscale_);
  if (frame->grayscale_)
    {
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("filled frame buffer with grayscale all on");
      QS_END ()
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("filled frame buffer with binary all on");
      QS_END ()
    }
  QF::PUBLISH (fe, ao);
}

void
FSP::Frame_fillFrameBufferWithDecodedFrame (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  FrameEvt *fe = Q_NEW (FrameEvt, FRAME_FILLED_SIG);

  BSP::fillFrameBufferWithDecodedFrame (fe->buffer, frame->grayscale_);
  QF::PUBLISH (fe, ao);
}

void
FSP::Frame_saveFrameReference (QP::QActive *const ao, QP::QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  Q_NEW_REF (frame->frame_, FrameEvt);
}

void
FSP::Frame_deleteFrameReference (QP::QActive *const ao, QP::QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  if (frame->frame_)
    {
      Q_DELETE_REF (frame->frame_);
      frame->frame_ = nullptr;
    }
}

void
FSP::Frame_reset (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  frame->panel_set_row_index_ = 0;
  frame->panel_set_col_index_ = 0;
  frame->buffer_position_ = 0;
}

void
FSP::Frame_beginTransferPanelSet (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  BSP::enablePanelSetSelectPin (frame->panel_set_row_index_,
                                frame->panel_set_col_index_);
  uint8_t panel_byte_count;
  if (frame->grayscale_)
    {
      panel_byte_count = constants::byte_count_per_panel_grayscale;
    }
  else
    {
      panel_byte_count = constants::byte_count_per_panel_binary;
    }
  BSP::transferPanelSet (frame->frame_->buffer, frame->buffer_position_,
                         panel_byte_count);
}

void
FSP::Frame_endTransferPanelSet (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  BSP::disablePanelSetSelectPin (frame->panel_set_row_index_,
                                 frame->panel_set_col_index_);
  ++frame->panel_set_row_index_;
  if (frame->panel_set_row_index_ == BSP::getPanelCountPerRegionRow ())
    {
      frame->panel_set_row_index_ = 0;
      ++frame->panel_set_col_index_;
    }
  if (frame->panel_set_col_index_ == BSP::getPanelCountPerRegionCol ())
    {
      frame->panel_set_col_index_ = 0;
    }
}

bool
FSP::Frame_ifFrameNotTransferred (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  return (frame->panel_set_row_index_
          != (BSP::getPanelCountPerRegionRow () - 1))
         || (frame->panel_set_col_index_
             != (BSP::getPanelCountPerRegionCol () - 1));
}

void
FSP::Frame_publishFrameTransferred (QActive *const ao, QEvt const *e)
{
  QF::PUBLISH (&constants::frame_transferred_evt, ao);
}

void
FSP::Frame_setGrayscale (QActive *const ao, QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  UnsignedValueEvt const *uvev = static_cast<UnsignedValueEvt const *> (e);

  uint8_t grayscale_index = uvev->value;

  if (grayscale_index == constants::set_grayscale_command_value_grayscale)
    {
      frame->grayscale_ = true;
      QS_BEGIN_ID (USER_COMMENT, AO_Frame->m_prio)
      QS_STR ("set grayscale to true");
      QS_END ()
    }
  else if (grayscale_index == constants::set_grayscale_command_value_binary)
    {
      frame->grayscale_ = false;
      QS_BEGIN_ID (USER_COMMENT, AO_Frame->m_prio)
      QS_STR ("set grayscale to false");
      QS_END ()
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Frame->m_prio)
      QS_STR ("invalid grayscale value");
      QS_END ()
    }
}

void
FSP::Frame_defer (QP::QActive *const ao, QP::QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  if (frame->event_queue_.getNFree () > 0)
    {
      frame->defer (&frame->event_queue_, e);
    }
}

void
FSP::Frame_recall (QP::QActive *const ao, QP::QEvt const *e)
{
  Frame *const frame = static_cast<Frame *const> (ao);
  frame->recall (&frame->event_queue_);
}

void
FSP::Watchdog_initializeAndSubscribe (QActive *const ao, QEvt const *e)
{
  BSP::initializeWatchdog ();

  Watchdog *const watchdog = static_cast<Watchdog *const> (ao);
  QS_OBJ_DICTIONARY (&(watchdog->watchdog_time_evt_));
  QS_SIG_DICTIONARY (WATCHDOG_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY (RESET_SIG, ao);
}

void
FSP::Watchdog_armWatchdogTimer (QActive *const ao, QEvt const *e)
{
  Watchdog *const watchdog = static_cast<Watchdog *const> (ao);
  watchdog->watchdog_time_evt_.armX (constants::ticks_per_second,
                                     constants::ticks_per_second);
}

void
FSP::Watchdog_disarmWatchdogTimer (QActive *const ao, QEvt const *e)
{
  Watchdog *const watchdog = static_cast<Watchdog *const> (ao);
  watchdog->watchdog_time_evt_.disarm ();
}

void
FSP::Watchdog_feedWatchdog (QActive *const ao, QEvt const *e)
{
  BSP::feedWatchdog ();
}

void
FSP::Pattern_initializeAndSubscribe (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->frame_rate_hz_ = 0;
  pattern->runtime_duration_ms_ = 0;
  pattern->byte_count_per_frame_ = 0;
  pattern->positive_direction_ = true;
  pattern->frame_ = nullptr;
  pattern->frame_count_per_pattern_ = 0;
  pattern->frame_index_ = 0;
  pattern->grayscale_ = true;
  pattern->gain_ = 10;
  pattern->timeout_count_ = 0;
  pattern->timeout_counts_per_frame_index_ = 1;

  static QEvt const *pattern_begin_pattern_queue_store
      [constants::pattern_begin_pattern_queue_size];
  pattern->begin_pattern_queue_.init (
      pattern_begin_pattern_queue_store,
      Q_DIM (pattern_begin_pattern_queue_store));
  static QEvt const *
      pattern_frame_rate_queue_store[constants::pattern_frame_rate_queue_size];
  pattern->frame_rate_queue_.init (pattern_frame_rate_queue_store,
                                   Q_DIM (pattern_frame_rate_queue_store));

  ao->subscribe (PLAY_PATTERN_SIG);
  ao->subscribe (SHOW_PATTERN_FRAME_SIG);
  ao->subscribe (ANALOG_CLOSED_LOOP_SIG);
  ao->subscribe (FRAME_FILLED_SIG);
  ao->subscribe (FRAME_TRANSFERRED_SIG);

  QS_SIG_DICTIONARY (FRAME_RATE_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY (RUNTIME_DURATION_TIMEOUT_SIG, ao);
  QS_SIG_DICTIONARY (FIND_CARD_SIG, ao);
  QS_SIG_DICTIONARY (FIND_PATTERN_SIG, ao);
  QS_SIG_DICTIONARY (BEGIN_PLAYING_PATTERN_SIG, ao);
  QS_SIG_DICTIONARY (END_PLAYING_PATTERN_SIG, ao);
  QS_SIG_DICTIONARY (FRAME_READ_FROM_FILE_SIG, ao);
  QS_SIG_DICTIONARY (FRAME_DECODED_SIG, ao);
  QS_SIG_DICTIONARY (SET_FRAME_COUNT_PER_PATTERN_SIG, ao);
  QS_SIG_DICTIONARY (SET_BYTE_COUNT_PER_FRAME_SIG, ao);
  QS_SIG_DICTIONARY (UPDATE_PATTERN_FRAME_SIG, ao);
  QS_SIG_DICTIONARY (GOT_ANALOG_INPUT_SIG, ao);

  pattern->card_->init (ao->m_prio);
}

void
FSP::Pattern_initializePlayPattern (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  PlayPatternEvt const *ppev = static_cast<PlayPatternEvt const *> (e);

  if (ppev->frame_rate == 0)
    {
      QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
      QS_STR ("invalid frame rate");
      QS_END ()
      AO_Arena->POST (&constants::all_off_evt, ao);
      return;
    }
  else if (ppev->frame_rate < 0)
    {
      pattern->positive_direction_ = false;
      QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
      QS_STR ("valid negative frame rate");
      QS_END ()
    }
  else
    {
      pattern->positive_direction_ = true;
      QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
      QS_STR ("valid positive frame rate");
      QS_END ()
    }
  pattern->frame_rate_hz_ = abs (ppev->frame_rate);
  pattern->runtime_duration_ms_
      = ppev->runtime_duration
        * constants::milliseconds_per_runtime_duration_unit;
  pattern->frame_index_ = ppev->frame_index;
  // QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
  //   QS_STR("check and store parameters");
  // QS_U8(0, control_mode);
  // QS_U16(5, pattern->frame_rate_hz_);
  // QS_U16(5, init_pos);
  // QS_U16(5, gain);
  // QS_U16(5, pattern->runtime_duration_ms_);
  // QS_END()
  pattern->card_->dispatch (e, ao->m_prio);
  AO_Pattern->POST (&constants::begin_playing_pattern_evt, ao);
}

void
FSP::Pattern_initializeShowPatternFrame (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  ShowPatternFrameEvt const *spfev
      = static_cast<ShowPatternFrameEvt const *> (e);
  pattern->frame_index_ = spfev->frame_index;
  pattern->card_->dispatch (e, ao->m_prio);
  AO_Pattern->POST (&constants::begin_showing_pattern_frame_evt, ao);
}

void
FSP::Pattern_initializeAnalogClosedLoop (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  AnalogClosedLoopEvt const *aclev
      = static_cast<AnalogClosedLoopEvt const *> (e);

  pattern->gain_ = aclev->gain;
  pattern->frame_rate_hz_ = constants::analog_closed_loop_frequency_hz;
  pattern->runtime_duration_ms_
      = aclev->runtime_duration
        * constants::milliseconds_per_runtime_duration_unit;
  pattern->frame_index_ = aclev->frame_index;
  // QS_BEGIN_ID(USER_COMMENT, AO_Arena->m_prio)
  //   QS_STR("check and store parameters");
  // QS_U8(0, control_mode);
  // QS_U16(5, pattern->frame_rate_hz_);
  // QS_U16(5, init_pos);
  // QS_U16(5, gain);
  // QS_U16(5, pattern->runtime_duration_ms_);
  // QS_END()
  pattern->card_->dispatch (e, ao->m_prio);
  AO_Pattern->POST (&constants::begin_playing_pattern_evt, ao);
}

void
FSP::Pattern_armFindCardTimer (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->find_card_time_evt_.armX (
      (constants::ticks_per_second * constants::find_card_timeout_duration)
      / constants::milliseconds_per_second);
  QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
  QS_STR ("finding card may cause reboot if card not found...");
  QS_END ()
}

void
FSP::Pattern_endRuntimeDuration (QActive *const ao, QEvt const *e)
{
  QF::PUBLISH (&constants::pattern_finished_playing_evt, ao);
  AO_Arena->POST (&constants::all_off_evt, ao);
}

void
FSP::Pattern_armTimers (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->frame_rate_time_evt_.armX (
      constants::ticks_per_second / pattern->frame_rate_hz_,
      constants::ticks_per_second / pattern->frame_rate_hz_);
  pattern->runtime_duration_time_evt_.armX (
      (constants::ticks_per_second * pattern->runtime_duration_ms_)
      / constants::milliseconds_per_second);
  QS_BEGIN_ID (USER_COMMENT, ao->m_prio)
  QS_STR ("arming pattern timers");
  QS_STR ("frame rate ticks");
  QS_U32 (8, (constants::ticks_per_second / pattern->frame_rate_hz_));
  QS_STR ("runtime duration ticks");
  QS_U32 (8, ((constants::ticks_per_second * pattern->runtime_duration_ms_)
              / constants::milliseconds_per_second));
  QS_END ()
}

void
FSP::Pattern_disarmTimersAndCleanup (QActive *const ao, QEvt const *e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("disarming pattern timers");
  // QS_END()
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->frame_rate_time_evt_.disarm ();
  pattern->runtime_duration_time_evt_.disarm ();

  UnsignedValueEvt *uvev = Q_NEW (UnsignedValueEvt, SET_ANALOG_OUTPUT_SIG);
  uvev->value = constants::analog_output_zero;
  AO_Arena->POST (uvev, ao);

  if (pattern->frame_)
    {
      Q_DELETE_REF (pattern->frame_);
      pattern->frame_ = nullptr;
    }
}

void
FSP::Pattern_deactivateDisplay (QActive *const ao, QEvt const *e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("deactivating display");
  // QS_END()
  QF::PUBLISH (&constants::deactivate_display_evt, ao);
}

void
FSP::Pattern_readFrameFromFile (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  FrameEvt *fe = Q_NEW (FrameEvt, FRAME_READ_FROM_FILE_SIG);
  BSP::readPatternFrameFromFileIntoBuffer (fe->buffer, pattern->frame_index_,
                                           pattern->byte_count_per_frame_);
  AO_Pattern->POST (fe, ao);
}

void
FSP::Pattern_saveFrameReference (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  Q_NEW_REF (pattern->frame_, FrameEvt);
}

void
FSP::Pattern_deleteFrameReference (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  if (pattern->frame_)
    {
      Q_DELETE_REF (pattern->frame_);
      pattern->frame_ = nullptr;
    }
}

void
FSP::Pattern_setupNextFrame (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  if (pattern->frame_)
    {
      Q_DELETE_REF (pattern->frame_);
      pattern->frame_ = nullptr;
    }
  pattern->timeout_count_ = pattern->timeout_count_ + 1;
  if (pattern->timeout_counts_per_frame_index_ == 0)
    {
      return;
    }
  if ((pattern->timeout_count_ % pattern->timeout_counts_per_frame_index_)
      != 0)
    {
      return;
    }
  uint16_t frame_index_inc = 1;
  if (pattern->positive_direction_)
    {
      pattern->frame_index_ = pattern->frame_index_ + frame_index_inc;
      if (pattern->frame_index_ >= pattern->frame_count_per_pattern_)
        {
          pattern->frame_index_
              = pattern->frame_index_ - pattern->frame_count_per_pattern_;
        }
    }
  else
    {
      if (pattern->frame_index_ >= frame_index_inc)
        {
          pattern->frame_index_ = pattern->frame_index_ - frame_index_inc;
        }
      else
        {
          pattern->frame_index_ = pattern->frame_count_per_pattern_
                                  - (frame_index_inc - pattern->frame_index_);
        }
    }
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("Pattern_setupNextFrame");
  //   QS_STR("frame_index");
  //   QS_U32(8, pattern->frame_index_);
  // QS_END()
}

void
FSP::Pattern_updatePatternFrame (QP::QActive *const ao, QP::QEvt const *e)
{
  QF::PUBLISH (&constants::deactivate_display_evt, ao);

  Pattern *const pattern = static_cast<Pattern *const> (ao);
  if (pattern->frame_)
    {
      Q_DELETE_REF (pattern->frame_);
      pattern->frame_ = nullptr;
    }

  UnsignedValueEvt const *uvev = static_cast<UnsignedValueEvt const *> (e);
  pattern->frame_index_ = uvev->value;
  if (pattern->frame_index_ >= pattern->frame_count_per_pattern_)
    {
      pattern->frame_index_ = 0;
    }
  QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
  QS_STR ("update pattern frame");
  QS_U16 (5, pattern->frame_index_);
  QS_END ()
}

void
FSP::Pattern_decodeFrame (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  BSP::decodePatternFrameBuffer (pattern->frame_->buffer, pattern->grayscale_);
  AO_Pattern->POST (&constants::frame_decoded_evt, ao);
}

void
FSP::Pattern_fillFrameBufferWithDecodedFrame (QActive *const ao, QEvt const *e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("filling frame buffer with decoded frame");
  // QS_END()
  AO_Frame->POST (&constants::fill_frame_buffer_with_decoded_frame_evt, ao);
}

void
FSP::Pattern_deferBeginPattern (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->defer (&pattern->begin_pattern_queue_, e);
}

void
FSP::Pattern_recallBeginPattern (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->recall (&pattern->begin_pattern_queue_);
}

void
FSP::Pattern_deferFrameRate (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  if (pattern->frame_rate_queue_.getNFree () > 0)
    {
      pattern->defer (&pattern->frame_rate_queue_, e);
    }
}

void
FSP::Pattern_recallFrameRate (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->recall (&pattern->frame_rate_queue_);
}

void
FSP::Pattern_displayFrame (QActive *const ao, QEvt const *e)
{
  // QS_BEGIN_ID(USER_COMMENT, ao->m_prio)
  //   QS_STR("displaying pattern frame");
  // QS_END()
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  UnsignedValueEvt *uvev = Q_NEW (UnsignedValueEvt, SET_ANALOG_OUTPUT_SIG);
  uvev->value = frameIndexToAnalogOutputValue (
      pattern->frame_index_, pattern->frame_count_per_pattern_);
  AO_Arena->POST (uvev, ao);
  AO_Display->POST (&constants::display_frame_evt, ao);
}

void
FSP::Pattern_initializeFrameIndex (QActive *const ao, QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  if (pattern->frame_index_ >= pattern->frame_count_per_pattern_)
    {
      pattern->frame_index_ = 0;
      pattern->timeout_count_ = 0;
      pattern->timeout_counts_per_frame_index_ = 1;
    }
}

void
FSP::Pattern_setFrameCountPerPattern (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  UnsignedValueEvt const *uvev = static_cast<UnsignedValueEvt const *> (e);
  pattern->frame_count_per_pattern_ = uvev->value;
  if (pattern->frame_index_ >= pattern->frame_count_per_pattern_)
    {
      pattern->frame_index_ = 0;
    }
}

void
FSP::Pattern_setByteCountPerFrame (QP::QActive *const ao, QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  UnsignedValueEvt const *uvev = static_cast<UnsignedValueEvt const *> (e);
  pattern->byte_count_per_frame_ = uvev->value;
}

void
FSP::Pattern_setGrayscaleAndDispatchToCard (QP::QActive *const ao,
                                            QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  UnsignedValueEvt const *uvev = static_cast<UnsignedValueEvt const *> (e);
  pattern->grayscale_ = uvev->value;

  pattern->card_->dispatch (e, pattern->m_prio);
}

void
FSP::Pattern_handleErrorAndDispatchToCard (QP::QActive *const ao,
                                           QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  QF::PUBLISH (&constants::play_pattern_error_evt, ao);
  pattern->card_->dispatch (e, pattern->m_prio);
}

void
FSP::Pattern_dispatchFindPatternToCard (QP::QActive *const ao,
                                        QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  pattern->card_->dispatch (&constants::find_pattern_evt, pattern->m_prio);
}

void
FSP::Pattern_updateAnalogClosedLoopValues (QP::QActive *const ao,
                                           QP::QEvt const *e)
{
  Pattern *const pattern = static_cast<Pattern *const> (ao);
  SignedValueEvt const *svev = static_cast<SignedValueEvt const *> (e);
  int32_t analog_input_millivolts = svev->value;
  int32_t frame_rate
      = ((int32_t)pattern->gain_
         * (analog_input_millivolts + constants::analog_closed_loop_offset))
        / constants::analog_closed_loop_scale_factor;
  if (frame_rate < 0)
    {
      pattern->positive_direction_ = false;
    }
  else
    {
      pattern->positive_direction_ = true;
    }
  frame_rate = abs (frame_rate);
  if (frame_rate > 0)
    {
      pattern->timeout_counts_per_frame_index_
          = constants::analog_closed_loop_frequency_hz / frame_rate;
    }
  else
    {
      pattern->timeout_counts_per_frame_index_ = 0;
    }
  // QS_BEGIN_ID(USER_COMMENT, AO_Pattern->m_prio)
  //   QS_STR("Pattern_updateAnalogClosedLoopValues");
  //   QS_STR("frame_rate");
  //   QS_I32(0, frame_rate);
  //   QS_STR("timeout_counts_per_frame_index");
  //   QS_U32(8, pattern->timeout_counts_per_frame_index_);
  // QS_END()
}

void
FSP::Card_initialize (QHsm *const hsm, QEvt const *e)
{
  Card *const card = static_cast<Card *const> (hsm);
  card->pattern_id_ = 0;
  card->file_size_ = 0;

  QS_SIG_DICTIONARY (FIND_PATTERN_SIG, hsm);
  QS_SIG_DICTIONARY (FIND_CARD_SIG, hsm);
  QS_SIG_DICTIONARY (CARD_FOUND_SIG, hsm);
  QS_SIG_DICTIONARY (CARD_NOT_FOUND_SIG, hsm);
  QS_SIG_DICTIONARY (DIRECTORY_OPEN_SUCCESS_SIG, hsm);
  QS_SIG_DICTIONARY (DIRECTORY_OPEN_FAILURE_SIG, hsm);
  QS_SIG_DICTIONARY (FILE_VALID_SIG, hsm);
  QS_SIG_DICTIONARY (FILE_NOT_VALID_SIG, hsm);
  QS_SIG_DICTIONARY (PATTERN_VALID_SIG, hsm);
  QS_SIG_DICTIONARY (PATTERN_NOT_VALID_SIG, hsm);
  QS_SIG_DICTIONARY (PLAY_PATTERN_SIG, hsm);
  QS_SIG_DICTIONARY (SHOW_PATTERN_FRAME_SIG, hsm);
  QS_SIG_DICTIONARY (ANALOG_CLOSED_LOOP_SIG, hsm);
}

void
FSP::Card_storePlayPatternParameters (QHsm *const hsm, QEvt const *e)
{
  Card *const card = static_cast<Card *const> (hsm);
  PlayPatternEvt const *ppev = static_cast<PlayPatternEvt const *> (e);

  card->pattern_id_ = ppev->pattern_id;
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("play pattern store pattern id");
  QS_U32 (8, card->pattern_id_);
  QS_END ()
}

void
FSP::Card_storeShowPatternFrameParameters (QHsm *const hsm, QEvt const *e)
{
  Card *const card = static_cast<Card *const> (hsm);
  ShowPatternFrameEvt const *spfev
      = static_cast<ShowPatternFrameEvt const *> (e);

  card->pattern_id_ = spfev->pattern_id;
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("show pattern frame store pattern id");
  QS_U32 (8, card->pattern_id_);
  QS_END ()
}

void
FSP::Card_storeAnalogClosedLoopParameters (QHsm *const hsm, QEvt const *e)
{
  Card *const card = static_cast<Card *const> (hsm);
  AnalogClosedLoopEvt const *aclev
      = static_cast<AnalogClosedLoopEvt const *> (e);

  card->pattern_id_ = aclev->pattern_id;
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("analog closed loop store pattern id");
  QS_U32 (8, card->pattern_id_);
  QS_END ()
}

void
FSP::Card_findCard (QHsm *const hsm, QEvt const *e)
{
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("Card_findCard");
  QS_END ()
  if (BSP::findPatternCard ())
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern card found");
      QS_END ()
      AO_Pattern->POST (&constants::card_found_evt, hsm);
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern card not found");
      QS_END ()
      AO_Pattern->POST (&constants::card_not_found_evt, hsm);
    }
}

void
FSP::Card_openDirectory (QHsm *const hsm, QEvt const *e)
{
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("Card_openDirectory");
  QS_END ()
  if (BSP::openPatternDirectory ())
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern directory open success");
      QS_STR (constants::pattern_dir_str);
      QS_END ()
      AO_Pattern->POST (&constants::directory_open_success_evt, hsm);
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern directory open failure");
      QS_STR (constants::pattern_dir_str);
      QS_END ()
      AO_Pattern->POST (&constants::directory_open_failure_evt, hsm);
    }
}

void
FSP::Card_postAllOff (QHsm *const hsm, QEvt const *e)
{
  AO_Arena->POST (&constants::all_off_evt, hsm);
}

void
FSP::Card_openFile (QHsm *const hsm, QEvt const *e)
{
  Card *const card = static_cast<Card *const> (hsm);

  card->file_size_ = BSP::openPatternFileForReading (card->pattern_id_);
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("file opened");
  QS_END ()
}

void
FSP::Card_closeFile (QHsm *const hsm, QEvt const *e)
{
  BSP::closePatternFile ();
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("file closed");
  QS_END ()
}

void
FSP::Card_checkFile (QHsm *const hsm, QEvt const *e)
{
  Card *const card = static_cast<Card *const> (hsm);
  if (card->file_size_)
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("file valid");
      QS_U32 (8, card->pattern_id_);
      QS_U32 (8, card->file_size_);
      QS_END ()
      AO_Pattern->POST (&constants::file_valid_evt, AO_Pattern);
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("file not valid");
      QS_U32 (8, card->pattern_id_);
      QS_END ()
      AO_Pattern->POST (&constants::file_not_valid_evt, AO_Pattern);
    }
}

void
FSP::Card_checkPattern (QHsm *const hsm, QEvt const *e)
{
  Card *const card = static_cast<Card *const> (hsm);

  PatternHeader pattern_header = BSP::rewindPatternFileAndReadHeader ();
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("frame_count_x");
  QS_U16 (5, pattern_header.frame_count_x);
  QS_STR ("frame_count_y");
  QS_U16 (5, pattern_header.frame_count_y);
  QS_STR ("grayscale_value");
  QS_U8 (0, pattern_header.grayscale_value);
  QS_STR ("panel_count_per_frame_row");
  QS_U8 (0, pattern_header.panel_count_per_frame_row);
  QS_STR ("panel_count_per_frame_col");
  QS_U8 (0, pattern_header.panel_count_per_frame_col);
  QS_END ()

  if (pattern_header.panel_count_per_frame_row
      != BSP::getPanelCountPerFrameRow ())
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern frame has incorrect number of rows:");
      QS_STR ("header row count");
      QS_U8 (0, pattern_header.panel_count_per_frame_row);
      QS_STR ("bsp row count");
      QS_U8 (0, BSP::getPanelCountPerRegionRow ());
      QS_END ()
      AO_Pattern->POST (&constants::pattern_not_valid_evt, AO_Pattern);
      return;
    }
  if (pattern_header.panel_count_per_frame_col
      != BSP::getPanelCountPerFrameCol ())
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern frame has incorrect number of cols:");
      QS_STR ("header col count");
      QS_U8 (0, pattern_header.panel_count_per_frame_col);
      QS_STR ("bsp col count");
      QS_U8 (0, BSP::getPanelCountPerRegionCol ()
                    * BSP::getRegionCountPerFrame ());
      QS_END ()
      AO_Pattern->POST (&constants::pattern_not_valid_evt, AO_Pattern);
      return;
    }

  bool grayscale;
  uint64_t byte_count_per_frame;
  switch (pattern_header.grayscale_value)
    {
    case constants::pattern_grayscale_value:
      {
        grayscale = true;
        byte_count_per_frame = BSP::getByteCountPerPatternFrameGrayscale ();
        break;
      }
    case constants::pattern_binary_value:
      {
        grayscale = false;
        byte_count_per_frame = BSP::getByteCountPerPatternFrameBinary ();
        break;
      }
    default:
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern has invalid grayscale value");
      QS_END ()
      AO_Pattern->POST (&constants::pattern_not_valid_evt, AO_Pattern);
      return;
    }

  if ((uint64_t)(pattern_header.frame_count_x * byte_count_per_frame
                 + constants::pattern_header_size)
      != card->file_size_)
    {
      QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
      QS_STR ("pattern frame has incorrect file size");
      QS_END ()
      AO_Pattern->POST (&constants::pattern_not_valid_evt, AO_Pattern);
      return;
    }

  UnsignedValueEvt *set_grayscale_ev
      = Q_NEW (UnsignedValueEvt, SET_GRAYSCALE_SIG);
  set_grayscale_ev->value = grayscale;
  AO_Frame->POST (set_grayscale_ev, AO_Pattern);

  UnsignedValueEvt *set_frame_count_ev
      = Q_NEW (UnsignedValueEvt, SET_FRAME_COUNT_PER_PATTERN_SIG);
  set_frame_count_ev->value = pattern_header.frame_count_x;
  AO_Pattern->POST (set_frame_count_ev, AO_Pattern);

  UnsignedValueEvt *set_byte_count_ev
      = Q_NEW (UnsignedValueEvt, SET_BYTE_COUNT_PER_FRAME_SIG);
  set_byte_count_ev->value = byte_count_per_frame;
  AO_Pattern->POST (set_byte_count_ev, AO_Pattern);

  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("byte count per pattern frame");
  QS_U16 (5, byte_count_per_frame);
  QS_END ()

  UnsignedValueEvt *pattern_valid_ev
      = Q_NEW (UnsignedValueEvt, PATTERN_VALID_SIG);
  pattern_valid_ev->value = grayscale;
  AO_Pattern->POST (pattern_valid_ev, AO_Pattern);
  QS_BEGIN_ID (USER_COMMENT, AO_Pattern->m_prio)
  QS_STR ("pattern valid");
  QS_END ()
}

uint16_t
FSP::frameIndexToAnalogOutputValue (uint16_t frame_index,
                                    uint16_t frame_count_per_pattern)
{
  return (uint32_t)constants::analog_output_min
         + (uint32_t)frame_index
               * (uint32_t)(constants::analog_output_max
                            - constants::analog_output_min)
               / (uint32_t)(frame_count_per_pattern - 1);
}

/**
  @brief Helper function: Appends a text message to a response buffer and
updates the length byte

  Copy the `message` to the end of `response` and increase the overall count of
the response accordingly. Change the first byte of the message to the buffer
length, but excluding that byte itself from the count. This should mirror the
behavior of the G4 Host.

  @param[out] response The response buffer to append the message to
  @param[in,out] response_byte_count Current position in the buffer, updated
after append
  @param[in] message Null-terminated string to append to the response
**/
void
FSP::appendMessage (uint8_t *response, uint8_t &response_byte_count,
                    const char *message)
{
  size_t msg_len = strlen (message);
  memcpy (&response[response_byte_count], message, msg_len);
  response_byte_count += msg_len;
  response[0] = response_byte_count - 1;
}

/**
  @brief Processes a binary command and generates an appropriate response

  Interprets the binary command from the command_buffer, performs the requested
  action by publishing the corresponding event, and generates a response
message. The response format consists of:

  - Byte 0: Length of response (excluding length byte)
  - Byte 1: Status code (0 = success)
  - Byte 2: Echo of the command byte
  - Remaining bytes: Text message describing the action taken, using the
        responses from G4 Host.

  @param[in] command_buffer Buffer containing the binary command to process
  @param[in] command_byte_count Number of bytes in the command buffer
  @param[out] response Buffer to store the generated response

  @return Total number of bytes written to the response buffer
**/
uint8_t
FSP::processBinaryCommand (
    uint8_t const *command_buffer, size_t command_byte_count,
    uint8_t response[constants::byte_count_per_response_max])
{
  uint8_t response_byte_count = 0;

  uint8_t command_buffer_position = 0;
  uint8_t command_byte_count_claim = command_buffer[command_buffer_position++];

  if ((command_byte_count - 1) != command_byte_count_claim)
    {
      appendMessage (response, response_byte_count, "Invalid-Command");
      QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
      QS_STR ("invalid command");
      QS_END ()
      return response_byte_count;
    }

  uint8_t command_byte = command_buffer[command_buffer_position++];
  response[response_byte_count++] = 2;
  response[response_byte_count++] = 0;
  response[response_byte_count++] = command_byte;
  switch (command_byte)
    {
    case ALL_OFF_CMD:
      {
        AO_Arena->POST (&constants::all_off_evt, &constants::fsp_id);
        appendMessage (response, response_byte_count, "All-Off Received");
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("all-off command");
        QS_END ()
        break;
      }
    case DISPLAY_RESET_CMD:
      {
        AO_Watchdog->POST (&constants::reset_evt, &constants::fsp_id);
        appendMessage (response, response_byte_count,
                       "Reset Command Sent to FPGA");
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("display-reset command");
        QS_END ()
        break;
      }
    case SWITCH_GRAYSCALE_CMD:
      {
        uint8_t grayscale_index;
        memcpy (&grayscale_index, command_buffer + command_buffer_position,
                sizeof (grayscale_index));

        AO_Arena->POST (&constants::all_off_evt, &constants::fsp_id);

        UnsignedValueEvt *uvev = Q_NEW (UnsignedValueEvt, SET_GRAYSCALE_SIG);
        uvev->value = grayscale_index;
        AO_Frame->POST (uvev, &constants::fsp_id);

        appendMessage (response, response_byte_count, "");
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("switch-grayscale command");
        QS_END ()
        break;
      }
    case TRIAL_PARAMS_CMD:
      {
        uint8_t control_mode;
        memcpy (&control_mode, command_buffer + command_buffer_position,
                sizeof (control_mode));
        command_buffer_position += sizeof (control_mode);

        uint16_t pattern_id;
        memcpy (&pattern_id, command_buffer + command_buffer_position,
                sizeof (pattern_id));
        command_buffer_position += sizeof (pattern_id);

        int16_t frame_rate;
        memcpy (&frame_rate, command_buffer + command_buffer_position,
                sizeof (frame_rate));
        command_buffer_position += sizeof (frame_rate);

        uint16_t frame_index;
        memcpy (&frame_index, command_buffer + command_buffer_position,
                sizeof (frame_index));
        command_buffer_position += sizeof (frame_index);

        uint16_t gain;
        memcpy (&gain, command_buffer + command_buffer_position,
                sizeof (gain));
        command_buffer_position += sizeof (gain);

        uint16_t runtime_duration;
        memcpy (&runtime_duration, command_buffer + command_buffer_position,
                sizeof (runtime_duration));
        command_buffer_position += sizeof (runtime_duration);

        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("trial-params command");
        QS_END ()
        switch (control_mode)
          {
          case PLAY_PATTERN_MODE:
            {
              PlayPatternEvt *ppev = Q_NEW (PlayPatternEvt, PLAY_PATTERN_SIG);
              ppev->pattern_id = pattern_id;
              ppev->frame_rate = frame_rate;
              ppev->runtime_duration = runtime_duration;
              ppev->frame_index = frame_index;
              QF::PUBLISH (ppev, &constants::fsp_id);

              appendMessage (response, response_byte_count, "");
              QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
              QS_STR ("play pattern mode");
              QS_U16 (5, pattern_id);
              QS_U16 (5, frame_rate);
              QS_U16 (5, runtime_duration);
              QS_U16 (5, frame_index);
              QS_END ()
              break;
            }
          case SHOW_PATTERN_FRAME_MODE:
            {
              ShowPatternFrameEvt *spfev
                  = Q_NEW (ShowPatternFrameEvt, SHOW_PATTERN_FRAME_SIG);
              spfev->pattern_id = pattern_id;
              spfev->frame_index = frame_index;
              QF::PUBLISH (spfev, &constants::fsp_id);

              appendMessage (response, response_byte_count, "");
              QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
              QS_STR ("showing pattern frame mode");
              QS_U16 (5, pattern_id);
              QS_U16 (5, frame_index);
              QS_END ()
              break;
            }
          case ANALOG_CLOSED_LOOP_MODE:
            {
              AnalogClosedLoopEvt *aclev
                  = Q_NEW (AnalogClosedLoopEvt, ANALOG_CLOSED_LOOP_SIG);
              aclev->pattern_id = pattern_id;
              aclev->gain = gain;
              aclev->runtime_duration = runtime_duration;
              aclev->frame_index = frame_index;
              QF::PUBLISH (aclev, &constants::fsp_id);

              appendMessage (response, response_byte_count, "");
              QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
              QS_STR ("analog closed loop mode");
              QS_U16 (5, pattern_id);
              QS_U16 (5, gain);
              QS_U16 (5, runtime_duration);
              QS_U16 (5, frame_index);
              QS_END ()
              break;
            }
          default:
            break;
          }
        break;
      }
    case SET_REFRESH_RATE_CMD:
      {
        uint16_t refresh_rate;
        memcpy (&refresh_rate, command_buffer + command_buffer_position,
                sizeof (refresh_rate));

        UnsignedValueEvt *uvev
            = Q_NEW (UnsignedValueEvt, SET_REFRESH_RATE_SIG);
        uvev->value = refresh_rate;
        AO_Display->POST (uvev, &constants::fsp_id);

        appendMessage (response, response_byte_count, "");
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("set-refresh-rate command");
        QS_END ()
        break;
      }
    case STOP_DISPLAY_CMD:
      {
        AO_Arena->POST (&constants::all_off_evt, &constants::fsp_id);
        appendMessage (response, response_byte_count,
                       "Display has been stopped");
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("stop-display command");
        QS_END ()
        break;
      }
    case GET_ETHERNET_IP_ADDRESS_CMD:
      {
        appendMessage (response, response_byte_count,
                       BSP::getEthernetIpAddress ());
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("get-ethernet-ip-address command");
        QS_END ()
        break;
      }
    case SET_FRAME_POSITION_CMD:
      {
        uint16_t frame_index;
        memcpy (&frame_index, command_buffer + command_buffer_position,
                sizeof (frame_index));

        UnsignedValueEvt *uvev
            = Q_NEW (UnsignedValueEvt, UPDATE_PATTERN_FRAME_SIG);
        uvev->value = frame_index;
        AO_Pattern->POST (uvev, &constants::fsp_id);

        appendMessage (response, response_byte_count, "");
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("set-frame-position command");
        QS_END ()
        break;
      }
    case ALL_ON_CMD:
      {
        AO_Arena->POST (&constants::all_on_evt, &constants::fsp_id);
        appendMessage (response, response_byte_count, "All-On Received");
        QS_BEGIN_ID (USER_COMMENT, AO_Arena->m_prio)
        QS_STR ("all-on command");
        QS_END ()
        break;
      }
    default:
      break;
    }
  return response_byte_count;
}

void
FSP::processStreamCommand (uint8_t const *stream_buffer,
                           uint32_t stream_byte_count)
{
  uint8_t const *frame_buffer
      = stream_buffer + constants::stream_header_byte_count;
  uint32_t frame_byte_count
      = stream_byte_count - constants::stream_header_byte_count;

  bool grayscale;
  if (frame_byte_count == BSP::getByteCountPerPatternFrameGrayscale ())
    {
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("streamed frame has grayscale size");
      QS_END ()
      grayscale = true;
    }
  else if (frame_byte_count == BSP::getByteCountPerPatternFrameBinary ())
    {
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("streamed frame has binary size");
      QS_END ()
      grayscale = false;
    }
  else
    {
      QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
      QS_STR ("streamed frame has invalid size");
      QS_U32 (8, frame_byte_count);
      QS_U32 (8, BSP::getByteCountPerPatternFrameGrayscale ());
      QS_U32 (8, BSP::getByteCountPerPatternFrameBinary ());
      QS_END ()
      AO_Arena->POST (&constants::all_off_evt, &constants::fsp_id);
      return;
    }
  UnsignedValueEvt *set_grayscale_ev
      = Q_NEW (UnsignedValueEvt, SET_GRAYSCALE_SIG);
  set_grayscale_ev->value = grayscale;
  AO_Frame->POST (set_grayscale_ev, &constants::fsp_id);

  // skip byte 0 (command byte) and bytes 1 and 2 (data length)
  uint8_t stream_buffer_position = 3;
  uint16_t analog_output_value_x;
  memcpy (&analog_output_value_x, stream_buffer + stream_buffer_position,
          sizeof (analog_output_value_x));
  stream_buffer_position += sizeof (analog_output_value_x);
  uint16_t analog_output_value_y;
  memcpy (&analog_output_value_y, stream_buffer + stream_buffer_position,
          sizeof (analog_output_value_y));
  stream_buffer_position += sizeof (analog_output_value_y);
  UnsignedValueEvt *set_analog_output_ev
      = Q_NEW (UnsignedValueEvt, SET_ANALOG_OUTPUT_SIG);
  set_analog_output_ev->value = analog_output_value_x;
  AO_Arena->POST (set_analog_output_ev, &constants::fsp_id);
  QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
  QS_STR ("analog_output_value_x");
  QS_U32 (8, analog_output_value_x);
  QS_STR ("analog_output_value_y");
  QS_U32 (8, analog_output_value_y);
  QS_END ()

  uint16_t bytes_decoded
      = BSP::decodePatternFrameBuffer (frame_buffer, grayscale);
  AO_Arena->POST (&constants::stream_frame_evt, &constants::fsp_id);
  QS_BEGIN_ID (USER_COMMENT, AO_EthernetCommandInterface->m_prio)
  QS_STR ("processed stream command");
  QS_U32 (8, bytes_decoded);
  QS_END ()
}

void
FSP::processStringCommand (const char *command, char *response)
{
  strcpy (response, command);
  if (strcmp (command, "RESET") == 0)
    {
      AO_Watchdog->POST (&constants::reset_evt, &constants::fsp_id);
    }
  if (strcmp (command, "LED_ON") == 0)
    {
      BSP::ledOn ();
    }
  else if (strcmp (command, "LED_OFF") == 0)
    {
      BSP::ledOff ();
    }
  else if (strcmp (command, "ALL_ON") == 0)
    {
      AO_Arena->POST (&constants::all_on_evt, &constants::fsp_id);
    }
  else if (strcmp (command, "ALL_OFF") == 0)
    {
      AO_Arena->POST (&constants::all_off_evt, &constants::fsp_id);
    }
  else if (strcmp (command, "EHS") == 0)
    {
      // BSP::getEthernetHardwareStatusString(response);
    }
  else if (strcmp (command, "ELS") == 0)
    {
      // BSP::getEthernetLinkStatusString(response);
    }
  else if (strcmp (command, "SIP") == 0)
    {
      // BSP::getServerIpAddressString(response);
    }
  else if (strcmp (command, "SET_REFRESH_RATE") == 0)
    {
      // command.replace("SET_REFRESH_RATE", "") == 0;
      // command.trim();
      // uint32_t frequency_hz = command.toInt();
      // BSP::setRefreshRate(frequency_hz);
    }
}
