#include <SPI.h>
#include <EventResponder.h>
#define SPIT SPI2
#define DBGSerial Serial4
#define CS_PIN 10
#define SMALL_TRANSFER_SIZE 128
//#define BUFFER_SIZE 70000 //(320*240*2) // size of ILI9341 display...
#define BUFFER_SIZE 0x12000l   // More than 64K
//uint8_t buffer[BUFFER_SIZE];
uint8_t *buffer;  // lets malloc it...
//uint8_t rxBuffer[SMALL_TRANSFER_SIZE];
DMAMEM uint8_t rxBuffer[BUFFER_SIZE];
//uint8_t *rxBuffer;
uint8_t *foo_bar = nullptr;
uint8_t static_buffer[16];

#define SERIAL_tt Serial1
// Trigger spare interrupts
IntervalTimer ITtest;
volatile uint32_t kk, jj = 0;
uint32_t tt = 0;
void TimeSome() {
  jj++;
  kk = micros();
  if ( !(jj % 10000) )   SERIAL_tt.print("*");
  if ( !(jj % 130000) )   SERIAL_tt.print("!\n");
}
#define CHANGE_SPEED // define to cycle the CPU speed
extern "C" uint32_t set_arm_clock(uint32_t frequency);
#define MAX_SPEED 800000000
#define MIN_SPEED 96000000
uint32_t ArmSpeed = 6 * 100000000;

EventResponder event;
volatile bool event_happened = false;
void asyncEventResponder(EventResponderRef event_responder)
{
  digitalWriteFast(CS_PIN, HIGH);
  event_happened = true;
}

void setup() {
  // debug pins
  uint8_t stack_buffer[10];
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  digitalWrite(0, LOW);
  digitalWrite(1, LOW);
  extern unsigned long _heap_start;
  extern unsigned long _heap_end;

  pinMode(CS_PIN, OUTPUT);
  digitalWriteFast(CS_PIN, HIGH);
  while (!Serial && millis() < 4000) ;  // wait for Serial port
  DBGSerial.begin(115200);
  SPIT.begin();
  DBGSerial.println("SPI Test program");

  buffer = (uint8_t *)malloc(BUFFER_SIZE);
  //rxBuffer = (uint8_t *)malloc(BUFFER_SIZE);

  SERIAL_tt.begin( 115200 );
  SERIAL_tt.println("\n********\n T4 connected Serial1 *******\n");

  DBGSerial.print("Buffer: ");
  DBGSerial.print((uint32_t)buffer, HEX);
  DBGSerial.print(" RX Buffer: ");
  DBGSerial.print((uint32_t)rxBuffer, HEX);
  DBGSerial.print(" ");
  DBGSerial.println(BUFFER_SIZE, DEC);
  DBGSerial.printf("Static buffer: %x, Stack Buffer: %x\n", (uint32_t)static_buffer, (uint32_t)stack_buffer);
  DBGSerial.printf("Heap Start: %x, Heap End: %x\n", (uint32_t)&_heap_start, (uint32_t)&_heap_end);
  event.attachImmediate(&asyncEventResponder);
  ITtest.begin( TimeSome, 2);
}
int nn = 0;
void loop() {
  // put your main code here, to run repeatedly:
  while (DBGSerial.read() != -1) ; // Make sure queue is empty.
  DBGSerial.println("Press any key to run test");
  //while (!DBGSerial.available()) ; // will loop until it receives something
  while (DBGSerial.read() != -1) ; // loop until queue is empty
  DBGSerial.println("Ready to start tests");
  DBGSerial.print("IntvTimer jj=");
  DBGSerial.print( jj );
  DBGSerial.print("\tIntvTimer jj=");
  DBGSerial.print( jj );
  DBGSerial.print("\tms Time=");
  DBGSerial.println( millis() - tt );
  DBGSerial.print("\tms Time=");
  DBGSerial.print( millis() - tt );
  tt = millis();
  jj = 0;
  DBGSerial.printf( "    deg C=%2.2f\n" , tempmonGetTemp() );
  delay( 500 );
#ifdef CHANGE_SPEED
  if ( !(nn % 4) ) {      // Change Clock Speed
    ArmSpeed -= 100000000;
    if ( ArmSpeed < MIN_SPEED ) ArmSpeed = MAX_SPEED;
    set_arm_clock( ArmSpeed );
    if ( F_CPU_ACTUAL < MIN_SPEED ) ArmSpeed = MAX_SPEED;
    set_arm_clock( ArmSpeed );
    DBGSerial.printf("\t>>> Clock Speed is:%u", F_CPU_ACTUAL);
  }
#endif
  delay( 500 );
  nn++;


  SPIT.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  DBGSerial.println("After Begin Transaction");

  //=================================================================
  // Transfer Sync
  //=================================================================


  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  for (uint32_t i = 0; i < BUFFER_SIZE; i++)  rxBuffer[i] = 0x5a;
  DBGSerial.println("Transfer Small"); //DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, rxBuffer, SMALL_TRANSFER_SIZE);
  digitalWriteFast(CS_PIN, HIGH);
  DBGSerial.println("*** Completed ***"); DBGSerial.flush();
  dumpBuffer(buffer, SMALL_TRANSFER_SIZE);
  DBGSerial.println();
  dumpBuffer(rxBuffer, SMALL_TRANSFER_SIZE);
  validateTXBuffer(0);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  DBGSerial.println("write Small"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, NULL, SMALL_TRANSFER_SIZE);
  digitalWriteFast(CS_PIN, HIGH);
  DBGSerial.println("*** Completed ***"); DBGSerial.flush();
  validateTXBuffer(0);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  DBGSerial.println("read Small"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(NULL, rxBuffer, SMALL_TRANSFER_SIZE);
  digitalWriteFast(CS_PIN, HIGH);
  DBGSerial.println("*** Completed ***"); DBGSerial.flush();
  dumpBuffer(rxBuffer, SMALL_TRANSFER_SIZE);
  delay(5);

  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  for (uint32_t i = 0; i < BUFFER_SIZE; i++)buffer[i] = i / 1024;
  for (uint32_t i = 0; i < BUFFER_SIZE; i++)  rxBuffer[i] = 0x5a;

  DBGSerial.println("Transfer Full"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, rxBuffer, BUFFER_SIZE);
  digitalWriteFast(CS_PIN, HIGH);
  validateTXBuffer(1);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i / 1024;
  DBGSerial.println("write full"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, NULL, BUFFER_SIZE);
  digitalWriteFast(CS_PIN, HIGH);
  validateTXBuffer(1);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  DBGSerial.println("read full"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(NULL, buffer, BUFFER_SIZE);
  digitalWriteFast(CS_PIN, HIGH);
  delay(5);
  //=================================================================
  // Transfer Async
  //=================================================================
  for (uint32_t i = 0; i < 5; i++) {
    digitalWriteFast(CS_PIN, LOW);
    delay(1);
    digitalWriteFast(CS_PIN, HIGH);
    delay(1);
  }
  event_happened = false;
  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  DBGSerial.println("Async write Small"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, NULL, SMALL_TRANSFER_SIZE, event);
  DBGSerial.println("After write call, waiting for event");
  while (!event_happened) ;
  event_happened = false;
  validateTXBuffer(0);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  for (uint32_t i = 0; i < BUFFER_SIZE; i++)  rxBuffer[i] = 0x5a;
  DBGSerial.println("Async Transfer Small"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, rxBuffer, SMALL_TRANSFER_SIZE, event);
  DBGSerial.println("After Transfer call, waiting for event");
  while (!event_happened) ;
  event_happened = false;
  dumpBuffer(buffer, SMALL_TRANSFER_SIZE);
  DBGSerial.println();
  dumpBuffer(rxBuffer, SMALL_TRANSFER_SIZE);
  validateTXBuffer(0);
  delay(5);


  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  for (uint32_t i = 0; i < BUFFER_SIZE; i++)  rxBuffer[i] = 0x5a;
  DBGSerial.println("Async read Small"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.setTransferWriteFill(0x42);
  SPIT.transfer(NULL, rxBuffer, SMALL_TRANSFER_SIZE, event);
  //arm_dcache_delete(rxBuffer, SMALL_TRANSFER_SIZE);
  while (!event_happened) ;
  event_happened = false;
  dumpBuffer(rxBuffer, SMALL_TRANSFER_SIZE);
  validateTXBuffer(0);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i / 1024;
  for (uint32_t i = 0; i < BUFFER_SIZE; i++)  rxBuffer[i] = 0x5a;
  DBGSerial.println("Async Transfer Full"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, rxBuffer, BUFFER_SIZE, event);
  while (!event_happened) ;
  event_happened = false;
  dumpBuffer(rxBuffer, 512);
  validateTXBuffer(1);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i / 1024;
  DBGSerial.println("Async write full"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(buffer, NULL, BUFFER_SIZE, event);
  while (!event_happened) ;
  event_happened = false;
  validateTXBuffer(1);
  delay(5);

  for (uint32_t i = 0; i < BUFFER_SIZE; i++) buffer[i] = i & 0xff;
  for (uint32_t i = 0; i < BUFFER_SIZE; i++)  rxBuffer[i] = 0x5a;
  DBGSerial.println("Async read full"); DBGSerial.flush();
  digitalWriteFast(CS_PIN, LOW);
  SPIT.transfer(NULL, rxBuffer, BUFFER_SIZE, event);
  while (!event_happened) ;
  event_happened = false;
  dumpBuffer(rxBuffer, 512);
  validateTXBuffer(0);
  delay(5);


  DBGSerial.println("Tests completed");
  SPIT.endTransaction();
}

void dumpBuffer(uint8_t *pb, int cb) {
  uint8_t i = 0;
  while (cb) {
    DBGSerial.print(*pb++, HEX);
    cb--;
    DBGSerial.print(" ");
    i++;
    if (i == 16) {
      DBGSerial.println();
      i = 0;
    }
  }
  DBGSerial.println();
}
void validateTXBuffer(uint8_t test)
{
  uint8_t error_count = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (((test == 0) && (buffer[i] != (i & 0xff)))
        || ((test == 1) && (buffer[i] != (i / 1024)))) {
      DBGSerial.print("Tx Buffer validate failed Index: ");
      DBGSerial.print(i, DEC);
      DBGSerial.print(" Value: ");
      DBGSerial.println(buffer[i], HEX);
      error_count++;
      DBGSerial.print("Tx Buffer validate failed Index: ");
      DBGSerial.print(i, DEC);
      if (error_count == 10)
        break;
    }
  }
}
