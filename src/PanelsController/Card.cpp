// Card.cpp
//
//
// Authors:
// Peter Polidoro peter@polidoro.io
// ----------------------------------------------------------------------------
#include "Card.h"


using namespace panels_controller;

void Card::setup()
{
  sd_.begin(SD_CONFIG);
}

void Card::mkdirDisplay()
{
  sd_.mkdir(DIR);
}

void Card::chdirDisplay()
{
  sd_.chdir(DIR);
}

void Card::openFileForWriting(const char* path, uint64_t file_length)
{
  file_.remove(path);
  file_.open(path, O_WRITE | O_CREAT | O_APPEND);
  file_.preAllocate(file_length);
}

void Card::openNextFileForReading()
{
  file_.openNext(DIR);
  file_.rewind();
}

void Card::closeFile()
{
  file_.close();
}

void Card::writeToFile(const uint8_t * buf, size_t count)
{
  file_.write(buf, count);
  file_.sync();
}
