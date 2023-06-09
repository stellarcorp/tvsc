// Objects storing WebSocketRpc instances for various RPCs.
let echo_rpc = null;
let transmit_rpc = null;

// Objects storing WebSocketStreams instances for various streams.
let datetime_stream = null;
let receive_stream = null;

/**
 * The protobuf library generates TypedArrays, but the web socket API uses buffers. Here we
 * transform from the encoded protobuf to the buffer that is needed to transmit the data.
 */
function TypedArrayToBuffer(array) {
  return array.buffer.slice(array.byteOffset, array.byteLength + array.byteOffset)
}

/**
 * Consolidate how web socket URLs are structured.
 */
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
      if (this.#response_handler != null) {
        this.#response_handler(received_msg);
      }
    };

    this.#ws.onclose = (evt) => {
      this.#ws = null;
    };

    this.#ws.onerror = (evt) => {
      this.#ws = null;
      if (this.#error_handler != null) {
        this.#error_handler(evt);
      }
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
      if (this.#response_handler != null) {
        this.#response_handler(received_msg);
      } else {
        console.log('received: ' + received_msg);
      }
    };

    this.#ws.onclose = (evt) => {
      this.#ws = null;
      if (this.#stop_handler != null) {
        this.#stop_handler(evt);
      } else {
        console.log('receive stream closed.');
      }
    };

    this.#ws.onerror = (evt) => {
      this.#ws = null;
      if (this.#error_handler != null) {
        console.log('receive stream error.');
      }
    };
  }

  constructor(url, request_proto_encoder_name, response_proto_decoder_name) {
    this.#url = url;
    this.#request_proto_encoder = Protos.get_proto_factory(request_proto_encoder_name);
    this.#response_proto_decoder = Protos.get_proto_factory(response_proto_decoder_name);
    this.#ws = null;

    this.#start_handler = null;
    this.#response_handler = null;
    this.#stop_handler = null;
    this.#error_handler = null;
  }

  url() {
    return this.#url;
  }

  start() {
    if (this.#ws) {
      let bits = this.#request_proto_encoder.encode({}).finish();
      this.#ws.send(TypedArrayToBuffer(bits));
      if (this.#start_handler != null) {
        this.#start_handler()
      }
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

function CreateTransmitSocket() {
  transmit_rpc = new WebSocketRpc(
      BuildWebSocketUrl('communications', 'transmit'),  //
      'tvsc.service.communications.Message',            //
      'tvsc.service.communications.SuccessResult');

  transmit_rpc.on_receive(function(evt) {
    // Clear the transmit input box.
    $('#transmit_message').val('');
  });

  transmit_rpc.on_error(function(evt) {
    RenderReceivedMessage('<TX error>');
  });
}

function RenderReceivedMessage(msg) {
  let level1 = $('<div class="outgoing-chats">');
  let level2 = $('<div class="outgoing-msg">');
  let level3 = $('<div class="outgoing-chats-msg">');
  let message_element = $('<p>').text(msg);
  let timestamp_element = $('<span class="msg-time">').text('18:06 PM | July 24');

  level3.append(message_element, timestamp_element);
  level2.append(level3);
  level1.append(level2);

  $('#msg-container').append(level1);
}

function CreateReceiveStream() {
  receive_stream = new WebSocketStream(
      BuildWebSocketUrl('communications', 'receive'),  //
      'tvsc.service.communications.EmptyMessage',      //
      'tvsc.service.communications.Message');

  receive_stream.on_receive(function(reply) {
    RenderReceivedMessage(reply.message);
  });

  receive_stream.on_error(function(evt) {
    RenderReceivedMessage('<RX error>');
  });

  receive_stream.start();
}

function CallEcho(msg) {
  let echo_request = Protos.get_proto_factory('tvsc.service.echo.EchoRequest').create({msg: msg});
  echo_rpc.send(echo_request);
}

function CallTransmit(msg) {
  let transmit_request =
      Protos.get_proto_factory('tvsc.service.communications.Message').create({message: msg});
  transmit_rpc.send(transmit_request);
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

function DisplayCommunications() {
  $('.debug').hide();
  $('.communications').show();
  $('.telemetry').hide();
}

function DisplayDebug() {
  $('.debug').show();
  $('.communications').hide();
  $('.telemetry').hide();
}

function DisplayTelemetry() {
  $('.debug').hide();
  $('.communications').hide();
  $('.telemetry').show();
}

/**
 * Function to add sample message data to help with debugging CSS and logic around message
 * rendering. Note that these messages are rendered in the same way as a received message.
 */
function AddSampleMessageData() {
  RenderReceivedMessage(
      'Hi !! This message was received from Riya . Lorem ipsum, dolor si amet consectetur adipisicing elit.Non quas nemo eum, earum sunt, nobis similique quisquam eveniet pariatur commodi modi voluptatibus iusto omnis harum illum iste distinctio expedita illo!');
  RenderReceivedMessage(
      'Hi riya , Lorem ipsum dolor sit amet consectetur adipisicing elit. Illo nobis deleniti earum magni recusandae assumenda.');
  RenderReceivedMessage('Lorem ipsum dolor sit amet consectetur adipisicing elit. Velit, sequi.');
}

function InitializeModule() {
  DisplayCommunications();

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

      protobuf.load('/static/communications.proto').then(function(root) {
        Protos.add_proto(
            'tvsc.service.communications.EmptyMessage',
            root.lookupType('tvsc.service.communications.EmptyMessage'));
        Protos.add_proto(
            'tvsc.service.communications.Message',
            root.lookupType('tvsc.service.communications.Message'));
        Protos.add_proto(
            'tvsc.service.communications.SuccessResult',
            root.lookupType('tvsc.service.communications.SuccessResult'));

        CreateTransmitSocket();
        CreateReceiveStream();

        AddSampleMessageData();
      });
    });
  }
}

window.onload = InitializeModule;
