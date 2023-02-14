let list_radio_rpc = null;
let echo_rpc = null;
let datetime_stream = null;

function TypedArrayToBuffer(array) {
  return array.buffer.slice(array.byteOffset, array.byteLength + array.byteOffset)
}

function BuildWebSocketUrl(service, method) {
  // Note that location.host includes the hostname and the port as "hostname:port".
  return 'ws://' + location.host + '/service/' + service + '/' + method;
}

/**
 * Class to provide a namespace for holding protobuf factories.
 *
 * @class
 */
class Protos {
  static #protos = {};

  static add_proto(name, factory) {
    Protos.#protos[name] = factory;
  }

  static get_proto_factory(name) {
    return Protos.#protos[name];
  }
}

/**
 * Class for managing a websocket-based RPC method.
 *
 * @class
 */
class WebSocketRpc {
  #url;
  #ws;
  #request_proto_encoder;
  #response_proto_decoder;
  #response_handler;
  #error_handler;

  #create_web_socket(open_handler) {
    this.#ws = new WebSocket(this.#url);

    this.#ws.binaryType = 'arraybuffer';

    this.#ws.onopen = open_handler;

    this.#ws.onmessage = (evt) => {
      let as_array = new Uint8Array(evt.data);
      let received_msg = this.#response_proto_decoder.decode(as_array);
      this.#response_handler(received_msg);
    };

    this.#ws.onclose = (evt) => {
      this.#ws = null;
    };

    this.#ws.onerror = (evt) => {
      this.#ws = null;
      this.#error_handler(evt);
    };
  }

  constructor(url, request_proto_encoder_name, response_proto_decoder_name) {
    this.#url = url;
    this.#request_proto_encoder = Protos.get_proto_factory(request_proto_encoder_name);
    this.#response_proto_decoder = Protos.get_proto_factory(response_proto_decoder_name);
    this.#ws = null;
    this.#response_handler = null;
    this.#error_handler = null;
  }

  url() {
    return this.#url;
  }

  send(request) {
    if (this.#ws) {
      let bits = this.#request_proto_encoder.encode(request).finish();
      this.#ws.send(TypedArrayToBuffer(bits));
    } else {
      this.#create_web_socket(() => {
        this.send(request);
      });
    }
  }

  on_receive(response_handler) {
    this.#response_handler = response_handler;
  }

  on_error(error_handler) {
    this.#error_handler = error_handler;
  }
}

/**
 * Class for managing a websocket-based server streams.
 *
 * @class
 */
class WebSocketStream {
  #url;
  #ws;
  #request_proto_encoder;
  #response_proto_decoder;

  #start_handler;
  #response_handler;
  #stop_handler;
  #error_handler;

  #create_web_socket(open_handler) {
    this.#ws = new WebSocket(this.#url);

    this.#ws.binaryType = 'arraybuffer';

    this.#ws.onopen = open_handler;

    this.#ws.onmessage = (evt) => {
      let as_array = new Uint8Array(evt.data);
      let received_msg = this.#response_proto_decoder.decode(as_array);
      this.#response_handler(received_msg);
    };

    this.#ws.onclose = (evt) => {
      this.#ws = null;
      this.#stop_handler(evt);
    };

    this.#ws.onerror = (evt) => {
      this.#ws = null;
      this.#error_handler(evt);
    };
  }

  constructor(url, request_proto_encoder_name, response_proto_decoder_name) {
    this.#url = url;
    this.#request_proto_encoder = Protos.get_proto_factory(request_proto_encoder_name);
    this.#response_proto_decoder = Protos.get_proto_factory(response_proto_decoder_name);
    this.#ws = null;
    this.#response_handler = null;
    this.#error_handler = null;
  }

  url() {
    return this.#url;
  }

  start() {
    if (this.#ws) {
      let bits = this.#request_proto_encoder.encode({}).finish();
      this.#ws.send(TypedArrayToBuffer(bits));
      this.#start_handler()
    } else {
      this.#create_web_socket(() => {
        this.start();
      });
    }
  }

  stop() {
    this.#ws.close();
  }

  toggle() {
    if (this.is_running()) {
      this.stop();
    } else {
      this.start();
    }
  }

  is_running() {
    return this.#ws != null;
  }

  on_start(start_handler) {
    this.#start_handler = start_handler;
  }

  on_receive(response_handler) {
    this.#response_handler = response_handler;
  }

  on_stop(stop_handler) {
    this.#stop_handler = stop_handler;
  }

  on_error(error_handler) {
    this.#error_handler = error_handler;
  }
}

function CreateDatetimeStream() {
  datetime_stream = new WebSocketStream(
      BuildWebSocketUrl('datetime', 'stream_datetime'),  //
      'tvsc.service.datetime.DatetimeRequest',           //
      'tvsc.service.datetime.DatetimeReply');
  datetime_stream.on_receive(function(reply) {
    let current_date = new Date();
    current_date.setTime(reply.datetime);
    let date_text = current_date.toISOString();
    $('#datetime_reply').text(date_text);
  });
  datetime_stream.on_start(function() {
    $('#datetime_button').html('Pause')
  });
  datetime_stream.on_stop(function(evt) {
    $('#datetime_button').html('Resume')
  });
  datetime_stream.on_error(function(evt) {
    $('#datetime_button').html('Resume')
    $('#datetime_reply').text('<error>');
  });
  $('#datetime_button').click(function() {
    datetime_stream.toggle();
  });
  datetime_stream.start();
}

function CreateRadioListSocket() {
  list_radio_rpc = new WebSocketRpc(
      BuildWebSocketUrl('radio', 'list_radios'),  //
      'tvsc.service.radio.RadioListRequest',      //
      'tvsc.service.radio.Radios');
  list_radio_rpc.on_receive(function(reply) {
    $('#radio_list').empty();
    for (let radio of reply.radios) {
      let item_element = $('<li>');
      item_element.append(document.createTextNode(radio.name));
      let keys_values_element = $('<ul class=\'keys_values_element\'>');
      for (let key_value of radio.keysValues) {
        let key_value_element = $('<li class=\'key_value_element\'>')
        key_value_element.append(document.createTextNode(key_value.key + ': ' + key_value.value));
        keys_values_element.append(key_value_element);
      }
      item_element.append(keys_values_element);
      $('#radio_list').append(item_element);
    }
  });
  list_radio_rpc.on_error(function(evt) {
    $('#radio_list').text('<error>');
  });
}

function GetRadioList() {
  list_radio_rpc.send({});
}

function CreateEchoSocket() {
  echo_rpc = new WebSocketRpc(
      BuildWebSocketUrl('echo', 'echo'),  //
      'tvsc.service.echo.EchoRequest',    //
      'tvsc.service.echo.EchoReply');
  echo_rpc.on_receive(function(reply) {
    $('#echo_reply').text(reply.msg);
  });
  echo_rpc.on_error(function(evt) {
    $('#echo_reply').text('<error>');
  });
}

function CallEcho(msg) {
  let echo_request = Protos.get_proto_factory('tvsc.service.echo.EchoRequest').create({msg: msg});
  echo_rpc.send(echo_request);
}

// This script gives a callback after a javascript file has been loaded. We use this to ensure
// that dependencies are loaded before using them.
function LoadScript(url, callback) {
  // Add a script tag to the document and leverage that tag's events to trigger the callback at the
  // appropriate time.
  let head = document.head;
  let script = document.createElement('script');
  script.type = 'text/javascript';
  script.src = url;

  // We bind multiple events to the callback function.
  // There are several events for cross browser compatibility.
  script.onreadystatechange = callback;
  script.onload = callback;

  head.appendChild(script);
}

function initialize_module() {
  if (!('WebSocket' in window)) {
    // The browser doesn't support WebSocket.
    alert('WebSocket NOT supported by your Browser!');
  } else {
    LoadScript('/static/protobuf-7.2.2.js', function() {
      protobuf.load('/static/datetime.proto').then(function(root) {
        Protos.add_proto(
            'tvsc.service.datetime.DatetimeRequest',
            root.lookupType('tvsc.service.datetime.DatetimeRequest'));
        Protos.add_proto(
            'tvsc.service.datetime.DatetimeReply',
            root.lookupType('tvsc.service.datetime.DatetimeReply'));

        CreateDatetimeStream();
      });
      protobuf.load('/static/echo.proto').then(function(root) {
        Protos.add_proto(
            'tvsc.service.echo.EchoRequest', root.lookupType('tvsc.service.echo.EchoRequest'));
        Protos.add_proto(
            'tvsc.service.echo.EchoReply', root.lookupType('tvsc.service.echo.EchoReply'));

        CreateEchoSocket();
      });
      protobuf.load('/static/radio.proto').then(function(root) {
        Protos.add_proto(
            'tvsc.service.radio.RadioListRequest',
            root.lookupType('tvsc.service.radio.RadioListRequest'));
        Protos.add_proto('tvsc.service.radio.Radio', root.lookupType('tvsc.service.radio.Radio'));
        Protos.add_proto('tvsc.service.radio.Radios', root.lookupType('tvsc.service.radio.Radios'));

        CreateRadioListSocket();
      });
    });
  }
}

window.onload = initialize_module;
