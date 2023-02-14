# Service

This module contains the high-level services used in our projects. These services are run as individual processes and communicate over gRPC. The individual services should have the same general structure:

- common directory that contains the service definition and other code common between client and server code.
- server directory that contains the code for the server binary. It may also contain other utilities and libraries that are specific to a computer acting as a server for that service.
- client directory that contains the code for various client libraries. It may also contain other utilities and libraries that are specific to using this service as a client.

See the directory for each service for more details on that service. The list of services as of the time of writing is

- [proxy](./proxy) HTTP server that provides web socket interfaces, and static file serving, to the other services.
- [radio](./radio) Service for listing and selecting radios for communications.
- [chat](./chat) Service implementing a trivial chat system.
- [echo](./echo) Test service that echos inputs as outputs.
- [hello](./hello) Test service that issues a greeting. This service has more debugging structure than the other test services.
- [datetime](./datetime) Test service that gives the current date and time using server-side streaming.
