# PebbleBucks 2.0

PebbleBucks is a Starbucks watch app for the Pebble Smartwatch. It shows your Starbucks barcode and your account information - including the dollar balance, time when it was last updated, number of stars and number of rewards.

Select button updates the account info with the latest data available.

Idea and images are from the original PebbleBucks by mattdonders.

## Requirements

* Place your barcode in `resources/images/barcode.png`.
	* Take a screenshot of the mobile app.
	* Crop the image to longer side being 168 pixels and shorter side being about 63 pixels.
	* Rotate it 90° such that the height is longer than the width.
* Add your Starbucks credentials in `src/js/config.js`.

Note: By default, PebbleBucks uses my own API that scrapes the required data from starbucks.com. It is open sourced [here](), feel free to run it on your own server if you don't want to pass your credentials through mine. (although I try to keep my server very secure and the API does not keep any logs or store anything.)

## Build

	$ sh build.sh

The `build.sh` script generates the `pebble-js-app.js` with your Starbucks credentials, so use that instead of the normal `pebble build`.

## Install

Build and install:

	$ sh build.sh install

Only install:

	$ pebble install

## Screenshot

![](http://cl.ineal.me/image/0m2t1T3s0O0t/PebbleBucks.png)
