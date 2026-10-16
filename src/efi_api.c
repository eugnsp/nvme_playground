#include "efi_api.h"

#include <efi.h>
#include <efilib.h>

// Opaque UEFI context passed to the C++ layer
struct efi_context {
    EFI_SYSTEM_TABLE* system_table;
};

// Prints an ASCII string to the UEFI console
void efi_print(struct efi_context const* context, char const* str) {
    constexpr UINTN buffer_size = 256;
    CHAR16 buffer[buffer_size];

    UINTN i = 0;
    while (*str != 0 && i < buffer_size - 1)
        buffer[i++] = (CHAR16)(unsigned char)*str++;
    buffer[i] = 0;

    auto const con_out = context->system_table->ConOut;
    uefi_call_wrapper(con_out->OutputString, 2, con_out, buffer);
}

// Prints a formatted ASCII string to the UEFI console
void efi_printf(struct efi_context const* context, char const* format, ...) {
    constexpr UINTN buffer_size = 256;
    CHAR8 buffer[buffer_size];

    va_list args;
    va_start(args, format);
    AsciiVSPrint(buffer, sizeof(buffer), (CHAR8 const*)format, args);
    va_end(args);

    efi_print(context, (char const*)buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// UEFI application entry point
EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    InitializeLib(image_handle, system_table);

    struct efi_context context = {
        .system_table = system_table
    };

    cpp_entry(&context);

    UINTN index;
    uefi_call_wrapper(system_table->BootServices->WaitForEvent, 3, 1, &system_table->ConIn->WaitForKey, &index);

    return EFI_SUCCESS;
}

#include <stddef.h>

void* memmove(void* dst, void const* src, size_t size)
{
    unsigned char* d = dst;
    unsigned char const* s = src;

    if (d < s) {
        for (size_t i = 0; i < size; ++i)
            d[i] = s[i];
    } else if (d > s) {
        for (size_t i = size; i != 0; --i)
            d[i - 1] = s[i - 1];
    }

    return dst;
}

int memcmp(void const* lhs, void const* rhs, size_t size)
{
    unsigned char const* a = lhs;
    unsigned char const* b = rhs;

    for (size_t i = 0; i < size; ++i) {
        if (a[i] < b[i])
            return -1;

        if (a[i] > b[i])
            return 1;
    }

    return 0;
}
