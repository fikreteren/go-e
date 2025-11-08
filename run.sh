#!/usr/bin/env bash
set -e

# build everything (using makefiles)
cmake -S ./ipc_task -B ./ipc_task/build
cmake --build ./ipc_task/build --parallel

# start the application
./ipc_task/output/ipc_task
