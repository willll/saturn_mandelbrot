#define CS1(x)                  (0x24000000UL + (x))

void print_debug(const char *message) {

  volatile Uint8 *addr = (volatile Uint8 *)CS1(0x1000);
  static char emu_printf_buffer[256];
  char *s = emu_printf_buffer;
  (void)snprintf(emu_printf_buffer, 256, "%s\n", message);
  while (*s)
    *addr = (Uint8)*s++;
}
