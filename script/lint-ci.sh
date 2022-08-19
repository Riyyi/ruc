#!/bin/sh

# Run all linters
# Depends: git

# ------------------------------------------

b="$(tput bold)"
red="$(tput setf 4)"
yellow="$(tput setf 6)"
blue="$(tput setf 1)"
green="$(tput setf 2)"
n="$(tput sgr0)"

if [ ! -d ".git" ]; then
	echo "${b}${red}Error:${n} please run this script from the project root" >&2
	exit 1
fi

# Temporary clear unstaged files from the repository to pass the diff checker
unstaged="$(git --no-pager diff --name-only)"
if [ -n "$unstaged" ]; then
	patch=".git/patch-$(date +%s)"
	echo "[${yellow}WARNING${n}]: Stashing unstaged files to $patch"
	git --no-pager diff > "$patch"
	git restore .

	remove() {
		git apply "$patch"
		rm -rf "${patch:?}"
		echo "[${blue}INFO${n}]: Restored changes from $patch"
	}
	trap remove EXIT HUP INT TERM
fi

# Get the path from the project root to the script
subDir="$(dirname -- "$0")"

# Get all files staged for commit
files="$(git --no-pager diff --cached --name-only)"

failures=0

linters="
lint-shell-script.sh
"

for linter in $linters; do
	echo "Running $subDir/$linter"
	if "$subDir/$linter" "$files"; then
		echo "[${green}PASS${n}]: $subDir/$linter"
	else
		echo "[${red}FAIL${n}]: $subDir/$linter"
		failures=$(( failures + 1 ))
	fi
done

echo "Running $subDir/lint-clang-format.sh"
# shellcheck disable=SC2086
if "$subDir/lint-clang-format.sh" "$files" && git diff --exit-code $files; then
    echo "[${green}PASS${n}]: $subDir/lint-clang-format.sh"
else
    echo "[${red}FAIL${n}]: $subDir/lint-clang-format.sh"
	failures=$(( failures + 1 ))
fi

exit "$failures"
