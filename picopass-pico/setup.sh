#!/bin/bash

TARGET="${HOME}/pico-sdk"

if [ -d "$TARGET" ] && [ "$(ls -A "$TARGET")" ]; then
  if [ -d "$TARGET/.git" ]; then
    echo "Updating existing pico-sdk..."
    git -C "$TARGET" fetch --depth=1 origin 2.2.0 || true
    git -C "$TARGET" checkout 2.2.0 || true
    git -C "$TARGET" submodule update --init --recursive
  else
    echo "Directory exists and is not empty but is not a git repo: $TARGET"
  fi
else
  git clone --branch 2.2.0 --recursive https://github.com/raspberrypi/pico-sdk.git "$TARGET"
fi

echo "PICO_SDK_PATH set to $TARGET"

echo "Setup complete."