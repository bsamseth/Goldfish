#!/bin/bash

set -e

while IFS= read -r link
do
	echo -n "Downloading $link"
	if ! test -f "$(basename "$link")"; then
		wget "$link"
	fi
	echo " -> OK"
done < links.txt


md5sum -c 5-man-checksums.md5 
