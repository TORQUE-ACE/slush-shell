
---

```
                         ███████╗██╗     ██╗   ██╗███████╗██╗  ██╗
                         ██╔════╝██║     ██║   ██║██╔════╝██║  ██║
                         ███████╗██║     ██║   ██║███████╗███████║
                         ╚════██║██║     ██║   ██║╚════██║██╔══██║
                         ███████║███████╗╚██████╔╝███████║██║  ██║
                         ╚══════╝╚══════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝
                                         
```

**Slush** is a minimalist, hacker-style shell written entirely in C.  
It offers simplicity, customization, and complete control of the user prompt and shell behavior. 
Perfect for Linux enthusiasts, OS developers, and shell hackers.

---

## ⚡ Overview

- Lightweight, fast shell with a readable C codebase
- Uses `readline` for input with command history
- Prompt shows user@host, current directory, and optional IP/interface
- Optional command logging to `.slush_history.log`
- `.slushrc` configuration system created automatically
- No bloated features — just clean, functional core shell behavior

---

## ✨ Features

| Feature             | Description                                                                 |
|---------------------|-----------------------------------------------------------------------------|
| 🧭 Navigation        | `cd [dir]` to change directories                                              |
| 🌐 Network Prompt    | Show IP/interface in prompt with `network show|hide`                         |
| 📓 Command Logging   | Enable with `log on`, disables with `log off` (stored in `~/.slush_history.log`) |
| 🧑 Root Masking      | Replace username with `root` using `rootmask on|off`                         |
| 🛠️ Auto Config       | Creates default `~/.slushrc` on first run                                   |
| 📂 Current Directory | Prompt always shows current working directory                               |

---

## 🧪 Built-in Commands

```bash
cd [dir]           # Change directory
network show       # Show IP and interface in prompt
network hide       # Hide network info from prompt
log on             # Enable command logging
log off            # Disable command logging
rootmask on        # Mask username as 'root' in prompt
rootmask off       # Use actual username
```
---

## 🛠️ Installation

```bash
# Clone the repository
https://github.com/TORQUE-ACE/slush-shell.git
cd slush

# Compile the source code
gcc slush.c -o slush -lreadline

# Run the shell
./slush
```
