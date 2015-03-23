#! /bin/bash

function die() {
	echo "$@" >&2
	exit 1
}

[ $# = 1 ] || die "You must specify a lab directory."
cd "$1" || die "No such directory: $1"

[ -f answers.txt ] || die "No answers.txt file!"

BOLD="`echo -ne \\\\033[1m`"
NORM="`echo -ne \\\\033[0m`"

netid=`grep "^net ID:" answers.txt | sed -e 's/^[^:]*:\s*//' | grep .`
[ "$netid" = "" ] && die "
*** You have not yet filled in your ${BOLD}net ID${NORM} in the answers.txt file!
*** Fill out the answers.txt file and try again.
*** NOTE: the format of net ID should be:
net ID: abc123
"

ids=`grep "^NYU ID(N#):" answers.txt | sed -e 's/^[^:]*:\s*//' | grep .`
[ "$ids" = "" ] && die "
*** You have not yet filled in your ${BOLD}NYU ID(N#)${NORM} in the answers.txt file!
*** Fill out the answers.txt file and try again.
*** NOTE: the format of NYU ID should be:
NYU ID(N#): N12345678
"

echo
echo "Here's the information in your answers.txt file."
echo "Please make sure it is correct."
echo

echo "Your name(s):$BOLD"
grep "^Name:" answers.txt | sed -e 's/^[^:]*:\s*//' | grep . | sed -e 's/^/	/'
echo "$NORM"

echo "Your net ID:$BOLD"
grep "^net ID:" answers.txt | sed -e 's/^[^:]*:\s*//' | grep . | sed -e 's/^/	/'
echo "$NORM"

echo "Your NYU ID(N#):$BOLD"
grep "^NYU ID(N#):" answers.txt | sed -e 's/^[^:]*:\s*//' | grep . | sed -e 's/^/	/'
echo "$NORM"

echo "Your partner:$BOLD"
grep "^Partner's Name (if any):" answers.txt | sed -e 's/^[^:]*:\s*//' | grep . | sed -e 's/^/	/'
echo "$NORM"

echo -n "Is all this information correct? [y/N] "
read LINE
echo

[ "$LINE" == "y" -o "$LINE"  == "Y" ] || die "Please correct the answers.txt file and try again."

echo "*** Thank you. Lab check passed"
