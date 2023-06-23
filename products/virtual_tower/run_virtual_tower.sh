#!/usr/bin/env bash

# Set trap to kill child processes on exit.
trap 'jobs -p | xargs kill' EXIT

service/datetime/server/datetime_server --alsologtostderr &
service/echo/server/echo_server --alsologtostderr &
service/hello/server/hello_server --alsologtostderr &
service/communications/server/tvsc-communications-service --alsologtostderr &

sleep 2

service/proxy/tvsc_proxy --alsologtostderr --doc_root=products/virtual_tower/ui &

while (true)
do
    sleep 1
done

echo "Exiting."
