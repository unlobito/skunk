var PebbleBucks = {};

PebbleBucks.domain = 'https://pebblebucks.herokuapp.com';
PebbleBucks.state = null;
PebbleBucks.token = '';
PebbleBucks.updating = false;
PebbleBucks.version = encodeURIComponent('3.0');

PebbleBucks.loadState = function() {
  var state_json = window.localStorage.state;
  if (!state_json) return false;

  PebbleBucks.state = JSON.parse(state_json);
  return true;
};

PebbleBucks.saveState = function() {
  if (!PebbleBucks.state) return false;

  window.localStorage.state = JSON.stringify(PebbleBucks.state);
  return true;
};

PebbleBucks.URLCredentials = function() {
  var credentials = window.localStorage.credentials;
  if (!credentials) {
    return false;
  }

  var credentialsbits = credentials.split("&");

  var credentialsURLencoded = "access_token=" + encodeURIComponent(credentialsbits[0].substr(13)) + "&access_token_secret=" + encodeURIComponent(credentialsbits[1].substr(20));

  return credentialsURLencoded;
}

PebbleBucks.onShowConfiguration = function() {
  var credentials = PebbleBucks.URLCredentials();
  if (!credentials) {
    var url = PebbleBucks.domain + '/login?pebble=' + PebbleBucks.token + '&version=' + PebbleBucks.version;
    Pebble.openURL(url);
  } else {
    var credentialsbits = credentials.split("&");

    var url = PebbleBucks.domain + '/settings?pebble=' + PebbleBucks.token + '&version=' + PebbleBucks.version + "&" + credentials;
    Pebble.openURL(url);
  }
};

PebbleBucks.onWebViewClosed = function(event) {
  var response = event.response;
  if (!response || response.indexOf('access_token') === -1) return;

  window.localStorage.credentials = decodeURIComponent(response);
};

PebbleBucks.sendPayload = function(payload, name, callback) {
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

PebbleBucks.sendPayloads = function(payloads, callback, i) {
  i = i || 0;
  if (i >= payloads.length) {
    callback();
    return;
  }

  var tuple = payloads[i];
  PebbleBucks.sendPayload(tuple[0], tuple[1], function(success) {
    PebbleBucks.sendPayloads(payloads, callback, i + 1);
  });
};

PebbleBucks.sendData = function(callback) {
  if (!PebbleBucks.state && !PebbleBucks.loadState()) {
    console.log("[sendData] Couldn't load state!");
    return false;
  }

  var data = PebbleBucks.state.data;
  var cards = data.cards;
  var rewards = data.rewards;

  var payloads = [];

  {
    var payload = { number_of_cards: cards.length };
    for (var key in rewards) {
      payload["rewards_" + key] = rewards[key];
    }
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
  PebbleBucks.sendPayloads(payloads, callback);
};

PebbleBucks.onAppMessage = function(event) {
  var payload = event.payload;
  if (payload.fetch_data && !PebbleBucks.updating) {
    PebbleBucks.updating = true;
    PebbleBucks.fetchData(function(success) {
      var done = function() {
        PebbleBucks.updating = false;
      };

      if (success) {
        PebbleBucks.sendData(done);
      } else {
        done();
      }
    });
  }
};

PebbleBucks.cleanOldKeys = function() {
  var keys = ['card_number', 'username', 'password'];
  for (var i = 0; i < keys.length; i++) {
    window.localStorage.removeItem(keys[i]);
  }
};

PebbleBucks.sendError = function(message) {
  console.log('[sendError] ' + message);
  PebbleBucks.sendPayload({ error: message }, 'error');
};

PebbleBucks.fetchData = function(callback) {
  var credentials = PebbleBucks.URLCredentials();
  if (!credentials) {
    console.log('[fetchData] No credentials.');
    PebbleBucks.sendError('Please log in.');
    callback(false);
    return;
  }

  var xhr = new XMLHttpRequest();

  var url = PebbleBucks.domain + '/data?pebble=' + PebbleBucks.token + '&version=' + PebbleBucks.version;
  xhr.open('POST', url, true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState != 4) return;

    if (xhr.status < 400) {
      // 2xx or 3xx
      if (!PebbleBucks.state) PebbleBucks.state = {};
      PebbleBucks.state.data = JSON.parse(xhr.responseText);
      PebbleBucks.state.updated_at = Date.now();
      PebbleBucks.saveState();
      callback(true);
    } else {
      var error = (xhr.status == 401) ? 'Please log in' : 'HTTP ' + xhr.status;
      PebbleBucks.sendError(error);
      callback(false);
    }
  };
  xhr.onerror = function() {
    console.log('Error: ' + xhr.statusText);
    PebbleBucks.sendError('Request failure');
    callback(false);
  };

  xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');

  console.log('[fetchData] Fetching data...');
  xhr.send(credentials);
};

PebbleBucks.init = function() {
  PebbleBucks.cleanOldKeys();
  PebbleBucks.token = encodeURIComponent(Pebble.getAccountToken());

  Pebble.addEventListener('showConfiguration', PebbleBucks.onShowConfiguration);
  Pebble.addEventListener('webviewclosed', PebbleBucks.onWebViewClosed);
  Pebble.addEventListener('appmessage', PebbleBucks.onAppMessage);
};


Pebble.addEventListener('ready', PebbleBucks.init);
