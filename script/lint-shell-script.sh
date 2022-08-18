#!/bin/sh

# Run shellcheck across the codebase
# Depends: git, shellcheck

# ------------------------------------------

b="$(tput bold)"
red="$(tput setf 4)"
n="$(tput sgr0)"

if [ ! -d ".git" ]; then
   echo "${b}${red}Error:${n} please run this script from the project root" >&2
   exit 1
fi

if ! command -v shellcheck > /dev/null 2>&1; then
	echo "shellcheck is not available, but shell script files need linting!"
	echo "Either skip this script, or install shellcheck."
	exit 1
fi

files="${1:-$(git --no-pager diff --cached --name-only)}"
files="$(echo "$files" | grep -E '\.sh$')"

if [ -z "$files" ]; then
	exit
fi

# shellcheck disable=SC2086
shellcheck $files
