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

./bazelisk-linux-amd64 coverage --compilation_mode opt //...
genhtml --output coverage "$(./bazelisk-linux-amd64 info output_path)/_coverage/_coverage_report.dat"

# Reset the cache for the optimized mode build on the local system.
./bazelisk-linux-amd64 test --compilation_mode opt //...
