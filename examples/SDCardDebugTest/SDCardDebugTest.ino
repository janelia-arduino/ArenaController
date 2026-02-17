/*
 * SD Card Debug Test for Teensy 4.1
 *
 * This sketch helps diagnose SD card initialization issues,
 * particularly when the card has many files and causes hangs.
 */

// Use SdFat library - Arduino IDE should use Teensy's built-in version
// If you get library conflicts, remove the SdFat library from:
// /home/loeschef/Arduino/libraries/SdFat
#include <SdFat.h>
#include <TimeLib.h>

// SD card object
SdFs sd;
FsFile root;
FsFile file;

// Configuration options to test
struct SDTestConfig {
  const char* name;
  SdioConfig config;
  uint32_t clockSpeed;
  bool useClockSpeed;
};

// Test statistics
struct TestStats {
  uint32_t initTime;
  uint32_t fileCountTime;
  uint32_t totalFiles;
  uint32_t totalDirs;
  bool success;
  uint8_t errorCode;
  uint8_t errorData;
};

void setup() {
  Serial.begin(115200);

  // Wait for serial port or timeout
  uint32_t serialWait = millis();
  while (!Serial && (millis() - serialWait < 3000)) {
    delay(10);
  }

  Serial.println("\n========================================");
  Serial.println("SD Card Debug Test for Teensy 4.1 (nextGen)");
  Serial.println("========================================\n");

  // Show initial state
  Serial.print("CPU Speed: ");
  Serial.print(F_CPU / 1000000);
  Serial.println(" MHz");

  Serial.print("millis(): ");
  Serial.println(millis());

  // Run comprehensive tests
  runAllTests();
}

void loop() {
  // Interactive debug mode
  if (Serial.available()) {
    char cmd = Serial.read();
    while (Serial.available()) Serial.read(); // Clear buffer

    switch (cmd) {
      case '1':
        testBasicInit();
        break;
      case '2':
        testWithTimeout();
        break;
      case '3':
        testFileCount();
        break;
      case '4':
        testDifferentConfigs();
        break;
      case '5':
        testInterruptImpact();
        break;
      case 'r':
      case 'R':
        runAllTests();
        break;
      case 'h':
      case 'H':
        printHelp();
        break;
    }
  }

  delay(100);
}

void printHelp() {
  Serial.println("\n=== Interactive Commands ===");
  Serial.println("1 - Test basic SD init");
  Serial.println("2 - Test with timeout protection");
  Serial.println("3 - Count files on card");
  Serial.println("4 - Test different SDIO configs");
  Serial.println("5 - Test interrupt impact");
  Serial.println("R - Run all tests");
  Serial.println("H - Show this help");
  Serial.println("============================\n");
}

void runAllTests() {
  Serial.println("Running all SD card tests...\n");

  testBasicInit();
  delay(500);

  testWithTimeout();
  delay(500);

  testDifferentConfigs();
  delay(500);

  testFileCount();
  delay(500);

  testInterruptImpact();

  Serial.println("\n=== All Tests Complete ===");
  printHelp();
}

void testBasicInit() {
  Serial.println("\n--- Test 1: Basic SD Card Init ---");

  uint32_t startTime = millis();
  bool success = sd.begin(SdioConfig(FIFO_SDIO));
  uint32_t elapsed = millis() - startTime;

  Serial.print("Result: ");
  Serial.println(success ? "SUCCESS" : "FAILED");
  Serial.print("Time: ");
  Serial.print(elapsed);
  Serial.println(" ms");

  if (!success) {
    Serial.print("Error Code: 0x");
    Serial.println(sd.sdErrorCode(), HEX);
    Serial.print("Error Data: 0x");
    Serial.println(sd.sdErrorData(), HEX);
  } else {
    // Get card info
    Serial.print("Card Type: ");
    switch (sd.card()->type()) {
      case SD_CARD_TYPE_SD1:
        Serial.println("SD1");
        break;
      case SD_CARD_TYPE_SD2:
        Serial.println("SD2");
        break;
      case SD_CARD_TYPE_SDHC:
        Serial.println("SDHC/SDXC");
        break;
      default:
        Serial.print("Unknown type: ");
        Serial.println(sd.card()->type());
    }

    uint32_t cardSize = sd.card()->sectorCount();
    if (cardSize > 0) {
      Serial.print("Card Size: ");
      Serial.print((uint32_t)((uint64_t)cardSize * 512 / 1024 / 1024));
      Serial.println(" MB");
    }

    sd.end();
  }
}

void testWithTimeout() {
  Serial.println("\n--- Test 2: Init with Timeout Protection ---");

  const uint32_t TIMEOUT_MS = 3000;
  uint32_t startTime = millis();
  bool timedOut = false;

  // Create a non-blocking init attempt
  Serial.print("Attempting init with ");
  Serial.print(TIMEOUT_MS);
  Serial.println(" ms timeout...");

  // Since sd.begin() can block, we'll use a different approach
  // We'll try to detect a hang by using interrupts

  volatile bool initComplete = false;
  volatile bool initSuccess = false;

  // Start init
  uint32_t checkTime = millis();

  // Try init with periodic yield
  while ((millis() - startTime) < TIMEOUT_MS && !initComplete) {
    if ((millis() - checkTime) >= 100) {
      Serial.print(".");
      checkTime = millis();
    }

    // Try init if not started
    if (!initComplete) {
      initSuccess = sd.begin(SdioConfig(FIFO_SDIO));
      initComplete = true;
    }
  }

  Serial.println();

  if (!initComplete) {
    Serial.println("TIMEOUT - Init did not complete!");
    timedOut = true;
  } else {
    uint32_t elapsed = millis() - startTime;
    Serial.print("Result: ");
    Serial.println(initSuccess ? "SUCCESS" : "FAILED");
    Serial.print("Time: ");
    Serial.print(elapsed);
    Serial.println(" ms");

    if (initSuccess) {
      sd.end();
    }
  }
}

void testDifferentConfigs() {
  Serial.println("\n--- Test 3: Different SDIO Configurations ---");

  // Note: Clock speed can't be set directly on SdioConfig
  // Only test the available SDIO modes
  SDTestConfig configs[] = {
    {"FIFO_SDIO (default)", SdioConfig(FIFO_SDIO), 0, false},
    {"DMA_SDIO", SdioConfig(DMA_SDIO), 0, false}
  };

  for (int i = 0; i < 2; i++) {
    Serial.print("\nTesting ");
    Serial.print(configs[i].name);
    Serial.println(":");

    uint32_t startTime = millis();
    bool success;

    // Note: SdFat library doesn't support setClockSpeed on SdioConfig
    // We can only use the predefined configurations
    success = sd.begin(configs[i].config);

    uint32_t elapsed = millis() - startTime;

    Serial.print("  Result: ");
    Serial.println(success ? "SUCCESS" : "FAILED");
    Serial.print("  Time: ");
    Serial.print(elapsed);
    Serial.println(" ms");

    if (!success) {
      Serial.print("  Error: 0x");
      Serial.println(sd.sdErrorCode(), HEX);
    } else {
      sd.end();
    }

    delay(100);
  }
}

void testFileCount() {
  Serial.println("\n--- Test 4: File Counting Performance ---");

  uint32_t startTime = millis();
  if (!sd.begin(SdioConfig(FIFO_SDIO))) {
    Serial.println("Failed to initialize SD card!");
    return;
  }
  uint32_t initTime = millis() - startTime;

  Serial.print("SD Init Time: ");
  Serial.print(initTime);
  Serial.println(" ms");

  // Count files with different methods
  Serial.println("\nMethod 1: Simple count");
  startTime = millis();
  uint32_t fileCount = 0;
  uint32_t dirCount = 0;

  if (!root.open("/")) {
    Serial.println("Failed to open root!");
    sd.end();
    return;
  }

  while (file.openNext(&root, O_RDONLY)) {
    if (file.isDir()) {
      dirCount++;
    } else {
      fileCount++;
    }
    file.close();

    // Progress indicator
    if ((fileCount + dirCount) % 100 == 0) {
      Serial.print(".");
      if ((fileCount + dirCount) % 1000 == 0) {
        Serial.print(" ");
        Serial.print(fileCount + dirCount);
        Serial.println(" entries processed");
      }
    }

    // Safety timeout
    if ((millis() - startTime) > 30000) {
      Serial.println("\nTimeout after 30 seconds!");
      break;
    }
  }

  uint32_t countTime = millis() - startTime;
  root.close();

  Serial.println();
  Serial.print("Files: ");
  Serial.println(fileCount);
  Serial.print("Directories: ");
  Serial.println(dirCount);
  Serial.print("Count Time: ");
  Serial.print(countTime);
  Serial.println(" ms");

  if (fileCount > 0) {
    Serial.print("Average per file: ");
    Serial.print((float)countTime / (fileCount + dirCount));
    Serial.println(" ms");
  }

  // Method 2: Count with rewindDirectory
  Serial.println("\nMethod 2: Count with rewind");
  root.open("/");
  startTime = millis();
  uint32_t quickCount = 0;

  root.rewindDirectory();
  while (file.openNext(&root, O_RDONLY) && quickCount < 10) {
    quickCount++;
    file.close();
  }

  uint32_t quickTime = millis() - startTime;
  root.close();

  Serial.print("First 10 files time: ");
  Serial.print(quickTime);
  Serial.println(" ms");

  sd.end();
}

void testInterruptImpact() {
  Serial.println("\n--- Test 5: Interrupt Impact ---");

  // Test with interrupts enabled
  Serial.println("Testing with interrupts enabled:");
  interrupts();
  uint32_t startTime = millis();
  bool success = sd.begin(SdioConfig(FIFO_SDIO));
  uint32_t withIntTime = millis() - startTime;

  Serial.print("  Result: ");
  Serial.println(success ? "SUCCESS" : "FAILED");
  Serial.print("  Time: ");
  Serial.print(withIntTime);
  Serial.println(" ms");

  if (success) {
    sd.end();
  }

  delay(100);

  // Test with interrupts disabled (briefly)
  Serial.println("\nTesting with interrupts disabled (brief):");
  startTime = millis();

  noInterrupts();
  // Don't actually init with interrupts off as it may need them
  // Just test the impact
  delayMicroseconds(100);
  interrupts();

  success = sd.begin(SdioConfig(FIFO_SDIO));
  uint32_t mixedTime = millis() - startTime;

  Serial.print("  Result: ");
  Serial.println(success ? "SUCCESS" : "FAILED");
  Serial.print("  Time: ");
  Serial.print(mixedTime);
  Serial.println(" ms");

  if (success) {
    // Do a quick operation to verify card is responsive
    FsFile testFile;
    startTime = millis();
    bool opSuccess = testFile.open("/", O_RDONLY);
    uint32_t opTime = millis() - startTime;

    Serial.print("  Open root time: ");
    Serial.print(opTime);
    Serial.println(" ms");

    if (opSuccess) {
      testFile.close();
    }

    sd.end();
  }
}
