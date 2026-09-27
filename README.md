# Caps Lang

Use Caps Lock to switch the keyboard layout on Windows.

- **Caps Lock** switches to the next input language in the active window.
- **Shift+Caps Lock** toggles Caps Lock as usual.
- **Ctrl+Shift+L** exits.

A tiny background program (one C file, no UI, no settings) built on a low-level keyboard hook.
It re-installs the hook every 5 minutes, since Windows can silently drop low-level hooks.
Only one instance runs at a time.

## Install

Download `install.cmd`, `capslang-task.xml` and `capslang-x64.exe` or `capslang-arm64.exe`
from the [latest release](https://github.com/mfilippov/caps-lang/releases/latest) into one
folder and run `install.cmd`. It checks the signature, copies the exe to
`C:\ProgramData\caps-lang\capslang.exe`, registers a "Caps Lang" task that starts it at logon,
and starts it now.

Uninstall: press Ctrl+Shift+L, then

```
schtasks /delete /tn "Caps Lang" /f
rmdir /s /q C:\ProgramData\caps-lang
```

## Verify

Release executables are Authenticode-signed by Mikhail Filippov and have GitHub build
provenance:

```
gh attestation verify capslang-x64.exe --repo mfilippov/caps-lang
```

## Build

With the Visual Studio C++ build tools, from a Developer Command Prompt:

```
cl capslang.c /link user32.lib /SUBSYSTEM:WINDOWS
```

Or with Zig: `zig build-exe -lc --subsystem windows capslang.c -target x86_64-windows`.
