#!/bin/bash

set -e

for link in $(cat links.txt); do
	wget $link
done
