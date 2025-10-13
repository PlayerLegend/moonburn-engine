#!/bin/sh

project="$(dirname "$0")"

(cd "$project" && find embed -type f -name '*.glb') | while read file; do
    echo "$file"

    safename="$(echo "$file" | tr '/' '_')"

    (cd "$project" && xxd -i "$file") >"$safename".c
done
