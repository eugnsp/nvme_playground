cmake_minimum_required(VERSION 3.28)

project(nvme_playground LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_path(GNU_EFI_INCLUDE_DIR
    NAMES efi.h
    PATHS /usr/include/efi
    REQUIRED
)

find_path(GNU_EFI_ARCH_INCLUDE_DIR
    NAMES efibind.h
    PATHS /usr/include/efi/x86_64
    REQUIRED
)

find_file(GNU_EFI_CRT0
    NAMES crt0-efi-x86_64.o
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu
    REQUIRED
)

find_file(GNU_EFI_LINKER_SCRIPT
    NAMES elf_x86_64_efi.lds
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu
    REQUIRED
)

find_library(GNU_EFI_GNUEFI_LIBRARY
    NAMES gnuefi
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu
    REQUIRED
)

find_library(GNU_EFI_EFI_LIBRARY
    NAMES efi
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu
    REQUIRED
)

find_program(OBJCOPY
    NAMES objcopy
    REQUIRED
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(objects OBJECT ${SOURCES})

target_include_directories(objects PRIVATE
    ${GNU_EFI_INCLUDE_DIR}
    ${GNU_EFI_ARCH_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(objects PRIVATE
    EFI_FUNCTION_WRAPPER
    $<$<COMPILE_LANGUAGE:CXX>:_GLIBCXX_NO_ASSERTIONS>
)

target_compile_options(objects PRIVATE
    -ffreestanding
    -fno-stack-protector
    -fpic
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-gnu-unique>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
    -mno-red-zone
    -Wall
    -Wextra
)

set(ELF_FILE "${CMAKE_CURRENT_BINARY_DIR}/nvme_playground.so")
set(EFI_FILE "${CMAKE_CURRENT_BINARY_DIR}/nvme_playground.efi")

add_custom_command(
    OUTPUT ${ELF_FILE}

    COMMAND ${CMAKE_LINKER}
        -nostdlib
        -znocombreloc
        --no-undefined
        -T ${GNU_EFI_LINKER_SCRIPT}
        ${GNU_EFI_CRT0}
        $<TARGET_OBJECTS:objects>
        -shared
        -Bsymbolic
        ${GNU_EFI_GNUEFI_LIBRARY}
        ${GNU_EFI_EFI_LIBRARY}
        -o ${ELF_FILE}

    DEPENDS $<TARGET_OBJECTS:objects>

    COMMAND_EXPAND_LISTS
    VERBATIM
)

add_custom_command(
    OUTPUT ${EFI_FILE}

    COMMAND ${OBJCOPY}
        -j .data
        -j .dynamic
        -j .dynsym
        -j .rel
        -j .rela
        -j .reloc
        -j .rodata
        -j .sdata
        -j .text
        --target=efi-app-x86_64
        ${ELF_FILE}
        ${EFI_FILE}

    DEPENDS ${ELF_FILE}

    VERBATIM
)

add_custom_target(nvme_playground ALL
    DEPENDS ${EFI_FILE}
)

set(ESP_DIR "${CMAKE_CURRENT_BINARY_DIR}/esp")
set(BOOT_DIR "${ESP_DIR}/EFI/BOOT")
set(OVMF_VARS "${CMAKE_CURRENT_BINARY_DIR}/OVMF_VARS.fd")
set(NVME_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/nvme.img")

find_program(QEMU_SYSTEM_X86_64
    NAMES qemu-system-x86_64
    REQUIRED
)

find_program(QEMU_IMG
    NAMES qemu-img
    REQUIRED
)

find_file(OVMF_CODE
    NAMES OVMF_CODE_4M.fd
    PATHS /usr/share/OVMF
    REQUIRED
)

find_file(OVMF_VARS_TEMPLATE
    NAMES OVMF_VARS_4M.fd
    PATHS /usr/share/OVMF
    REQUIRED
)

add_custom_command(
    OUTPUT ${NVME_IMAGE}
    COMMAND ${QEMU_IMG} create -f raw ${NVME_IMAGE} 64M
    VERBATIM
)

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BOOT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy
        ${EFI_FILE}
        ${BOOT_DIR}/BOOTX64.EFI

    COMMAND ${CMAKE_COMMAND} -E copy
        ${OVMF_VARS_TEMPLATE}
        ${OVMF_VARS}

    COMMAND ${QEMU_SYSTEM_X86_64}
        -machine q35
        -enable-kvm
        -cpu host
        -m 256M
        -drive if=pflash,format=raw,readonly=on,file=${OVMF_CODE}
        -drive if=pflash,format=raw,file=${OVMF_VARS}
        -drive format=raw,file=fat:rw:${ESP_DIR}
        -drive file=${NVME_IMAGE},if=none,id=nvme0,format=raw
        -device nvme,drive=nvme0,serial=NVME0001

    DEPENDS
        nvme_playground
        ${NVME_IMAGE}

    USES_TERMINAL
    VERBATIM
)
