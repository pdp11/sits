#!/bin/bash

SOURCE=$HOME/src/mit-logo-and-sits-raw-files/files

rm -rf src
for i in 11logo nlogo rjl nlogo sits sw jay bee gld bs; do
    mkdir -p src/$i
done

git reset `git rev-list --max-parents=0 HEAD`

previous() {
    f=`basename "$1" | sed 's/\..*//'`
    case "$1" in
        sits/nsits.111) f="sits.[1-9]*";;
        sits/sits.103) f="nsits.[1-9]*";;
        11logo/ologo.11*) f="logo.[1-9]*";;
        11logo/logo.538) f="ologo.[1-9]*";;
        nlogo/logo.1) f="";;
        *read*this*) f="";;
        *.[1-9]*) f="$f.[1-9]*";;
        *.cmd) f="$f.cmd";;
        *) f="$f.[^1-9]*";;
    esac
    find src -name "$f"
}

cat files.txt | while read i; do
    set $i
    t="$1"
    d="${t:0:4}-${t:4:2}-${t:6:2} ${t:8:2}:${t:10:2}:${t:13:2}"
    p=`previous "$3"`
    test -n "$p" && git rm $p
    cp "$SOURCE/$2/$3" "src/$3"
    touch -t "$t" "src/$3"
    git add -A src
    TZ=EST git commit --date "$d" -m "Update $3."
done
