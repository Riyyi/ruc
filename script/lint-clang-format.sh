#!/bin/sh

# Run clang-format across the codebase
# Depends: clang-format, git

# ------------------------------------------

b="$(tput bold)"
red="$(tput setf 4)"
n="$(tput sgr0)"

if [ ! -d ".git" ]; then
   echo "${b}${red}Error:${n} please run this script from the project root" >&2
   exit 1
fi

formatter=false
if command -v clang-format-11 >/dev/null 2>&1; then
	formatter="clang-format-11"
elif command -v clang-format >/dev/null 2>&1; then
	formatter="clang-format"
	if ! "$formatter" --version | awk '{ if (substr($NF, 1, index($NF, ".") - 1) < 11) exit 1; }'; then
		echo "You are using '$("${CLANG_FORMAT}" --version)', which appears to not be clang-format 11 or later."
		exit 1
	fi
else
	echo "clang-format-11 is not available, but C++ files need linting!"
	echo "Either skip this script, or install clang-format-11."
	exit 1
fi

files="${1:-$(git --no-pager diff --cached --name-only)}"
files="$(echo "$files" | grep -E '\.(cpp|h)$')"

if [ -z "$files" ]; then
	exit
fi

# shellcheck disable=SC2086
"$formatter" --style=file -i $files
