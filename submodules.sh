#!/usr/bin/env bash

git submodule update --init --filter=blob:none --recursive --single-branch --jobs "$(nproc || echo 1)"
