#!/bin/sh

# Manage pre-commit hooks in .git/hooks
# Depends: -

# ------------------------------------------

scriptName="$(basename "$0")"

b="$(tput bold)"
u="$(tput smul)"
red="$(tput setf 4)"
n="$(tput sgr0)"

help() {
	cat << EOF
${b}NAME${n}
	${scriptName} - manage pre-commit hooks

${b}SYNOPSIS${n}
	${b}${scriptName}${n} ${u}COMMAND${n}

${b}COMMANDS${n}
	${b}install${n}
		Install all pre-commit hooks.

	${b}remove${n}
		Remove all pre-commit hooks.
EOF
}

# Exit if no option is provided
[ "$#" -eq 0 ] && help && exit 1

if [ ! -d ".git" ]; then
   echo "${b}${red}Error:${n} please run this script from the project root" >&2
   exit 1
fi

# Get the path from the project root to the script
subDir="$(dirname -- "$0")"

create() {
	file="$1"
	if ! test -f "$file"; then
		touch "$file"
		chmod +x "$file"
		echo "#!/bin/sh" > "$file"
	fi
}

install() {
	echo "Installing commit hooks"

	preCommit=".git/hooks/pre-commit"
	create "$preCommit"
	sed -Ei "/lint-ci.sh/d" "$preCommit"
	sed -Ei "\$ a $subDir/lint-ci.sh" "$preCommit"

	commitMsg=".git/hooks/commit-msg"
	create "$commitMsg"
	sed -Ei "/lint-commit.sh/d" "$commitMsg"
	sed -Ei "\$ a $subDir/lint-commit.sh \"\$@\"" "$commitMsg"
}

remove() {
	echo "Removing commit hooks"

	preCommit=".git/hooks/pre-commit"
	sed -Ei "/lint-ci.sh/d" "$preCommit"

	commitMsg=".git/hooks/commit-msg"
	sed -Ei "/lint-commit.sh/d" "$commitMsg"
}

# Command handling
shift $((OPTIND - 1))
case "$1" in
	install | remove)
		"$1"
		;;
	*)
		echo "$scriptName: invalid command '$1'"
		echo "Try '$scriptName -h' for more information."
		exit 1
		;;
esac
