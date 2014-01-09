(function() {
  var currentXHR = null;

  function storage(key, value) {
    var ls = window.localStorage;
    return (arguments.length > 1) ? ls.setItem(key, value) : ls.getItem(key);
  }

  function sendResponse(rewards, stars, balance, status) {
    Pebble.sendAppMessage({
      rewards: rewards,
      stars: stars,
      balance: balance,
      status: status
    });
  }

  function sendError(error) {
    sendResponse("?", "?", "Error", error || "Log in to Sbux in Pebble app.");
  }

  function fetch() {
    if (currentXHR) {
      currentXHR.abort();
    }

    var username = storage('username');
    var password = storage('password');
    if (username && password) {
      function failure(that) {
        that = that || this;
        sendError('HTTP Error ' + that.status);
      };

      function startLoad() {
        currentXHR = this;
      };

      var xhr = new XMLHttpRequest();
      xhr.open('POST', 'https://www.starbucks.com/account/signin', true);
      xhr.onloadstart = startLoad;
      xhr.onload = function() {
        if (xhr.readyState != 4) return;

        if (xhr.status < 200 || xhr.status > 299) {
          failure(this);
          return;
        }

        xhr = new XMLHttpRequest();
        xhr.open('GET', 'https://www.starbucks.com/account/home', true);
        xhr.onloadstart = startLoad;
        xhr.onload = function() {
          if (xhr.readyState != 4) return;

          if (xhr.status < 200 || xhr.status > 299) {
            failure(this);
            return;
          };

          function parseResponseText(key) {
            var re = new RegExp(key + ": '([^']+?)'", "ig");
            var match = re.exec(xhr.responseText);
            return match ? match[1] : '';
          }

          if (parseResponseText('customer_full_name')) {
            var rewards = parseResponseText('num_unredeemed_rewards') || '0';
            var stars = parseResponseText('num_stars_till_next_drink') || '0';
            var balance = parseResponseText('card_dollar_balance');
            if (balance) {
              balance = '$' + parseFloat(Math.round(balance * 100) / 100).toFixed(2);
            } else {
              balance = '$0';
            }
            var dateUpdated = parseResponseText('card_balance_date');
            var timeUpdated = parseResponseText('card_balance_time');
            var status = (dateUpdated && timeUpdated) ? (dateUpdated + '\n' + timeUpdated) : '';
            Pebble.sendAppMessage({
              rewards: rewards,
              stars: stars,
              balance: balance,
              status: status
            });
          } else {
            sendError();
          }
        };
        xhr.onerror = failure;
        xhr.send(null);

        Pebble.sendAppMessage({ status: 'Retrieving balance...' });
      };
      xhr.onerror = failure;

      var params = 'Account.UserName=' + encodeURIComponent(username) + '&Account.PassWord=' + encodeURIComponent(password);
      xhr.send(params);

      Pebble.sendAppMessage({ status: 'Logging in...' });
    } else {
      sendError();
    }
  }

  Pebble.addEventListener('ready', fetch);
  Pebble.addEventListener('appmessage', fetch);

  Pebble.addEventListener("webviewclosed", function(e) {
    if (!e.response) return;
    var payload = JSON.parse(decodeURIComponent(e.response));

    function storeKeyFromPayload(key) {
      if (payload[key]) storage(key, payload[key]);
    }

    storeKeyFromPayload('card_number');
    storeKeyFromPayload('username');
    storeKeyFromPayload('password');
    
    if (payload.barcode_data) Pebble.sendAppMessage({ barcode: payload.barcode_data });
  });

  Pebble.addEventListener("showConfiguration", function() {
    var card_number = storage('card_number') || '';
    var username = storage('username') || '';
    Pebble.openURL('http://a2.github.io/PebbleBucks/configure.html?card_number=' + encodeURIComponent(card_number) + '&username=' + encodeURIComponent(username));
  });
}).call(this);
