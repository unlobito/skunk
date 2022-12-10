var Skunk = {};

Skunk.domain = 'https://skunk-config.canidae.systems';
Skunk.state = null;
Skunk.token = '';
Skunk.updating = false;
Skunk.version = encodeURIComponent('1.7');

// http://phpjs.org/functions/base64_encode/
function base64_encode(data) {
  var b64 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
  var o1, o2, o3, h1, h2, h3, h4, bits, i = 0,
    ac = 0,
    enc = '',
    tmp_arr = [];

  if (!data) {
    return data;
  }

  do { // pack three octets into four hexets
    o1 = data.charCodeAt(i++);
    o2 = data.charCodeAt(i++);
    o3 = data.charCodeAt(i++);

    bits = o1 << 16 | o2 << 8 | o3;

    h1 = bits >> 18 & 0x3f;
    h2 = bits >> 12 & 0x3f;
    h3 = bits >> 6 & 0x3f;
    h4 = bits & 0x3f;

    // use hexets to index into b64, and append result to encoded string
    tmp_arr[ac++] = b64.charAt(h1) + b64.charAt(h2) + b64.charAt(h3) + b64.charAt(h4);
  } while (i < data.length);

  enc = tmp_arr.join('');

  var r = data.length % 3;

  return (r ? enc.slice(0, r - 3) : enc) + '==='.slice(r || 3);
}

// http://phpjs.org/functions/base64_decode/
function base64_decode(data) {
  var b64 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
  var o1, o2, o3, h1, h2, h3, h4, bits, i = 0,
    ac = 0,
    dec = '',
    tmp_arr = [];

  if (!data) {
    return data;
  }

  data += '';

  do { // unpack four hexets into three octets using index points in b64
    h1 = b64.indexOf(data.charAt(i++));
    h2 = b64.indexOf(data.charAt(i++));
    h3 = b64.indexOf(data.charAt(i++));
    h4 = b64.indexOf(data.charAt(i++));

    bits = h1 << 18 | h2 << 12 | h3 << 6 | h4;

    o1 = bits >> 16 & 0xff;
    o2 = bits >> 8 & 0xff;
    o3 = bits & 0xff;

    if (h3 == 64) {
      tmp_arr[ac++] = String.fromCharCode(o1);
    } else if (h4 == 64) {
      tmp_arr[ac++] = String.fromCharCode(o1, o2);
    } else {
      tmp_arr[ac++] = String.fromCharCode(o1, o2, o3);
    }
  } while (i < data.length);

  dec = tmp_arr.join('');

  return dec.replace(/\0+$/, '');
}

Skunk.loadState = function() {
  var state_json = localStorage.state;
  if (!state_json) return false;

  Skunk.state = JSON.parse(state_json);
  return true;
};

Skunk.saveState = function() {
  if (!Skunk.state) return false;

  localStorage.state = JSON.stringify(Skunk.state);
  return true;
};

Skunk.onShowConfiguration = function() {
  var url;
  if (!localStorage.config || localStorage.config === "") {
    url = Skunk.domain + '/settings';
  } else {
    url = Skunk.domain + '/settings#' + base64_encode(localStorage.config);
  }
  Pebble.openURL(url);
};

Skunk.onWebViewClosed = function(event) {
  var response = event.response;
  if (!response || base64_decode(response).indexOf('{') === -1) return;

  localStorage.config = base64_decode(response);

  Pebble.sendAppMessage({pushing_data: true});
};

Skunk.sendPayload = function(payload, name, callback) {
  var pid = -1;
  var log = function(success) {
    return function() {
      console.log('[sendPayload] ' + (!success ? 'N' : '') + 'ACK ' + name + ' (' + pid + ')');
      if (callback) callback(success);
    };
  };
  pid = Pebble.sendAppMessage(payload, log(true), log(false));
  return pid;
};

Skunk.sendPayloads = function(payloads, callback, i) {
  i = i || 0;
  if (i >= payloads.length) {
    callback();
    return;
  }

  var tuple = payloads[i];
  Skunk.sendPayload(tuple[0], tuple[1], function(success) {
    Skunk.sendPayloads(payloads, callback, i + 1);
  });
};

Skunk.sendData = function(callback) {
  if (!Skunk.state && !Skunk.loadState()) {
    console.log("[sendData] Couldn't load state!");
    return false;
  }

  var data = Skunk.state.data;
  var cards = data.cards;

  var payloads = [];

  var payload = { number_of_cards: cards.length };
  payloads.push([payload, 'main']);

  for (var i = 0; i < cards.length; i++) {
    var card = cards[i];
    payload = { card_index: i };
    for (var key in card) {
      payload["card_" + key] = card[key];
    }
    payloads.push([payload, 'card #' + i]);
  }

  console.log("[sendData] Sending " + payloads.length + " payload(s)");
  Skunk.sendPayloads(payloads, callback);
};

Skunk.onAppMessage = function(event) {
  var payload = event.payload;
  if (payload.fetch_data && !Skunk.updating) {
    Skunk.updating = true;
    Skunk.fetchData(function(success) {
      var done = function() {
        Skunk.updating = false;
      };

      if (success) {
        Skunk.sendData(done);
      } else {
        done();
      }
    });
  }
};

Skunk.sendError = function(message) {
  console.log('[sendError] ' + message);
  Skunk.sendPayload({ error: message }, 'error');
};

Skunk.fetchData = function(callback) {
  if (!localStorage.config || localStorage.config === "") {
    console.log('[fetchData] No state.');
    Skunk.sendError('Please open Settings.');
    callback(false);
    return;
  }

  var xhr = new XMLHttpRequest();

  var url = Skunk.domain + '/data';
  xhr.open('POST', url, true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState != 4) return;

    if (xhr.status < 400) {
      // 2xx or 3xx
      if (!Skunk.state) Skunk.state = {};
      Skunk.state.data = JSON.parse(xhr.responseText);
      Skunk.state.updated_at = Date.now();
      Skunk.saveState();
      callback(true);
    } else {
      var error = (xhr.status == 401) ? 'Please log in' : 'HTTP ' + xhr.status;
      Skunk.sendError(error);
      callback(false);
    }
  };
  xhr.onerror = function() {
    console.log('Error: ' + xhr.statusText);
    Skunk.sendError('Request failure');
    callback(false);
  };

  xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');

  console.log('[fetchData] Fetching data...');
  xhr.send(localStorage.config);
};

Skunk.init = function() {
  Skunk.token = encodeURIComponent(Pebble.getAccountToken());

  Pebble.addEventListener('showConfiguration', Skunk.onShowConfiguration);
  Pebble.addEventListener('webviewclosed', Skunk.onWebViewClosed);
  Pebble.addEventListener('appmessage', Skunk.onAppMessage);
};


Pebble.addEventListener('ready', Skunk.init);
