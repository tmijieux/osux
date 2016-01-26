#!/bin/bash

SONG_DIRECTORY=${SONG_DIRECTORY:-"/mnt/windata/Songs/"}

function parse_directory {
    for i in *; do
        if [ -d "$i" ]; then
            pushd "$i" > /dev/null
            parse_directory
            popd > /dev/null
        else
            extension="${i##*.}"
            if [[ "$extension" == "osu" ]]; then
                grep -q -e "Mode: 1" "$i" && echo "$(pwd)/$i"
            fi
        fi
    done
}

pushd $SONG_DIRECTORY > /dev/null
parse_directory

