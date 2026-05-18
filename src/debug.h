#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>

/**
 * @file debug.h
 * @brief Emulator-side trace output helpers.
 */

/**
 * @brief Cartridge/CS1 base address helper.
 * @param x Offset from CS1 base.
 */
#define CS1(x)                  (0x24000000UL + (x))

/**
 * @brief Writes a message to the emulator debug port.
 * @param message Null-terminated text to emit.
 */
static void debug_print(const char *message) {
  volatile Uint8 *addr = (volatile Uint8 *)CS1(0x1000);
  const char *s = message;
  while (*s)
    *addr = (Uint8)*s++;

  if((Uint8)*(s-1) != '\n') {
    *addr = '\n';
  }
}

#endif
