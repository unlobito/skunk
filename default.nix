{ pebbleDotNixSrc ? builtins.fetchTarball {
    url = "https://github.com/Sorixelle/pebble.nix/archive/24c445d0766573f7b09982b5ff3cedf9622c2723.tar.gz";
    sha256 = "1cnbz4d4r8b5gpvc45dmgj1pj24qxdry6xh4rrikqrdjialfvq6w";
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
