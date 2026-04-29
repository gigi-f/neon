#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build"
TRANSCRIPT="$ROOT/.neon_feature_playtest.txt"
CHECK_CHANGED=0

usage() {
    cat <<'USAGE'
Usage: tools/verify_feature_completion.sh [--transcript PATH] [--check-changed]

Verifies that gameplay feature work included a current AI playtest pass.

Required feature-completion workflow:
  1. Update the playtest harness when a feature adds/changes verbs, targets,
     readouts, map glyphs, scenarios, or state the AI needs to observe.
  2. Run an interactive terminal playtest and save the transcript:
       ./build/neon_ai_playtest play default --transcript .neon_feature_playtest.txt
  3. Run this script before calling a feature complete.

Options:
  --transcript PATH   Transcript produced by neon_ai_playtest play.
  --check-changed     Require transcript to be newer than changed gameplay files.
USAGE
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --transcript)
            TRANSCRIPT="$2"
            shift 2
            ;;
        --check-changed)
            CHECK_CHANGED=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

mtime() {
    if stat -f %m "$1" >/dev/null 2>&1; then
        stat -f %m "$1"
    else
        stat -c %Y "$1"
    fi
}

require_file_contains() {
    local file="$1"
    local needle="$2"
    if ! grep -Fq -- "$needle" "$file"; then
        echo "Missing expected text in $file: $needle" >&2
        exit 1
    fi
}

cd "$ROOT"

if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    cmake -S "$ROOT" -B "$BUILD_DIR"
fi

cmake --build "$BUILD_DIR" --target neon_ai_playtest

HARNESS_STATE="$(mktemp "${TMPDIR:-/tmp}/neon_harness_state.XXXXXX")"
HARNESS_SNAPSHOT="$(mktemp "${TMPDIR:-/tmp}/neon_harness_snapshot.XXXXXX")"
trap 'rm -f "$HARNESS_STATE" "$HARNESS_SNAPSHOT"' EXIT

"$BUILD_DIR/neon_ai_playtest" reset default --state "$HARNESS_STATE" >"$HARNESS_SNAPSHOT"

require_file_contains "$HARNESS_SNAPSHOT" "COMMANDS: snapshot | key W/A/S/D/E/F/T/SPACE/G"
require_file_contains "$HARNESS_SNAPSHOT" "-- PLAYER VIEW 33x17 CELL=8WU CENTERED ON @ --"
require_file_contains "$HARNESS_SNAPSHOT" "ACTION:"
require_file_contains "$HARNESS_SNAPSHOT" "TARGET:"
require_file_contains "$HARNESS_SNAPSHOT" "DEBUGGER_RESULT:"

if [[ ! -f "$TRANSCRIPT" ]]; then
    cat >&2 <<EOF
Missing interactive playtest transcript: $TRANSCRIPT

Run a real observe-act-observe playtest before marking gameplay work complete:
  ./build/neon_ai_playtest play default --transcript $TRANSCRIPT

Use 'reset suspicion' inside play mode when the new feature touches local-risk,
suspicion, debugger, or wage/debt behavior.
EOF
    exit 1
fi

require_file_contains "$TRANSCRIPT" "=== NEON INTERACTIVE PLAYTEST ==="
require_file_contains "$TRANSCRIPT" "ai>"
require_file_contains "$TRANSCRIPT" "=== FINAL OUTPUT ==="
require_file_contains "$TRANSCRIPT" "-- PLAYER VIEW 33x17 CELL=8WU CENTERED ON @ --"
require_file_contains "$TRANSCRIPT" "ACTION:"
require_file_contains "$TRANSCRIPT" "TARGET:"
require_file_contains "$TRANSCRIPT" "DEBUGGER_RESULT:"

if [[ "$CHECK_CHANGED" -eq 1 ]]; then
    transcript_mtime="$(mtime "$TRANSCRIPT")"
    changed_files="$(
        {
            git diff --cached --name-only --diff-filter=ACMR
            git diff --name-only --diff-filter=ACMR
        } | sort -u | grep -E '^(src/|tools/ai_playtest.cpp|tests/ai_playtest_tests.cpp|CMakeLists.txt)$|^src/|^tools/ai_playtest.cpp|^tests/ai_playtest_tests.cpp' || true
    )"

    while IFS= read -r changed; do
        [[ -z "$changed" || ! -f "$changed" ]] && continue
        changed_mtime="$(mtime "$changed")"
        if [[ "$changed_mtime" -gt "$transcript_mtime" ]]; then
            cat >&2 <<EOF
Playtest transcript is older than changed gameplay/harness file:
  transcript: $TRANSCRIPT
  changed:    $changed

Re-run an interactive playtest after the latest feature edits:
  ./build/neon_ai_playtest play default --transcript $TRANSCRIPT
EOF
            exit 1
        fi
    done <<< "$changed_files"
fi

echo "Feature completion playtest gate passed."
