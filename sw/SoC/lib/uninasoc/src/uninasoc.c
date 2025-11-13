#include "uninasoc.h"

void uninasoc_init()
{
    extern const volatile uintptr_t _peripheral_UART_start;
    // TinyIO init
    uintptr_t uart_base_address = (uintptr_t)&_peripheral_UART_start;
    tinyIO_init(uart_base_address);
}
