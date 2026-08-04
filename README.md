
---

# ft-IRC

> A modern IRC server implementation built with C++20, developed as a 42 school project.

---

## 📖 Table of Contents

- [About](#about)
- [Features](#features)
- [Tech Stack](#tech-stack)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [Usage](#usage)
- [Commands](#commands)
- [Project Structure](#project-structure)
- [Authors](#authors)
- [License](#license)

---

## About

**ft-IRC** is a functional IRC (Internet Relay Chat) server written from scratch. This project was developed as part of the 42 curriculum to deepen understanding of networking, socket programming, and client-server architecture. The server handles multiple simultaneous clients, supports channel operations, and implements core IRC protocol commands.

---

## Features

- **Multi-client support** – Handles multiple concurrent connections using non-blocking I/O
- **Channel management** – Create, join, leave, and delete channels
- **Operator privileges** – Channel operators with kick, invite, topic, and mode management
- **Core IRC commands** – Full implementation of essential IRC commands:
  - Connection: `NICK`, `USER`, `PASS`, `QUIT`
  - Messaging: `PRIVMSG`
  - Channels: `JOIN`, `PART`, `TOPIC`, `INVITE`, `KICK`
  - Modes: `MODE` (channel modes)
  - Utilities: `PING` (and reply with `PONG`)
- **Logging system** – Comprehensive server activity logging
- **Client registration guard** – Ensures proper client authentication flow
- **Signal handling** – Graceful shutdown on interrupt signals

---

## Tech Stack

| Component | Technology |
|-----------|------------|
| **Language** | C++20 |
| **Build System** | Make |
| **Networking** | Socket programming (non-blocking I/O) |
| **Standard Library** | STL containers and algorithms |
| **Code Quality** | clang-format, clang-tidy, clangd |

---

## Getting Started

### Prerequisites

- C++20 compatible compiler (clang++ or g++)
- Make
- IRC client for testing (e.g., Irssi, WeeChat, HexChat, or `nc`)

### Installation

```bash
# Clone the repository
git clone https://github.com/mordori/ft-IRC.git
cd ft-IRC

# Build the project
make
```

### Usage

Start the server:

```bash
./ircserv <port> <password>
```

- `<port>` – The port number on which the server will listen (e.g., 6667)
- `<password>` – The server connection password required by clients

Connect with an IRC client:

```bash
# Using netcat (for testing)
nc <host> <port>

# Using an IRC client
irc <host> <port> <password>
```

---

## Commands

| Command | Description | Example |
|---------|-------------|---------|
| `PASS <password>` | Authenticate with the server | `PASS secret123` |
| `NICK <nickname>` | Set or change your nickname | `NICK Alice` |
| `USER <user> <mode> <unused> <realname>` | Register user information | `USER alice 0 * :Alice Johnson` |
| `JOIN <channel>` | Join a channel | `JOIN #general` |
| `PART <channel>` | Leave a channel | `PART #general` |
| `PRIVMSG <target> :<message>` | Send a private message | `PRIVMSG #general :Hello everyone!` |
| `TOPIC <channel> :<topic>` | Set or view channel topic | `TOPIC #general :Welcome to the chat!` |
| `INVITE <nickname> <channel>` | Invite a user to a channel | `INVITE Bob #general` |
| `KICK <channel> <user> :<reason>` | Kick a user from a channel | `KICK #general Bob :Spamming` |
| `MODE <target> <modes>` | Set user or channel modes | `MODE #general +o Bob` |
| `QUIT :<message>` | Disconnect from the server | `QUIT :Goodbye!` |

---

## Project Structure

```
ft-IRC/
├── inc/					# Header files
│   ├── Commands/
│   │   ├── ICommand.hpp	# Command interface
│   │   ├── Invite.hpp
│   │   ├── Join.hpp
│   │   ├── Kick.hpp
│   │   └── ...
│   ├── Channel.hpp
│   ├── Client.hpp
│   ├── CommandRequest.hpp
│   ├── Server.hpp
│   └── Utils.hpp
├── src/					# Source files
│   ├── Channel.cpp
│   ├── Client.cpp
│   ├── CommandRequest.cpp
│   ├── Server.cpp
│   └── main.cpp
├── Makefile				# Build configuration
├── .clang-format			# Code formatting rules
├── .clang-tidy				# Static analysis rules
└── .clangd					# Language server configuration
```

---

## Authors

| Author | GitHub |
|--------|--------|
| **myli-pen** | [@Mika](https://github.com/mordori) |
| **hyunjkim** | [@Jean](https://github.com/kkzzbb) |
| **gita** | [@Giang](https://github.com/Kjngita) |

---

## License

This project is part of the curriculum at **42 School** and is intended for educational purposes.

---

<div align="center">

**[⬆ Back to Top](#ft-irc)**

</div>