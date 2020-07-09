#pragma once
#include "windows.h"
#include <string>

class SerialPort {
public:
  enum class BaudRate { Baud9600 = 9600, Baud115200 = 115200 };
  enum class DataBits { Data8 = 8 };

public:
  ~SerialPort();

  bool open(std::string portName, bool overlapped = false, BaudRate baud = BaudRate::Baud9600,
            DataBits dataBits = DataBits::Data8);
  bool close();
  bool reopen();

  int read(unsigned char *buffer, int limit);
  int write(const unsigned char *buffer, int size);

  bool isOpened() { return (opened); }

private:
  HANDLE hComm = NULL;
  OVERLAPPED overlappedRead{}, overlappedWrite{};
  bool opened = false;
  bool overlapped = false;
  std::string portName{};
  BaudRate baudRate{};
  DataBits dataBits{};
};
