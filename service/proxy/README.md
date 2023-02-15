# TVSC Proxy Service

The TVSC Proxy Service is a combination of a web service to RPC proxy and web server. It is designed to be simplistic; for our use cases, it only needs to handle a tiny number of users. As a proxy, it translates web socket requests into gRPC method calls. As a web server, it can only serve static files. In the near future, we plan to add SSL capabilities. Once that is done, the proxy service will support encrypted web connections with a uniform notion of authentication.

## Proxied Services

As of the time of writing, the TVSC Proxy Service handles connections to the following services:

- [radio](../radio)
- [chat](../chat)
- [datetime](../datetime) This is a debug service and may be disabled in some builds.
- [echo](../echo) This is a debug service and may be disabled in some builds.
- [hello](../hello) This is a debug service and may be disabled in some builds.

## Structure

The TVSC Proxy Service is based on gRPC and a web socket library called uWebSockets.
