#include "winapiprint.h"

BOOL RawDataToPrinter(LPTSTR szPrinterName, LPSTR lpData, DWORD dwCount) {
  BOOL bStatus = FALSE;
  HANDLE hPrinter = NULL;
  DOC_INFO_1 DocInfo;
  DWORD dwJob = 0L;
  DWORD dwBytesWritten = 0L;

  // Open a handle to the printer.
  bStatus = OpenPrinter(szPrinterName, &hPrinter, NULL);
  if (bStatus) {
    // Fill in the structure with info about this "document."
    DocInfo.pDocName = (LPTSTR)("Label");
    DocInfo.pOutputFile = NULL;
    DocInfo.pDatatype = (LPTSTR)("RAW");

    // Inform the spooler the document is beginning.
    dwJob = StartDocPrinter(hPrinter, 1, (LPBYTE)&DocInfo);
    if (dwJob > 0) {
      // Start a page.
      bStatus = StartPagePrinter(hPrinter);
      if (bStatus) {
        // Send the data to the printer.
        bStatus = WritePrinter(hPrinter, lpData, dwCount, &dwBytesWritten);
        EndPagePrinter(hPrinter);
      }
      // Inform the spooler that the document is ending.
      EndDocPrinter(hPrinter);
    }
    // Close the printer handle.
    ClosePrinter(hPrinter);
  }
  // Check to see if correct number of bytes were written.
  if (!bStatus || (dwBytesWritten != dwCount)) {
    bStatus = FALSE;
  } else {
    bStatus = TRUE;
  }
  return bStatus;
}
