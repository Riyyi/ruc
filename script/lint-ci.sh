#!/bin/sh

# Run all linters
# Depends: git

# ------------------------------------------

b="$(tput bold)"
red="$(tput setf 4)"
n="$(tput sgr0)"

if [ ! -d ".git" ]; then
   echo "${b}${red}Error:${n} please run this script from the project root" >&2
   exit 1
fi

# Get the path from the project root to the script
subDir="$(dirname -- "$0")"

# Get all files staged for commit
files="$(git --no-pager diff --cached --name-only)"

green="$(tput setf 2)"
red="$(tput setf 4)"
nc="$(tput sgr0)"

failures=0

linters="
lint-shell-script.sh
"

for linter in $linters; do
	echo "Running $subDir/$linter"
	if "$subDir/$linter" "$files"; then
		echo "[${green}PASS${nc}]: $subDir/$linter"
	else
		echo "[${red}FAIL${nc}]: $subDir/$linter"
		failures=$(( failures + 1 ))
	fi
done

echo "Running $subDir/lint-clang-format.sh"
# shellcheck disable=SC2086
if "$subDir/lint-clang-format.sh" "$files" && git diff --exit-code $files; then
    echo "[${green}PASS${nc}]: $subDir/lint-clang-format.sh"
else
    echo "[${red}FAIL${nc}]: $subDir/lint-clang-format.sh"
	failures=$(( failures + 1 ))
fi

exit "$failures"
