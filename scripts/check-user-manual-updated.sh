#!/bin/sh
set -eu

manual_path="docs/USER_MANUAL.md"
mode="${1:---staged}"

if [ "${SKIP_USER_MANUAL_CHECK:-}" = "1" ]; then
    exit 0
fi

repo_root="$(git rev-parse --show-toplevel 2>/dev/null)"
cd "$repo_root"

case "$mode" in
    --staged)
        changed_files="$(git diff --cached --name-only --diff-filter=ACMR)"
        ;;
    --working-tree)
        changed_files="$({
            git diff --name-only --diff-filter=ACMR
            git ls-files --others --exclude-standard
        } | sort -u)"
        ;;
    *)
        echo "usage: $0 [--staged|--working-tree]" >&2
        exit 2
        ;;
esac

[ -n "$changed_files" ] || exit 0

manual_changed=0
feature_changed=0

printf '%s\n' "$changed_files" | while IFS= read -r path; do
    [ -n "$path" ] || continue

    if [ "$path" = "$manual_path" ]; then
        manual_changed=1
    fi

    case "$path" in
        src/*|tests/*|openspec/specs/*|openspec/changes/*|todo/TODO.md|assets/ART_DIRECTION.md)
            feature_changed=1
            ;;
    esac

    printf '%s %s\n' "$manual_changed" "$feature_changed"
done | tail -n 1 | {
    read -r manual_changed feature_changed

    if [ "$feature_changed" = "1" ] && [ "$manual_changed" != "1" ]; then
        echo "Feature-facing files are staged, but $manual_path is not staged." >&2
        echo "Update the user manual with new controls/features, then git add $manual_path." >&2
        echo "For mechanical non-feature commits only: SKIP_USER_MANUAL_CHECK=1 git commit" >&2
        exit 1
    fi
}
