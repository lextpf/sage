<div align="center">

# sage
**AES-256-GCM with Qt6 GUI providing on-demand credential management**

🛡️ [Features](#features) | 🚅 [Quick Start](#quick-start) | 📗 [Documentation](#documentation) | 🤝 [Contributing](./CONTRIBUTING.md)

![AES-256-GCM](https://img.shields.io/badge/AES--256--GCM-Encryption-06B6D4.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTY4cHgiIGhlaWdodD0iMTY4cHgiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiBzdHJva2U9IiNmZmZmZmYiPjxnIGlkPSJTVkdSZXBvX2JnQ2FycmllciIgc3Ryb2tlLXdpZHRoPSIwIj48L2c+PGcgaWQ9IlNWR1JlcG9fdHJhY2VyQ2FycmllciIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj48L2c+PGcgaWQ9IlNWR1JlcG9faWNvbkNhcnJpZXIiPiA8cGF0aCBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGNsaXAtcnVsZT0iZXZlbm9kZCIgZD0iTTUuNSAxMFY3QzUuNSA1LjI3NjA5IDYuMTg0ODIgMy42MjI3OSA3LjQwMzgxIDIuNDAzODFDOC42MjI3OSAxLjE4NDgyIDEwLjI3NjEgMC41IDEyIDAuNUMxMy43MjM5IDAuNSAxNS4zNzcyIDEuMTg0ODIgMTYuNTk2MiAyLjQwMzgxQzE3LjgxNTIgMy42MjI3OSAxOC41IDUuMjc2MDkgMTguNSA3VjEwSDE5QzIwLjY1NjkgMTAgMjIgMTEuMzQzMSAyMiAxM1YyMEMyMiAyMS42NTY5IDIwLjY1NjkgMjMgMTkgMjNINUMzLjM0MzE1IDIzIDIgMjEuNjU2OSAyIDIwVjEzQzIgMTEuMzQzMSAzLjM0MzE1IDEwIDUgMTBINS41Wk05LjUyNTEzIDQuNTI1MTNDMTAuMTgxNSAzLjg2ODc1IDExLjA3MTcgMy41IDEyIDMuNUMxMi45MjgzIDMuNSAxMy44MTg1IDMuODY4NzUgMTQuNDc0OSA0LjUyNTEzQzE1LjEzMTMgNS4xODE1IDE1LjUgNi4wNzE3NCAxNS41IDdWMTBIOC41VjdDOC41IDYuMDcxNzQgOC44Njg3NSA1LjE4MTUgOS41MjUxMyA0LjUyNTEzWiIgZmlsbD0iI2ZmZmZmZiI+PC9wYXRoPiA8L2c+PC9zdmc+)
![OCR](https://img.shields.io/badge/Webcam-OCR-3B82F6.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB2ZXJzaW9uPSIxLjEiIGlkPSJJY29ucyIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgdmlld0JveD0iMCAwIDMyIDMyIiB4bWw6c3BhY2U9InByZXNlcnZlIiB3aWR0aD0iMTY4cHgiIGhlaWdodD0iMTY4cHgiIGZpbGw9IiNmZmZmZmYiPjxnIGlkPSJTVkdSZXBvX2JnQ2FycmllciIgc3Ryb2tlLXdpZHRoPSIwIj48L2c+PGcgaWQ9IlNWR1JlcG9fdHJhY2VyQ2FycmllciIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj48L2c+PGcgaWQ9IlNWR1JlcG9faWNvbkNhcnJpZXIiPiA8c3R5bGUgdHlwZT0idGV4dC9jc3MiPiAuc3Qwe2ZpbGw6bm9uZTtzdHJva2U6I2ZmZmZmZjtzdHJva2Utd2lkdGg6MjtzdHJva2UtbGluZWNhcDpyb3VuZDtzdHJva2UtbGluZWpvaW46cm91bmQ7c3Ryb2tlLW1pdGVybGltaXQ6MTA7fSA8L3N0eWxlPiA8Zz4gPGNpcmNsZSBjeD0iMTYiIGN5PSIxNCIgcj0iMiI+PC9jaXJjbGU+IDxwYXRoIGQ9Ik0yMy40LDIzLjRjMi44LTIuMiw0LjYtNS42LDQuNi05LjRjMC02LjYtNS40LTEyLTEyLTEyUzQsNy40LDQsMTRjMCwzLjgsMS44LDcuMiw0LjYsOS40bC0xLjUsMS4zIGMtMC45LDAuOC0xLjIsMi4xLTAuOCwzLjNDNi44LDI5LjIsNy45LDMwLDkuMiwzMGgxMy43YzEuMiwwLDIuMy0wLjgsMi44LTEuOWMwLjUtMS4yLDAuMS0yLjUtMC44LTMuM0wyMy40LDIzLjR6IE0xNiwyMCBjLTMuMywwLTYtMi43LTYtNnMyLjctNiw2LTZzNiwyLjcsNiw2UzE5LjMsMjAsMTYsMjB6IE0yMi43LDkuN2MwLDAtMC4xLDAuMS0wLjIsMC4xYzAsMC0wLjEsMC4xLTAuMiwwLjFjLTAuMSwwLTAuMSwwLTAuMiwwLjEgYy0wLjEsMC0wLjEsMC0wLjIsMGMtMC4xLDAtMC4xLDAtMC4yLDBjLTAuMSwwLTAuMSwwLTAuMi0wLjFjLTAuMSwwLTAuMS0wLjEtMC4yLTAuMWMwLDAtMC4xLTAuMS0wLjEtMC4xYzAtMC4xLTAuMS0wLjEtMC4xLTAuMSBjMC0wLjEtMC4xLTAuMS0wLjEtMC4yYzAtMC4xLDAtMC4xLTAuMS0wLjJjMC0wLjEsMC0wLjEsMC0wLjJjMC0wLjEsMC0wLjEsMC0wLjJjMC0wLjEsMC0wLjEsMC4xLTAuMmMwLTAuMSwwLTAuMSwwLjEtMC4yIGMwLTAuMSwwLjEtMC4xLDAuMS0wLjFjMCwwLDAuMS0wLjEsMC4xLTAuMWMwLjEsMCwwLjEtMC4xLDAuMi0wLjFjMC4xLDAsMC4xLDAsMC4yLTAuMWMwLjMtMC4xLDAuNywwLDAuOSwwLjMgQzIyLjksOC41LDIzLDguNywyMyw5UzIyLjksOS41LDIyLjcsOS43eiI+PC9wYXRoPiA8L2c+IDwvZz48L3N2Zz4=)
![No Telemetry](https://img.shields.io/badge/Telemetry-None-10B981.svg?style=flat&logo=data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTY4cHgiIGhlaWdodD0iMTY4cHgiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48ZyBpZD0iU1ZHUmVwb19iZ0NhcnJpZXIiIHN0cm9rZS13aWR0aD0iMCI+PC9nPjxnIGlkPSJTVkdSZXBvX3RyYWNlckNhcnJpZXIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCI+PC9nPjxnIGlkPSJTVkdSZXBvX2ljb25DYXJyaWVyIj4gPGcgY2xpcC1wYXRoPSJ1cmwoI2NsaXAwXzQyOV8xMTE1OCkiPiA8cGF0aCBkPSJNMTAuNzMwMiA1LjA3MzE5QzExLjE0NDggNS4wMjQ4NSAxMS41Njg0IDUgMTEuOTk5OSA1QzE2LjY2MzkgNSAyMC4zOTk4IDcuOTAyNjQgMjEuOTk5OSAxMkMyMS42MDUzIDEzLjAxMDQgMjEuMDgwOSAxMy45NDgyIDIwLjQ0NDYgMTQuNzg3N002LjUxOTU2IDYuNTE5NDRDNC40Nzk0OSA3Ljc2NDA2IDIuOTAxMDUgOS42OTI1OSAxLjk5OTk0IDEyQzMuNjAwMDggMTYuMDk3NCA3LjMzNTk3IDE5IDExLjk5OTkgMTlDMTQuMDM3NSAxOSAxNS44OTc5IDE4LjQ0NiAxNy40ODA1IDE3LjQ4MDRNOS44Nzg3MSA5Ljg3ODU5QzkuMzM1NzYgMTAuNDIxNSA4Ljk5OTk0IDExLjE3MTUgOC45OTk5NCAxMkM4Ljk5OTk0IDEzLjY1NjkgMTAuMzQzMSAxNSAxMS45OTk5IDE1QzEyLjgyODQgMTUgMTMuNTc4NSAxNC42NjQyIDE0LjEyMTQgMTQuMTIxMiIgc3Ryb2tlPSIjZmZmZmZmIiBzdHJva2Utd2lkdGg9IjIuNSIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj48L3BhdGg+IDxwYXRoIGQ9Ik00IDRMMjAgMjAiIHN0cm9rZT0iI2ZmZmZmZiIgc3Ryb2tlLXdpZHRoPSIyLjUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCI+PC9wYXRoPiA8L2c+IDxkZWZzPiA8Y2xpcFBhdGggaWQ9ImNsaXAwXzQyOV8xMTE1OCI+IDxyZWN0IHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0id2hpdGUiPjwvcmVjdD4gPC9jbGlwUGF0aD4gPC9kZWZzPiA8L2c+PC9zdmc+)
![Qt6](https://img.shields.io/badge/Qt6-GUI-8B5CF6.svg?style=flat&logo=qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.10%2B-c0392b?style=flat&logo=cmake&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-475569.svg?logo=data:image/svg+xml;base64,PHN2ZyBmaWxsPSIjZmZmZmZmIiB3aWR0aD0iMTY0cHgiIGhlaWdodD0iMTY0cHgiIHZpZXdCb3g9IjAgMCA1MTIuMDAgNTEyLjAwIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHN0cm9rZT0iI2ZmZmZmZiIgc3Ryb2tlLXdpZHRoPSIwLjAwNTEyIj48ZyBpZD0iU1ZHUmVwb19iZ0NhcnJpZXIiIHN0cm9rZS13aWR0aD0iMCI+PC9nPjxnIGlkPSJTVkdSZXBvX3RyYWNlckNhcnJpZXIiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlPSIjQ0NDQ0NDIiBzdHJva2Utd2lkdGg9IjMuMDcyIj48L2c+PGcgaWQ9IlNWR1JlcG9faWNvbkNhcnJpZXIiPjxwYXRoIGQ9Ik0yNTYgOEMxMTkuMDMzIDggOCAxMTkuMDMzIDggMjU2czExMS4wMzMgMjQ4IDI0OCAyNDggMjQ4LTExMS4wMzMgMjQ4LTI0OFMzOTIuOTY3IDggMjU2IDh6bTExNy4xMzQgMzQ2Ljc1M2MtMS41OTIgMS44NjctMzkuNzc2IDQ1LjczMS0xMDkuODUxIDQ1LjczMS04NC42OTIgMC0xNDQuNDg0LTYzLjI2LTE0NC40ODQtMTQ1LjU2NyAwLTgxLjMwMyA2Mi4wMDQtMTQzLjQwMSAxNDMuNzYyLTE0My40MDEgNjYuOTU3IDAgMTAxLjk2NSAzNy4zMTUgMTAzLjQyMiAzOC45MDRhMTIgMTIgMCAwIDEgMS4yMzggMTQuNjIzbC0yMi4zOCAzNC42NTVjLTQuMDQ5IDYuMjY3LTEyLjc3NCA3LjM1MS0xOC4yMzQgMi4yOTUtLjIzMy0uMjE0LTI2LjUyOS0yMy44OC02MS44OC0yMy44OC00Ni4xMTYgMC03My45MTYgMzMuNTc1LTczLjkxNiA3Ni4wODIgMCAzOS42MDIgMjUuNTE0IDc5LjY5MiA3NC4yNzcgNzkuNjkyIDM4LjY5NyAwIDY1LjI4LTI4LjMzOCA2NS41NDQtMjguNjI1IDUuMTMyLTUuNTY1IDE0LjA1OS01LjAzMyAxOC41MDggMS4wNTNsMjQuNTQ3IDMzLjU3MmExMi4wMDEgMTIuMDAxIDAgMCAxLS41NTMgMTQuODY2eiI+PC9wYXRoPjwvZz48L3N2Zz4=)
<br/>
<br/>
<br/>
![Sponsor](https://img.shields.io/static/v1?label=sponsor&message=%E2%9D%A4&color=ff69b4&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA2NDAgNjQwIj48IS0tIUZvbnQgQXdlc29tZSBQcm8gdjcuMi4wIGJ5IEBmb250YXdlc29tZSAtIGh0dHBzOi8vZm9udGF3ZXNvbWUuY29tIExpY2Vuc2UgLSBodHRwczovL2ZvbnRhd2Vzb21lLmNvbS9saWNlbnNlIChDb21tZXJjaWFsIExpY2Vuc2UpIENvcHlyaWdodCAyMDI2IEZvbnRpY29ucywgSW5jLi0tPjxwYXRoIG9wYWNpdHk9IjEiIGZpbGw9IiNmZjY5YjRmZiIgZD0iTTMyIDQ4MEwzMiA1NDRDMzIgNTYxLjcgNDYuMyA1NzYgNjQgNTc2TDM4NC41IDU3NkM0MTMuNSA1NzYgNDQxLjggNTY2LjcgNDY1LjIgNTQ5LjVMNTkxLjggNDU2LjJDNjA5LjYgNDQzLjEgNjEzLjQgNDE4LjEgNjAwLjMgNDAwLjNDNTg3LjIgMzgyLjUgNTYyLjIgMzc4LjcgNTQ0LjQgMzkxLjhMNDI0LjYgNDgwTDMxMiA0ODBDMjk4LjcgNDgwIDI4OCA0NjkuMyAyODggNDU2QzI4OCA0NDIuNyAyOTguNyA0MzIgMzEyIDQzMkwzODQgNDMyQzQwMS43IDQzMiA0MTYgNDE3LjcgNDE2IDQwMEM0MTYgMzgyLjMgNDAxLjcgMzY4IDM4NCAzNjhMMjMxLjggMzY4QzE5Ny45IDM2OCAxNjUuMyAzODEuNSAxNDEuMyA0MDUuNUw5OC43IDQ0OEw2NCA0NDhDNDYuMyA0NDggMzIgNDYyLjMgMzIgNDgweiIvPjxwYXRoIGZpbGw9InJnYmEoMjU1LCAyNTUsIDI1NSwgMS4wMCkiIGQ9Ik0yNTAuOSA2NEMyNzQuOSA2NCAyOTcuNSA3NS41IDMxMS42IDk1TDMyMCAxMDYuN0wzMjguNCA5NUMzNDIuNSA3NS41IDM2NS4xIDY0IDM4OS4xIDY0QzQzMC41IDY0IDQ2NCA5Ny41IDQ2NCAxMzguOUw0NjQgMTQxLjNDNDY0IDIwNS43IDM4MiAyNzQuNyAzNDEuOCAzMDQuNkMzMjguOCAzMTQuMyAzMTEuMyAzMTQuMyAyOTguMyAzMDQuNkMyNTguMSAyNzQuNiAxNzYgMjA1LjcgMTc2LjEgMTQxLjNMMTc2LjEgMTM4LjlDMTc2IDk3LjUgMjA5LjUgNjQgMjUwLjkgNjR6Ii8+PC9zdmc+)
</div>

**An encryption and credential manager** built with **C++23** and powered by *OpenSSL* **AES-256-GCM**.It pairs a *Qt6* interface, where credentials are stored **encrypted** and only *decrypted on demand*, with a *streaming CLI* that slots into **pipes** and **scripts**. sage can encrypt *individual files*, *entire directories*, or *inline text*, and fills credentials directly into applications via its *auto-fill controller*. Under the hood, every secret sits in **hardened memory** - *locked pages*, *guard pages*, *canary sentinels* and the clipboard is **scrubbed automatically** if used.

<div align="center">
<br>

<img src="PREVIEW.png" alt="Preview" width="525"/>

</div>

> [!IMPORTANT]
> **Windows 10/11 Only** - sage enforces Windows-specific security mitigations and will not build on other platforms.
>
> - For best protection, enable `SeLockMemoryPrivilege` via Group Policy (`gpedit.msc` -> Local Policies -> User Rights Assignment -> Lock pages in memory).

```
/*  ============================================================================================  *
 *                                                            ⠀⣠⡤⠀⢀⣀⣀⡀⠀⠀⠀⠀⣦⡀⠀⠀⠀⠀⠀⠀
 *                                                            ⠀⠘⠃⠈⢿⡏⠉⠉⠀⢀⣀⣰⣿⣿⡄⠀⠀⠀⠀⢀
 *           ::::::::      :::      ::::::::  ::::::::::      ⠀⠀⠀⠀⠀⢹⠀⠀⠀⣸⣿⡿⠉⠿⣿⡆⠀⠰⠿⣿
 *          :+:    :+:   :+: :+:   :+:    :+: :+:             ⠀⠀⠀⠀⠀⢀⣠⠾⠿⠿⠿⠀⢰⣄⠘⢿⠀⠀⠀⠞
 *          +:+         +:+   +:+  +:+        +:+             ⢲⣶⣶⡂⠐⢉⣀⣤⣶⣶⡦⠀⠈⣿⣦⠈⠀⣾⡆⠀
 *          +#++:++#++ +#++:++#++: :#:        +#++:++#        ⠀⠀⠿⣿⡇⠀⠀⠀⠙⢿⣧⠀⠳⣿⣿⡀⠸⣿⣿⠀
 *                 +#+ +#+     +#+ +#+   +#+# +#+             ⠀⠀⠐⡟⠁⠀⠀⢀⣴⣿⠛⠓⠀⣉⣿⣿⢠⡈⢻⡇
 *          #+#    #+# #+#     #+# #+#    #+# #+#             ⠀⠀⠀⠀⠀⠀⠀⣾⣿⣿⣆⠀⢹⣿⣿⣷⡀⠁⢸⡇
 *           ########  ###     ###  ########  ##########      ⠀⠀⠀⠀⠀⠀⠘⠛⠛⠉⠀⠀⠈⠙⠛⠿⢿⣶⣼⠃
 *                                                            ⠀⠀⠀⢰⣧⣤⠤⠖⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
 *
 *                                  << P A S S   M A N A G E R >>
 *
 *  ============================================================================================  */
```

## Features

### Interface

sage ships with a **Qt6 QML** graphical manager and a **streaming CLI** for scripts and automation.

```mermaid
---
config:
  look: handDrawn
  theme: mc
  themeVariables:
    fontSize: 18px
  layout: elk
---
graph LR
    classDef gui fill:#134e3a,stroke:#10b981,color:#e2e8f0
    classDef cli fill:#1e3a5f,stroke:#3b82f6,color:#e2e8f0
    classDef core fill:#2e1f5e,stroke:#8b5cf6,color:#e2e8f0

    subgraph GUI["🖥️ Qt6 GUI"]
        Vault["🔐 Vault Manager"]:::gui
        AutoFill["⌨️ Auto-Fill"]:::gui
        OCR["📷 Webcam OCR"]:::gui
    end

    subgraph CLI["⚡ Command Line"]
        Stream["📡 Stream Mode"]:::cli
        Interactive["💬 Interactive"]:::cli
        Import["📥 Batch Import"]:::cli
    end

    subgraph Core["🛡️ Crypto Engine"]
        AES["🔑 AES-256-GCM"]:::core
        Memory["🧱 Hardened Memory"]:::core
        Clipboard["📋 Clipboard Scrub"]:::core
    end

    GUI --> Core
    CLI --> Core
```

### Memory Protection

```mermaid
---
config:
  look: handDrawn
  theme: mc
  themeVariables:
    fontSize: 18px
  layout: elk
---
graph LR
    classDef lock fill:#7c2d12,stroke:#f97316,color:#fef3c7
    classDef guard fill:#4c1d95,stroke:#e879f9,color:#e2e8f0
    classDef wipe fill:#064e3b,stroke:#34d399,color:#e2e8f0
    classDef canary fill:#713f12,stroke:#facc15,color:#fef9c3

    V["🔒 VirtualLock"]:::lock
    G["🚧 Guard Pages"]:::guard
    S["🧹 SecureZeroMemory"]:::wipe
    C["🐤 0xD0 Canary"]:::canary

    V --- G --- S --- C
```

- 🔒 **VirtualLock** — Locks sensitive pages to prevent swapping to disk
- 🚧 **Guard Pages** — `PAGE_NOACCESS` regions flanking every allocation detect overflows
- 🧹 **Secure Wiping** — `SecureZeroMemory` scrubs data before deallocation
- 🐤 **Canary Protection** — 0xD0 sentinel at the tail of every buffer catches overruns
- 📋 **Clipboard Scrubbing** — Auto-wipes copied credentials after a configurable TTL

### Process Hardening

sage enables Windows security mitigations at startup:

- Prohibits dynamic code generation
- Requires signed images for loading
- Enables strict handle checks
- Restricts image loading policies
- Detects and blocks Remote Desktop sessions

### Vault System

Credentials are stored in `.sage` files using **on-demand decryption** - each record stays encrypted until the moment you need it.

|  Component | Description                                         |
|------------|-----------------------------------------------------|
|  Platform  | AES-256-GCM encrypted, displayed as search index    |
| Credential | `username:password` blob, decrypted only on access  |

### Additional Capabilities

- 📷 **Webcam OCR** - Read master password from screen captures via the in-tree `tess` OCR engine
- ⌨️ **Auto-Fill** - Type credentials directly into target application fields
- 📁 **Batch Processing** - Encrypt/decrypt entire directories recursively
- 📡 **Stream Mode** - Pipe-friendly stdin/stdout encryption for scripting

## Technology Stack

| Component       | Technology              |
|-----------------|-------------------------|
| Language        | C++20                   |
| GUI Framework   | Qt6 (QML + Quick)       |
| Cryptography    | OpenSSL 3.x             |
| Build System    | CMake 3.20+             |
| Package Manager | vcpkg                   |
| Testing         | Google Test             |
| Documentation   | Doxide + MkDocs         |
| OCR Engine      | tess (in-tree)          |
| Platform        | Windows 10/11 (64-bit)  |

## Quick Start

### Prerequisites

- **Windows 10/11** (64-bit)
- **MSVC 2019+** (C++20 support)
- **vcpkg** with the toolchain at a known path

### Building

```powershell
# 1. Clone the repository
git clone https://github.com/lextpf/sage.git
cd sage

# 2. Build (configure + compile + docs)
.\build.bat

# 3. Run
.\build\bin\Release\sage.exe
```

Output: `build/bin/Release/sage.exe`

## Architecture

```mermaid
---
config:
  look: handDrawn
  theme: mc
  themeVariables:
    fontSize: 18px
  layout: elk
---
graph TB
    classDef core fill:#1e3a5f,stroke:#3b82f6,color:#e2e8f0
    classDef gui fill:#134e3a,stroke:#10b981,color:#e2e8f0
    classDef vault fill:#4a3520,stroke:#f59e0b,color:#e2e8f0
    classDef crypto fill:#2e1f5e,stroke:#8b5cf6,color:#e2e8f0

    Main["🚀 main.cpp"]:::core

    subgraph GUI["🖥️ GUI Layer"]
        Backend["⚙️ Backend"]:::gui
        QML["🎨 QML Views"]:::gui
        Fill["⌨️ FillController"]:::gui
    end

    subgraph Vault["🗄️ Vault Layer"]
        VaultModel["📋 VaultListModel"]:::vault
        VaultIO["💾 Vault I/O"]:::vault
    end

    subgraph Crypto["🔐 Crypto Layer"]
        Cryptography["🔑 Cryptography"]:::crypto
        FileOps["📁 FileOperations"]:::crypto
        Clipboard["📋 Clipboard"]:::crypto
    end

    Main --> GUI
    Main --> Vault
    GUI --> Vault
    Vault --> Crypto
    GUI --> Crypto
```

|              File | Purpose                                    |
|-------------------|--------------------------------------------|
|        `main.cpp` | Entry point, CLI argument parsing          |
|    `Cryptography` | AES-256-GCM primitives, key derivation     |
|  `FileOperations` | File/directory encryption and streaming    |
|           `Vault` | Vault format, on-demand decryption         |
|      `VaultModel` | QML list model for vault entries           |
|         `Backend` | QML ↔ C++ bridge, application logic        |
|  `FillController` | Credential auto-fill into target windows   |
|       `Clipboard` | Clipboard operations with TTL scrubbing    |
|           `Utils` | String, hex, and path utilities            |

## Project Structure

```
sage/
|-- src/                        # C++ source code
|   |-- main.cpp                # Entry point and CLI parsing
|   |-- Cryptography.cpp/h      # AES-256-GCM, scrypt key derivation
|   |-- FileOperations.cpp/h    # File/directory/stream encryption
|   |-- Vault.cpp/h             # Vault format and on-demand decryption
|   |-- VaultModel.cpp/h        # QML list model for vault entries
|   |-- Backend.cpp/h           # QML backend bridge
|   |-- FillController.cpp/h    # Credential auto-fill controller
|   |-- Clipboard.cpp/h         # Clipboard with TTL scrubbing
|   |-- Utils.cpp/h             # String, hex, path utilities
|   |-- Console.cpp/h           # Console/CLI helpers
|   |-- QmlMain.cpp/h           # QML application entry
|   +-- Version.h               # Version macros
|-- qml/                        # Qt6 QML UI
|   |-- Main.qml                # Application window
|   |-- AccountsTable.qml       # Credential list view
|   |-- AccountDialog.qml       # Add/edit credential dialog
|   |-- PasswordDialog.qml      # Master password prompt
|   +-- ...                     # Theme, headers, search, icons
|-- internal/tess/              # In-tree webcam OCR engine
|-- tests/                      # Google tests
|-- assets/                     # Fonts and icons
|-- scripts/                    # Documentation post-processing
|-- CMakeLists.txt              # Build configuration
|-- vcpkg.json                  # Dependency manifest
|-- build.bat                   # Build pipeline script
+-- mkdocs.yml                  # Documentation site config
```

## Documentation

API documentation is generated via a three-stage pipeline:

```powershell
# 1. Generate markdown from C++ headers
doxide build

# 2. Post-process (strip noise, fix formatting, add icons)
python scripts/clean_docs.py

# 3. Build the documentation site
mkdocs build
```

The site is output to `site/` and can be served locally with `mkdocs serve`.

## Troubleshooting

|                  Problem | Solution                                                                |
|--------------------------|-------------------------------------------------------------------------|
| `SeLockMemoryPrivilege`  | Enable via `gpedit.msc` -> Local Policies -> User Rights Assignment     |
| Remote Desktop blocked   | Intended - sage refuses to run over RDP for security                    |
| vcpkg ninja crash        | Known issue with bundled ninja 1.13.2, retry or use system ninja        |
| Qt plugins not found     | Ensure `windeployqt` ran during build, check `qt.conf` paths            |

## Contributing

Contributions are welcome! Please read the [Contributing Guidelines](CONTRIBUTING.md) before submitting pull requests.

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. ~~Run tests and~~ ensure the build passes
5. Commit with descriptive messages
6. Push to your fork and open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [OpenSSL](https://www.openssl.org/) - Cryptographic primitives
- [Qt](https://www.qt.io/) - GUI framework
- [Google Test](https://github.com/google/googletest) - Testing framework
- [vcpkg](https://github.com/microsoft/vcpkg) - C++ package manager
- [Doxide](https://github.com/lawmurray/doxide) - API documentation generator
- [MkDocs Material](https://squidfunk.github.io/mkdocs-material/) - Documentation theme
- [Claude](https://claude.ai/) - AI coding assistant by Anthropic
- [Codex](https://openai.com/index/openai-codex/) - AI coding assistant by OpenAI
