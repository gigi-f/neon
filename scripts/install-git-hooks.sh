#!/bin/sh
set -eu

repo_root="$(git rev-parse --show-toplevel 2>/dev/null)"
cd "$repo_root"

git config core.hooksPath .githooks
echo "Git hooks installed from .githooks"
