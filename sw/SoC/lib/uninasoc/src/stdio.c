#include "stdio.h"
#include "stdarg.h"

#include "print.h"
#include "scan.h"


int printf(const char *format, ...) {
  va_list va;
  int ret;
  va_start(va, format);

  // Call to the TinyIO printf
  ret = c_printf(format, va);

  va_end(va);
  return ret;
}

int scanf(char *format, ...) {
  va_list va;
  int ret;
  va_start(va, format);

  // Call to the TinyIO printf
  ret = c_scanf(format, va);

  va_end(va);
  return ret;
}
