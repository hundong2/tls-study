#!/usr/bin/env bash
set -euo pipefail

# Purpose:
# - Reuse a persistent Ubuntu container named "ubuntu-dev2"
# - Mount this repo to /workspace
# - Enter an interactive shell or run a command (e.g. make)

NAME="ubuntu-dev2"
IMAGE="ubuntu:latest"
WORKDIR="/workspace"
HOST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Run optional command inside the container.
# Examples:
#   ./dev-ubuntu-dev2.sh            # open bash
#   ./dev-ubuntu-dev2.sh make safe  # run make safe
CMD=("${@:-bash}")

if ! command -v docker >/dev/null 2>&1; then
  echo "docker not found. Install Docker Desktop (macOS) or docker engine." >&2
  exit 1
fi

# Create container if missing; otherwise ensure it's started.
if ! docker ps -a --format '{{.Names}}' | grep -qx "$NAME"; then
  # First-time create.
  docker run -it \
    --name "$NAME" \
    -v "$HOST_DIR:$WORKDIR" \
    -w "$WORKDIR" \
    "$IMAGE" bash
  exit 0
fi

# Start if stopped.
if ! docker ps --format '{{.Names}}' | grep -qx "$NAME"; then
  docker start "$NAME" >/dev/null
fi

# Exec into it.
docker exec -it -w "$WORKDIR" "$NAME" "${CMD[@]}"
