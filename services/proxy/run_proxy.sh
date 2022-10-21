#!/usr/bin/env bash

# Set trap to kill child processes on exit.
trap 'jobs -p | xargs kill' EXIT

pushd services/virtual_tower_ui
mini_httpd -p 50053 -dd . -D &
popd

services/hello/server/greeter_server --alsologtostderr &
services/echo/server/echo_server --alsologtostderr &
services/datetime/server/datetime_server --alsologtostderr &
services/radio/server/radio_server --alsologtostderr &
services/proxy/tvsc_proxy --alsologtostderr

echo "Exiting."
