/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int main() {
  int bytesread = 1000;

  Printint(bytesread);

  Write("who\n", 4, ConsoleOutput);
  Exit(0);
  return 0;
}
