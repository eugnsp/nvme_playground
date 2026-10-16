#pragma once

#include <efi.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque UEFI context passed to the C++ layer
struct efi_context;

// Prints an ASCII string to the UEFI console
void efi_print(struct efi_context const*, char const*);

// Prints a formatted ASCII string to the UEFI console
void efi_printf(struct efi_context const*, char const* format, ...);

///////////////////////////////////////////////////////////////////////////////////////////////////

// Entry point into the C++ part of the application
void cpp_entry(struct efi_context*);

#ifdef __cplusplus
}
#endif
