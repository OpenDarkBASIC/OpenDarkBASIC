#!/bin/sh

while inotifywait -e close_write "$1"
do
    dot -Tpdf "$1" > build/out.pdf;
done
