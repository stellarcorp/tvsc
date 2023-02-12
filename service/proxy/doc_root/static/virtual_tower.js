var protos = {};

var web_sockets = {};
var datetime_interval_id = -1;
var current_date = new Date();

function DecodeWsMessage(evt, proto_type) {
  var as_array = new Uint8Array(evt.data);
  var received_msg = protos[proto_type].decode(as_array);
  return received_msg;
}

function BuildWebSocketUrl(service, method) {
  // Note that location.host includes the hostname and the port as "hostname:port".
  return 'ws://' + location.host + '/service/' + service + '/' + method;
}

function CreateEchoSocket() {
  var ws = new WebSocket(BuildWebSocketUrl('echo', 'echo'));
  ws.binaryType = 'arraybuffer';

  ws.onmessage = function(evt) {
    var received_msg = DecodeWsMessage(evt, 'tvsc.service.echo.EchoReply');
    $('#echo_reply').text(received_msg.msg);
  };

  return ws;
}

function ChangeDatetimeButtonToStop() {
  $('#datetime_button').html('Pause')
  $('#datetime_button').on('click.stop', function() {
    StopDatetimeRequests();
    $('#datetime_button').off('click.stop');
  });
}

function ChangeDatetimeButtonToStart() {
  $('#datetime_button').html('Resume')
  $('#datetime_button').on('click.start', function() {
    StartDatetimeRequests();
    $('#datetime_button').off('click.start');
  });
}

function StartDatetimeRequests() {
  if (web_sockets['datetime'] == null) {
    CreateDatetimeSocket(StartDatetimeRequests, StopDatetimeRequests);
  } else if (
      web_sockets['datetime'].readyState == 1 /* OPEN */ &&
      web_sockets['datetime'].bufferedAmount == 0) {
    web_sockets['datetime'].send('');
    ChangeDatetimeButtonToStop();
  } else {
    // Try again in a bit.
    window.setTimeout(function() {
      StartDatetimeRequests();
    }, 5000);
  }
}

function StopDatetimeRequests() {
  if (datetime_interval_id != -1) {
    window.clearInterval(datetime_interval_id);
    datetime_interval_id = -1;
  }
  ChangeDatetimeButtonToStart();
}

function CreateDatetimeSocket(starter, stopper) {
  // Change to a subscription to a topic.
  var ws = new WebSocket(BuildWebSocketUrl('datetime', 'stream_datetime'));
  ws.binaryType = 'arraybuffer';

  ws.onopen = starter;

  ws.onmessage = function(evt) {
    var received_msg = DecodeWsMessage(evt, 'tvsc.service.datetime.DatetimeReply');
    current_date.setTime(received_msg.datetime);
    var date_text = current_date.toISOString();
    $('#datetime_reply').text(date_text);
  };

  ws.onerror = function(evt) {
    console.log('Error on Datetime socket: ' + evt);
  };

  ws.onclose = function(evt) {
    console.log('Closing Datetime socket: ' + evt);
    stopper(evt);
    web_sockets['datetime'] = null;
  };

  web_sockets['datetime'] = ws;
}

function GetRadioList() {
  web_sockets['radio/list_radios'].send('');
}

function CreateRadioListSocket() {
  var ws = new WebSocket(BuildWebSocketUrl('radio', 'list_radios'));
  ws.binaryType = 'arraybuffer';

  ws.onmessage = function(evt) {
    var radios = DecodeWsMessage(evt, 'tvsc.service.radio.Radios');
    $('#radio_list').empty();
    console.log(radios.radios)
    for (let radio of radios.radios) {
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
  };

  ws.onopen = function() {
    GetRadioList();
  };

  return ws;
}

function CreateWebSockets() {
  if (!('WebSocket' in window)) {
    // The browser doesn't support WebSocket
    alert('WebSocket NOT supported by your Browser!');
  } else {
    CreateDatetimeSocket(StartDatetimeRequests, StopDatetimeRequests);

    web_sockets['echo'] = CreateEchoSocket();

    // web_sockets['radio/list_radios'] = CreateRadioListSocket();
  }
}

function CallEcho(msg) {
  web_sockets['echo'].send(msg);
}

// This script gives a callback after a javascript file has been loaded. We use this to ensure
// that dependencies are loaded before using them.
function LoadScript(url, callback) {
  // Add a script tag to the document and leverage that tag's events to trigger the callback at the
  // appropriate time.
  var head = document.head;
  var script = document.createElement('script');
  script.type = 'text/javascript';
  script.src = url;

  // We bind multiple events to the callback function.
  // There are several events for cross browser compatibility.
  script.onreadystatechange = callback;
  script.onload = callback;

  head.appendChild(script);
}

function initialize_module() {
  LoadScript('/static/protobuf-7.2.2.js', function() {
    protobuf.load('/static/datetime.proto').then(function(root) {
      protos['tvsc.service.datetime.DatetimeRequest'] =
          root.lookupType('tvsc.service.datetime.DatetimeRequest');
      protos['tvsc.service.datetime.DatetimeReply'] =
          root.lookupType('tvsc.service.datetime.DatetimeReply');
    });
    protobuf.load('/static/echo.proto').then(function(root) {
      protos['tvsc.service.echo.EchoRequest'] = root.lookupType('tvsc.service.echo.EchoRequest');
      protos['tvsc.service.echo.EchoReply'] = root.lookupType('tvsc.service.echo.EchoReply');
    });
    protobuf.load('/static/radio.proto').then(function(root) {
      protos['tvsc.service.radio.RadioListRequest'] =
          root.lookupType('tvsc.service.radio.RadioListRequest');
      protos['tvsc.service.radio.Radio'] = root.lookupType('tvsc.service.radio.Radio');
      protos['tvsc.service.radio.Radios'] = root.lookupType('tvsc.service.radio.Radios');
    });

    CreateWebSockets();
  });
}

window.onload = initialize_module;
