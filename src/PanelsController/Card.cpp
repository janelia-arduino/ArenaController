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
  sd_.mkdir("display");
}

void Card::chdirDisplay()
{
  sd_.chdir("display");
}

void Card::openFileForWriting(const char* path)
{
  file_.open(path, O_WRONLY | O_CREAT);
}
