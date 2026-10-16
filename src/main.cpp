#include "efi_api.h"

// Entry point into the C++ part of the application
void cpp_entry(efi_context* context) {
    efi_print(context, "Hello, world!\r\n");
}
