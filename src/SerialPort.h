#pragma once
#include "windows.h"
#include <string>

class SerialPort {
public:
  enum class BaudRate { Baud9600 = 9600, Baud115200 = 115200 };
  enum class DataBits { Data8 = 8 };

public:
  ~SerialPort();

  bool open(std::string, BaudRate = BaudRate::Baud115200,
            DataBits = DataBits::Data8);
  bool close();

  int read(void *buffer, int limit);
  int write(const char *buffer, int size);

  bool isOpened() { return (opened); }

private:
  HANDLE hComm = NULL;
  OVERLAPPED overlappedRead{}, overlappedWrite{};
  bool opened = false;
};
