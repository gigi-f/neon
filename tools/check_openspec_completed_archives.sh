#!/usr/bin/env bash
set -euo pipefail

mode="${1:---check}"
if [[ "$mode" != "--check" && "$mode" != "--archive" ]]; then
    echo "Usage: $0 [--check|--archive]" >&2
    exit 2
fi

if [[ ! -d "openspec" ]]; then
    exit 0
fi

openspec_bin="${OPENSPEC_BIN:-}"
if [[ -z "$openspec_bin" ]]; then
    openspec_bin="$(command -v openspec || true)"
fi
if [[ -z "$openspec_bin" || ! -x "$openspec_bin" ]]; then
    echo "OpenSpec archive check skipped: openspec CLI not found." >&2
    exit 0
fi

list_output="$("$openspec_bin" list 2>/dev/null || true)"
completed_changes="$(
    printf '%s\n' "$list_output" |
        awk '/Complete/ && $1 != "Changes:" { print $1 }'
)"

if [[ -z "$completed_changes" ]]; then
    exit 0
fi

if [[ "$mode" == "--archive" ]]; then
    while IFS= read -r change; do
        [[ -z "$change" ]] && continue
        POSTHOG_DISABLED=1 "$openspec_bin" archive "$change" --yes
    done <<< "$completed_changes"
    exit 0
fi

echo "Completed OpenSpec changes are still active and must be archived before completion:" >&2
while IFS= read -r change; do
    [[ -z "$change" ]] && continue
    echo "  POSTHOG_DISABLED=1 openspec archive $change --yes" >&2
done <<< "$completed_changes"
echo "Then rerun: openspec list && POSTHOG_DISABLED=1 openspec validate --all --strict" >&2
exit 1
