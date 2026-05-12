# MFRTX System Integration Utility

MFRTX (Metal for RTX) is a specialized deployment framework designed to bridge the gap between NVIDIA's Ampere/Ada Lovelace architectures and the Apple Metal® Graphics API. This utility facilitates the installation of the MFRTX micro-kernel, enabling hardware-level acceleration and native shader translation on Windows and macOS environments.

## Features

* **Metal-to-NVIDIA Bridge:** Implements a low-overhead translation layer for direct execution of Metal shaders on RTX silicon.
* **Kernel-Level Integration:** Deep system integration to ensure maximum throughput and minimum latency during draw calls.
* **Hackintosh Deployment Suite:** Automated redirection and asset preparation for OpenCore-based macOS installations.
* **Unified Shader Cache:** Shared shader pre-caching to reduce stuttering in cross-platform rendering workloads.

## Installation Paths

### 1. Windows Native Extension
The Windows deployment path installs a system service that emulates the Metal framework within the Win32 subsystem. This allows developers to test Metal-based applications without leaving the Windows environment. 
*Note: Requires an active connection to the MFRTX Cloud for asset validation.*

### 2. macOS Bootloader Configuration
Designed for users converting their PC into a Hackintosh. This utility serves as a gateway to the official OpenCore deployment documentation and provides necessary kernel extension (Kext) templates for supported hardware.

## System Requirements

| Component | Minimum Requirement | Recommended |
| :--- | :--- | :--- |
| GPU | NVIDIA RTX 20-series | NVIDIA RTX 30/40-series |
| CPU | Intel 10th Gen / AMD Ryzen 3000 | Intel 12th Gen+ / Ryzen 7000 |
| OS | Windows 10/11 (Build 19044+) | Windows 11 (23H2+) |
| Network | 10 Mbps for Handshake | 100 Mbps+ for Asset Sync |

## Troubleshooting

### Connection Errors (0x8004210B)
If you encounter the "Can't connect to MFRTX servers" message, verify the following:
1. Ensure the MFRTX-Service-Daemon is not blocked by your local firewall.
2. Check if your hardware ID (HWID) is blacklisted on the global cluster.
3. Validate that the MFRTX deployment infrastructure is online in your region.

## Legal Disclaimer

Metal is a registered trademark of Apple Inc. NVIDIA and RTX are trademarks of NVIDIA Corporation. This project is an independent implementation and is not affiliated with, endorsed by, or sponsored by Apple Inc. or NVIDIA Corporation. Use at your own risk.
