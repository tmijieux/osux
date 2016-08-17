#!/bin/bash

echo "Regenerating autotools files"
autoreconf --force --install || exit 1
