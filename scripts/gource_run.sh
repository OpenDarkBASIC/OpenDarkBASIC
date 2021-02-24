#!/bin/sh
gource --time-scale 4 --key --file-filter "$(cat scripts/ignore_files)"
