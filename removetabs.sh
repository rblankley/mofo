#!/usr/bin/env bash

# convert tabs to spaces
find . -name '*.h' ! -type d -exec bash -c 'expand -t 4 "$0" > /tmp/e && mv /tmp/e "$0"' {} \;
find . -name '*.cpp' ! -type d -exec bash -c 'expand -t 4 "$0" > /tmp/e && mv /tmp/e "$0"' {} \;

# use unix style line endings
find . -name '*.h' ! -type d -exec dos2unix {} {} \;
find . -name '*.cpp' ! -type d -exec dos2unix {} {} \;

