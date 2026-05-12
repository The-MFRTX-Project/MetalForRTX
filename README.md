# MFRTX

MFRTX is a local driver solution designed to enable Metal API support on NVIDIA hardware for both Windows and macOS (Hackintosh) environments. This suite provides the necessary kernel-level binaries to facilitate Metal shader execution without requiring an active internet connection.

## Hardware Support
* **NVIDIA GTX Series:** ![Alternatif Metin](https://img.shields.io/badge/ALL_MODELS_RELEASED_AFTER_2018-orange?style=for-the-badge&logo=nvidia).
* **NVIDIA RTX 20-Series:** ![Alternatif Metin](https://img.shields.io/badge/ALL_MODELS-green?style=for-the-badge&logo=nvidia)
* **NVIDIA RTX 30-Series:** ![Alternatif Metin](https://img.shields.io/badge/ALL_MODELS-green?style=for-the-badge&logo=nvidia) 
* **NVIDIA RTX 40-Series:** ![Alternatif Metin](https://img.shields.io/badge/Up_to_RTX_4090-orange?style=for-the-badge&logo=nvidia).
* **NVIDIA RTX 50-Series:** ![Alternatif Metin](https://img.shields.io/badge/NO_SUPPORT_(for_now)-red?style=for-the-badge&logo=nvidia)
* **UNSUPPORTED:** NVIDIA RTX 40 series SUPER/SUPER Ti/Ti graphics cards are not supported due to architecture-specific hardware locks.

## Components
1. **MFRTX_Setup.exe:** Standalone Windows driver installer. Contains pre-compiled shader libraries and kernel binaries.
2. **mfrtx.kext:** macOS Kernel Extension for Hackintosh environments. Required for Metal framework recognition.

## Installation Process

### Windows Deployment
1. Run `MFRTX_Setup.exe`.
2. Follow the steps on the screen.

### macOS Configuration
1. Copy `mfrtx.kext` to your EFI partition (`/EFI/OC/Kexts`).
2. Add the kext entry to your `config.plist` under the Kernel section.
3. Ensure `DisableIoMapper` is configured correctly to avoid conflicts with the MFRTX memory mapping.
4. **IMPORTANT:** You must create a folder named `MFRTX` (case sensitive) in the root of your `/EFI/` directory.
