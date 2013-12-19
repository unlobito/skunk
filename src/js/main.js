var maxAppMessageTries = 3;
var appMessageRetryTimeout = 3000;
var appMessageTimeout = 100;
var appMessageQueue = [];

function sendAppMessage() {
	if (appMessageQueue.length > 0) {
		currentAppMessage = appMessageQueue[0];
		currentAppMessage.numTries = currentAppMessage.numTries || 0;
		currentAppMessage.transactionId = currentAppMessage.transactionId || -1;
		if (currentAppMessage.numTries < maxAppMessageTries) {
			console.log('Sending AppMessage to Pebble: ' + JSON.stringify(currentAppMessage.message));
			Pebble.sendAppMessage(
				currentAppMessage.message,
				function(e) {
					appMessageQueue.shift();
					setTimeout(function() {
						sendAppMessage();
					}, appMessageTimeout);
				}, function(e) {
					console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
					appMessageQueue[0].transactionId = e.data.transactionId;
					appMessageQueue[0].numTries++;
					setTimeout(function() {
						sendAppMessage();
					}, appMessageRetryTimeout);
				}
			);
		} else {
			console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Bailing. ' + JSON.stringify(currentAppMessage.message));
		}
	}
}

function fetch() {
	var params = 'username=' + username + '&password=' + password;
	var xhr = new XMLHttpRequest();
	xhr.open('POST', api_url, true);
	xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xhr.setRequestHeader('Content-length', params.length);
	xhr.setRequestHeader('Connection', 'close');
	xhr.onload = function(e) {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				if (xhr.responseText) {
					res = JSON.parse(xhr.responseText);
					rewards = res.rewards || '0';
					stars = res.stars || '0';
					balance = res.dollar_balance || '0';
					balance = '$' + parseFloat(Math.round(balance * 100) / 100).toFixed(2);
					status = res.dollar_balance_updated || '';
					status = status.replace(' ', '\n');
					appMessageQueue.push({'message': {'rewards': rewards, 'stars': stars, 'balance': balance, 'status': status}});
				} else {
					console.log('Invalid response received! ' + JSON.stringify(xhr));
					appMessageQueue.push({'message': {'status': 'Invalid response!'}});
				}
			} else {
				console.log('Request returned error code ' + xhr.status.toString());
				appMessageQueue.push({'message': {'status': 'HTTP/1.1 ' + xhr.statusText}});
			}
		}
		sendAppMessage();
	}
	xhr.onerror = function() {
		console.log('HTTP request return error');
		appMessageQueue.push({'message': {'status': 'Failed to connect!'}});
		sendAppMessage();
	};
	xhr.send(params);
}

Pebble.addEventListener('ready', function(e) {
	fetch();
});

Pebble.addEventListener('appmessage', function(e) {
	fetch();
});

