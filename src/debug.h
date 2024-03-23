
#include <stdarg.h>

#define CS1(x)                  (0x24000000UL + (x))

static char emu_printf_buffer[256];

void debug_print(const char *message) {
  volatile Uint8 *addr = (volatile Uint8 *)CS1(0x1000);
  const char *s = message;
  while (*s)
    *addr = (Uint8)*s++;

  if((Uint8)*(s-1) != '\n') {
    *addr = '\n';
  }
}
