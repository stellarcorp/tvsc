#!/usr/bin/env bash

# Set trap to kill child processes on exit.
trap 'jobs -p | xargs kill' EXIT

service/chat/server/chat_server --alsologtostderr &
service/datetime/server/datetime_server --alsologtostderr &
service/echo/server/echo_server --alsologtostderr &
service/hello/server/hello_server --alsologtostderr &
service/radio/server/tvsc-radio-service --alsologtostderr &

sleep 2

service/proxy/tvsc_proxy --alsologtostderr &

while (true)
do
    sleep 1
done

echo "Exiting."
