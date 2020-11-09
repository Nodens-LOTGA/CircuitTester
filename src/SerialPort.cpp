#include "SerialPort.h"
#include <QDebug>

SerialPort::~SerialPort() { close(); }

bool SerialPort::open(std::string portName, bool overlapped, BaudRate baud,
                      DataBits dataBits) {
  if (opened)
    return (true);

  hComm = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0,
                     OPEN_EXISTING, overlapped ? FILE_FLAG_OVERLAPPED : 0, 0);
  if (hComm == INVALID_HANDLE_VALUE || hComm == NULL) {
    qDebug() << "Failed to open serial port.";
    return false;
  }

  COMMTIMEOUTS timeouts{};
  timeouts.ReadIntervalTimeout = overlapped ? MAXDWORD : 25;
  timeouts.ReadTotalTimeoutConstant = overlapped ? 0 : 1;
  timeouts.ReadTotalTimeoutMultiplier = overlapped ? 0 : 1;
  timeouts.WriteTotalTimeoutMultiplier = overlapped ? 0 : 1;
  timeouts.WriteTotalTimeoutConstant = 25;
  SetCommTimeouts(hComm, &timeouts);

  if (overlapped) {
    overlappedRead = overlappedWrite = OVERLAPPED{};
    overlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    overlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  }

  DCB dcb{};
  dcb.DCBlength = sizeof(dcb);
  GetCommState(hComm, &dcb);
  dcb.BaudRate = static_cast<DWORD>(baud);
  dcb.ByteSize = static_cast<DWORD>(dataBits);

  if (overlapped) {
    if (overlappedRead.hEvent == NULL || overlappedWrite.hEvent == NULL) {
      DWORD dwError = GetLastError();
      if (overlappedRead.hEvent != NULL)
        CloseHandle(overlappedRead.hEvent);
      if (overlappedWrite.hEvent != NULL)
        CloseHandle(overlappedWrite.hEvent);
      CloseHandle(hComm);
      qDebug("Failed to setup serial port. Error: %i", dwError);
      return false;
    }
  }

  if (!SetCommState(hComm, &dcb) || !SetupComm(hComm, 1000, 1000)) {
    DWORD dwError = GetLastError();
    CloseHandle(hComm);
    qDebug("Failed to setup serial port. Error: %i", dwError);
    return false;
  } else
    qDebug() << "Port was opened.";

  opened = true;
  this->overlapped = overlapped;
  this->portName = portName;
  this->baudRate = baud;
  this->dataBits = dataBits;
  return opened;
}

bool SerialPort::close() {
  if (!opened || hComm == NULL || hComm == INVALID_HANDLE_VALUE)
    return true;

  qDebug() << "Port was closed.";

  if (overlappedRead.hEvent != NULL)
    CloseHandle(overlappedRead.hEvent);
  if (overlappedWrite.hEvent != NULL)
    CloseHandle(overlappedWrite.hEvent);
  CloseHandle(hComm);
  opened = false;
  hComm = NULL;

  return true;
}

bool SerialPort::reopen() {
  close();
  return open(portName, overlapped, baudRate, dataBits);
}

int SerialPort::write(const unsigned char *buffer, int size) {
  if (!opened || hComm == NULL || hComm == INVALID_HANDLE_VALUE)
    return 0;

  BOOL bWriteStat;
  DWORD dwBytesWritten;

  if (overlapped) {
    bWriteStat = WriteFile(hComm, buffer, (DWORD)size, &dwBytesWritten,
                           &overlappedWrite);
    if (!bWriteStat && (GetLastError() == ERROR_IO_PENDING)) {
      if (WaitForSingleObject(overlappedWrite.hEvent, 1000))
        dwBytesWritten = 0;
      else {
        GetOverlappedResult(hComm, &overlappedWrite, &dwBytesWritten, FALSE);
        overlappedWrite.Offset += dwBytesWritten;
      }
    }
  } else {
    WriteFile(hComm, buffer, (DWORD)size, &dwBytesWritten, NULL);
  }

  return (int)dwBytesWritten;
}

int SerialPort::read(unsigned char *buffer, int limit) {
  if (!opened || hComm == NULL || hComm == INVALID_HANDLE_VALUE)
    return 0;

  if (overlapped) {
    BOOL bReadStatus;
    DWORD dwBytesRead, dwErrorFlags;
    COMSTAT ComStat;

    DWORD dwEventMask;
    if (!SetCommMask(hComm, EV_RXCHAR))
      return 0;

    if (!WaitCommEvent(hComm, &dwEventMask, &overlappedRead))
      if (GetLastError() != ERROR_IO_PENDING)
        return 0;
    switch (WaitForSingleObject(overlappedRead.hEvent, 1000)) {
    case WAIT_OBJECT_0:
      break;
    default:
      return 0;
    }

    ClearCommError(hComm, &dwErrorFlags, &ComStat);
    if (!ComStat.cbInQue)
      return 0;

    dwBytesRead = (DWORD)ComStat.cbInQue;
    if (limit < (int)dwBytesRead)
      dwBytesRead = (DWORD)limit;
    bReadStatus =
        ReadFile(hComm, buffer, dwBytesRead, &dwBytesRead, &overlappedRead);
    if (!bReadStatus) {
      if (GetLastError() == ERROR_IO_PENDING) {
        WaitForSingleObject(overlappedRead.hEvent, 2000);
        return (int)dwBytesRead;
      }
      return 0;
    }
    return (int)dwBytesRead;
  } else {
    DWORD dwEventMask, dwBytesRead;
    int count{};
    if (!SetCommMask(hComm, EV_RXCHAR))
      return 0;
    if (WaitCommEvent(hComm, &dwEventMask, NULL)) {
      unsigned char szBuf;
      do {
        if (ReadFile(hComm, &szBuf, sizeof(szBuf), &dwBytesRead, NULL) != 0) {
          if (dwBytesRead > 0) {
            buffer[count++] = szBuf;
          }
        } else
          return 0;
      } while (dwBytesRead > 0 && count < limit);
    }
    return (int)count;
  }
}
