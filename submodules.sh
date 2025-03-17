#!/usr/bin/env bash

# see https://github.blog/open-source/git/get-up-to-speed-with-partial-clone-and-shallow-clone/
git submodule update --init --filter=blob:none --recursive --single-branch --jobs "$(nproc || echo 1)"
