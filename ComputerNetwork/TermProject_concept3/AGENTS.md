# Repository Guidelines

## Project Structure & Module Organization

This repository contains coursework examples for networking and IoT labs. C socket, process, and thread examples live in `ComputerNetwork/`, with related lab 7 files in `ComputerNetwork/lab7/`. IoT examples live in `IoT_Lab/`, grouped by chapter (`ch06/` through `ch10/`); Flask templates are under `templates/`, static assets under `static/`, and MQTT/HTML examples are in `ch10/`. `ComputerNetwork/TermProject/` is currently the workspace for term-project additions. Keep new project source, tests, and documentation grouped here unless the change clearly belongs to an existing lab folder.

## Build, Test, and Development Commands

There is no central build system. Compile C examples directly:

```sh
gcc -Wall -Wextra -pthread ComputerNetwork/chat_serv.c -o ComputerNetwork/chat_serv
gcc -Wall -Wextra ComputerNetwork/echo_client.c -o ComputerNetwork/echo_client
```

Run programs from their source directory when they depend on relative files. For Python/Flask examples, create or reuse a virtual environment and run the target script:

```sh
python3 -m venv IoT_Lab/.venv
IoT_Lab/.venv/bin/python IoT_Lab/ch06/hello_flask.py
```

## Coding Style & Naming Conventions

Match the existing instructional style. C files use simple lowercase names such as `echo_client.c`, `thread1.c`, and `chat_serv.c`; prefer clear function names, four-space indentation, and explicit error handling around socket, process, and thread calls. Python examples use chapter-prefixed filenames such as `6-4.py` and `10-3mqtt.py`; keep scripts small and readable, with configuration values near the top.

## Testing Guidelines

No automated test suite is present. Validate changes by compiling touched C files with warnings enabled and manually exercising client/server pairs on localhost. For Flask or MQTT examples, verify the app starts, routes render, and broker-dependent code handles missing services cleanly. If adding reusable term-project modules, add focused tests under `ComputerNetwork/TermProject/tests/` and document how to run them.

## Commit & Pull Request Guidelines

Recent history uses short, descriptive commits, often in Korean, with occasional Conventional Commit prefixes such as `feat:`. Keep messages concise and specific, for example `feat: add chat server timeout` or `lab7 mutex example update`. Pull requests should describe the changed lab or project area, list manual verification commands, link any related issue, and include screenshots only for web UI changes.

## Security & Configuration Tips

Do not commit virtual environments, generated binaries, `.env` files, or local broker credentials. Existing `.gitignore` files already cover common C, Python, and Flask artifacts; extend them when adding new generated outputs.
