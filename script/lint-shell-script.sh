#!/bin/sh

# Run shellcheck across the codebase
# Depends: git, shellcheck

# ------------------------------------------

error() {
	b="$(tput bold)"
	red="$(tput setf 4)"
	n="$(tput sgr0)"

	echo "${b}${red}Error:${n} $1" >&2
	exit 1
}

if [ ! -d ".git" ]; then
	error "please run this script from the project root"
fi

if ! command -v shellcheck > /dev/null 2>&1; then
	error "shellcheck is not available, but shell-script files need linting!
Either skip this script, or install shellcheck."
fi

files="${1:-$(git --no-pager diff --cached --name-only --diff-filter=d)}"
files="$(echo "$files" | grep -E '\.sh$')"

if [ -z "$files" ]; then
	exit
fi

# shellcheck disable=SC2086
shellcheck $files
