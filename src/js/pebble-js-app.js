var Skunk = {};

Skunk.domain = 'https://skunkapp.herokuapp.com';
Skunk.state = null;
Skunk.token = '';
Skunk.updating = false;
Skunk.version = encodeURIComponent('1.2');

Skunk.loadState = function() {
  var state_json = window.localStorage.state;
  if (!state_json) return false;

  Skunk.state = JSON.parse(state_json);
  return true;
};

Skunk.saveState = function() {
  if (!Skunk.state) return false;

  window.localStorage.state = JSON.stringify(Skunk.state);
  return true;
};

Skunk.onShowConfiguration = function() {
  if (!window.localStorage.config || window.localStorage.config == "") {
    var url = Skunk.domain + '/settings';
  } else {
    var url = Skunk.domain + '/settings#' + encodeURIComponent(window.localStorage.config);
  }
  Pebble.openURL(url);
};

Skunk.onWebViewClosed = function(event) {
  var response = event.response;
  if (!response || decodeURIComponent(response).indexOf('{') === -1) return;

  window.localStorage.config = decodeURIComponent(response);

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

  {
    var payload = { number_of_cards: cards.length };
    payloads.push([payload, 'main']);
  }

  for (var i = 0; i < cards.length; i++) {
    var card = cards[i];
    var payload = { card_index: i };
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
  if (!window.localStorage.config || window.localStorage.config == "") {
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
  xhr.send(window.localStorage.config);
};

Skunk.init = function() {
  Skunk.token = encodeURIComponent(Pebble.getAccountToken());

  Pebble.addEventListener('showConfiguration', Skunk.onShowConfiguration);
  Pebble.addEventListener('webviewclosed', Skunk.onWebViewClosed);
  Pebble.addEventListener('appmessage', Skunk.onAppMessage);
};


Pebble.addEventListener('ready', Skunk.init);
