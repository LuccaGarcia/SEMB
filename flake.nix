{
  description = "Basic rpi pico development shell";
  inputs = {
    nixpkgs.url = "nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
      #pico-sdk-submodules = (pkgs.pico-sdk.override (previous: {
      #  withSubmodules = true;
      #}));
    in {
      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          cmake
          gcc-arm-embedded
          libusb1
          openocd
          # pico-sdk-submodules
          picotool
          python3
        ];
        #shellHook = ''
        #  export PICO_SDK_PATH="${pico-sdk-submodules}/lib/pico-sdk"
        #'';
        shellHook = ''
          export PICO_SDK_PATH="/home/dvalinn/.pico/pico-sdk"
        '';
      };
    });
}
