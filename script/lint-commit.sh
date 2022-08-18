#!/bin/sh

# Lint commit message
# Depends: git

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

# The file containing the commit message is passed as the first argument
file="$1"
message="$(cat "$file")"

newline="$(printf '\x0D')"
if grep -Uq "$newline" "$file"; then
	error "commit message contains CRLF line breaks (only unix-style LF linebreaks are allowed)"
fi

lineNumber=0
echo "$message" | while read -r line; do
	lineNumber=$((lineNumber + 1))
	lineLength=${#line}

	# Ignore comment lines
	if echo "$line" | awk '$0 !~ /^#.*/ { exit 1 }'; then continue; fi
	# Ignore overlong 'fixup!' commit descriptions
	if echo "$line" | awk '$0 !~ /^fixup! .*/ { exit 1 }'; then continue; fi

	if [ "$lineNumber" -eq 2 ] && [ "$lineLength" -ne 0 ]; then
		error "empty line between commit title and body is missing"
	fi

	categoryPattern="^\S.*?\S: .+"
	if [ $lineNumber -eq 1 ] && (echo "$line" | grep -Evq "$categoryPattern"); then
		error "missing category in commit title (if this is a fix up of a previous commit, it should be squashed)"
	fi

	titleCasePattern="^\S.*?: [A-Z0-9]"
	if [ $lineNumber -eq 1 ] && (echo "$line" | grep -Evq "$titleCasePattern"); then
		error "first word of commit after the subsystem is not capitalized"
	fi

	if [ $lineNumber -eq 1 ] && (echo "$line" | awk '$0 !~ /\.$/ { exit 1 }' ); then
		error "commit title ends in a period"
	fi

	urlPattern="([a-z]+:\/\/)?(([a-zA-Z0-9_]|-)+\.)+[a-z]{2,}(:\d+)?([a-zA-Z_0-9@:%\+.~\?&\/=]|-)+"
	if [ "$lineLength" -gt 72 ] && (echo "$line" | grep -Evq "$urlPattern"); then
		error "commit message lines are too long (maximum allowed is 72 characters)"
	fi
done

exit 0
