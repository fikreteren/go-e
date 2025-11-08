#!/usr/bin/env bash
set -e

sudo docker build -t ipc_project:latest .
sudo docker run --rm -it ipc_project:latest