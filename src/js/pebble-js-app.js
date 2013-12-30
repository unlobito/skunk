var currentXHR = null;

function fetch() {
	if (currentXHR) {
		currentXHR.abort();
		currentXHR = null;
	}

	function sendError(error) {
		Pebble.sendAppMessage({
			rewards: "?",
			stars: "?",
			balance: "Error",
			status: error || "Log in to Sbux in Pebble app."
		});
	};

	var username = window.localStorage.getItem('username');
	var password = window.localStorage.getItem('password');
	if (!(username && password)) {
		sendError();
		return;
	}

	function failure(that) {
		var theOther = that || this;
		sendError(theOther.status + ' ' + theOther.statusText);
	};

	function startLoad() {
		currentXHR = this;
	};

	var postXHR = new XMLHttpRequest();
	postXHR.open('POST', 'https://www.starbucks.com/account/signin', true);
	postXHR.onloadstart = startLoad;
	postXHR.onload = function() {
		if (postXHR.readyState != 4) return;

		if (postXHR.status < 200 || postXHR.status > 299) {
			failure(this);
			return;
		}

		var getXHR = new XMLHttpRequest();
		getXHR.open('GET', 'https://www.starbucks.com/account/home', true);
		getXHR.onloadstart = startLoad;
		getXHR.onload = function() {
			if (getXHR.readyState != 4) return;

			if (getXHR.status < 200 || getXHR.status > 299) {
				failure(this);
				return;
			};

			function parseResponseText(key) {
				var re = new RegExp(key + ": '([^']+?)'", "ig");
				var match = re.exec(getXHR.responseText);
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
		getXHR.onerror = failure;
		getXHR.send(null);

		Pebble.sendAppMessage({ status: 'Retrieving balance...' });
	};
	postXHR.onerror = failure;

	var params = 'Account.UserName=' + encodeURIComponent(username) + '&Account.PassWord=' + encodeURIComponent(password);
	postXHR.send(params);

	Pebble.sendAppMessage({ status: 'Logging in...' });
}

Pebble.addEventListener('ready', fetch);
Pebble.addEventListener('appmessage', fetch);

Pebble.addEventListener("webviewclosed", function(e) {
	if (!e.response) return;
	var payload = JSON.parse(decodeURIComponent(e.response));
	var keys = [ 'card_number', 'username', 'password' ];
	for (var i = 0; i < keys.length; i++) {
		var key = keys[i];
		if (payload[key]) {
			window.localStorage.setItem(key, payload[key]);
		}
	}
	if (payload.barcode_data) Pebble.sendAppMessage({ barcode: payload.barcode_data });
});

Pebble.addEventListener("showConfiguration", function() {
	var card_number = window.localStorage.getItem('card_number') || '';
	var username = window.localStorage.getItem('username') || '';
	Pebble.openURL('http://a2.github.io/PebbleBucks/configure.html?card_number=' + encodeURIComponent(card_number) + '&username=' + encodeURIComponent(username));
});