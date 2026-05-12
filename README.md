# MFRTX Driver Suite

MFRTX is a local driver solution designed to enable Metal API support on NVIDIA hardware for both Windows and macOS (Hackintosh) environments. This suite provides the necessary kernel-level binaries to facilitate Metal shader execution without requiring an active internet connection.

## Hardware Support
* **NVIDIA GTX Series:** ![Alternatif Metin](https://img.shields.io/badge/ALL_MODELS_RELEASED_AFTER_2018-blue?style=for-the-badge&logo=nvidia).
* **NVIDIA RTX 20-Series:** ![Alternatif Metin](https://img.shields.io/badge/ALL_MODELS-blue?style=for-the-badge&logo=nvidia)
* **NVIDIA RTX 30-Series:** ![Alternatif Metin](https://img.shields.io/badge/ALL_MODELS-blue?style=for-the-badge&logo=nvidia) 
* **NVIDIA RTX 40-Series:** ![Alternatif Metin](https://img.shields.io/badge/Up_to_RTX_4090-blue?style=for-the-badge&logo=nvidia).
* **UNSUPPORTED:** NVIDIA RTX 40 series Super/Super TI/TI graphics cards are not supported due to architecture-specific hardware locks.

## Components
1. **MFRTX_Setup.exe:** Standalone Windows driver installer. Contains pre-compiled shader libraries and kernel binaries.
2. **mfrtx.kext:** macOS Kernel Extension for Hackintosh environments. Required for Metal framework recognition.

## Installation Process

### Windows Deployment
1. Run `MFRTX_Setup.exe` with Administrator privileges.
2. The installer will perform a local hardware ID (HWID) scan.
3. Metal-to-NVIDIA microcode will be extracted to the system drivers directory.
4. The setup will register the MFRTX service to initialize upon every system boot.

### macOS Configuration
1. Copy `mfrtx.kext` to your EFI partition (`/EFI/OC/Kexts`).
2. Add the kext entry to your `config.plist` under the Kernel section.
3. Ensure `DisableIoMapper` is configured correctly to avoid conflicts with the MFRTX memory mapping.
4. **IMPORTANT:** You must create a folder named `MFRTX` (case sensitive) in the root of your `/EFI/` directory.
