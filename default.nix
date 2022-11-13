{ pebbleDotNixSrc ? builtins.fetchTarball {
    url = "https://github.com/Sorixelle/pebble.nix/archive/a7be1fd6523b32f176a9558c7719beab94f9ba1c.tar.gz";
    sha256 = "1x8izcv77biwx8l8ic57vfz72yg9p4v1k36f1z97lcl7631vyhy7";
  }
, pebbleDotNix ? import pebbleDotNixSrc
, packageJSON ? builtins.fromJSON (builtins.readFile ./package.json)
}:

(pebbleDotNix.buildPebbleApp {
  inherit (packageJSON) name version;
  type = "watchapp";
  src = ./.;

  category = "Tools & Utilities";
  description = ''
Skunk allows easy access to up to eight store cards and other barcodes.

Linear Barcodes: Codabar, Code 39, Code 128, EAN-8, EAN-13, Interleaved 2 of 5 (ITF), UPC-A
Matrix Barcodes: Aztec, Data Matrix, PDF417, QR
'';

  releaseNotes = "Whoop.";

  screenshots = {
    all = [ "assets/screenshot_coffee.png" "assets/screenshot_flight.png" "assets/screenshot_loyalty.png" "assets/screenshot_supermarket.png" ];
  };
  banner = "assets/banner.png";
  smallIcon = "assets/icon_small.png";
  largeIcon = "assets/icon_large.png";
  sourceUrl = "https://github.com/unlobito/skunk";
}) // {
  devShell = pebbleDotNix.pebbleEnv { };
}
