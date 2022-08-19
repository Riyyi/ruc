#!/bin/sh

# Run clang-format across the codebase
# Depends: clang-format, git

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

formatter=false
if command -v clang-format-11 >/dev/null 2>&1; then
	formatter="clang-format-11"
elif command -v clang-format >/dev/null 2>&1; then
	formatter="clang-format"
	if ! "$formatter" --version | awk '{ if (substr($NF, 1, index($NF, ".") - 1) < 11) exit 1; }'; then
		error "you are using '$("${formatter}" --version)', which appears to not be clang-format 11 or later."
	fi
else
	error "clang-format-11 is not available, but C++ files need linting!
Either skip this script, or install clang-format-11."
fi

files="${1:-$(git --no-pager diff --cached --name-only)}"
files="$(echo "$files" | grep -E '\.(cpp|h)$')"

if [ -z "$files" ]; then
	exit
fi

# shellcheck disable=SC2086
"$formatter" --style=file -i $files
