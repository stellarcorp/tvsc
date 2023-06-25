#!/usr/bin/env bash

set -o errexit
set -o nounset

DIRNAME="$( dirname -- "$0"; )"

cd "$DIRNAME"
for mode in fastbuild opt dbg
do
    for sku in linux
    do
	./bazelisk-linux-amd64 test --compilation_mode "${mode}" --config="${sku}" //...
    done
done

for mode in fastbuild opt dbg
do
    for sku in odroid_m1 teensy40 teensy41
    do
	./bazelisk-linux-amd64 build --compilation_mode "${mode}" --config="${sku}" //...
    done
done
