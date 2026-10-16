# NVMe playground

:construction:

A small UEFI C++ playground for exploring PCIe and NVMe at a low level. The project uses GNU-EFI and runs under QEMU + OVMF.

## Dependencies

```bash
sudo apt install gnu-efi qemu-system-x86 ovmf
```

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Run

```bash
cmake --build . --target run
```
