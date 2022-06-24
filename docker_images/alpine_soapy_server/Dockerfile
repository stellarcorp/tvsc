FROM alpine:edge
MAINTAINER James Pack

RUN apk update
RUN apk add --no-cache avahi augeas dbus avahi-compat-libdns_sd
RUN apk add --no-cache bash
RUN apk add --no-cache rtl-sdr
RUN apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/testing soapy-sdr-remote

RUN rm -rf /var/cache/apk/*

RUN echo "blacklist dvb_usb_rtl28xxu" > /etc/modprobe.d/blacklist-dvb.conf

COPY entrypoint.sh /opt/entrypoint.sh
RUN chmod a+x /opt/entrypoint.sh

ENTRYPOINT ["/opt/entrypoint.sh"]
