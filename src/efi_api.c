#include "efi_api.h"

#include <efi.h>
#include <efilib.h>

// Opaque UEFI context passed to the C++ layer
struct efi_context {
    EFI_SYSTEM_TABLE* system_table;
};

// Prints an ASCII string to the UEFI console
void efi_print(struct efi_context const* context, char const* str) {
    auto const con_out = context->system_table->ConOut;

    constexpr UINTN buffer_size = 256;
    CHAR16 buffer[buffer_size];

    while (*str != 0) {
        UINTN i = 0;

        while (*str != 0 && i < buffer_size - 1)
            buffer[i++] = (CHAR16)(unsigned char)*str++;
        buffer[i] = 0;

        uefi_call_wrapper(con_out->OutputString, 2, con_out, buffer);
    }
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
