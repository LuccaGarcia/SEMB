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

      flash-util = pkgs.writeShellScriptBin "flash" ''
        # Check if an argument is provided
          if [ -z "$1" ]; then
            echo "Usage: $0 <file.elf>"
            exit 1
          fi

        # Run the openocd command with the specified file
        openocd \
        -f interface/cmsis-dap.cfg \
        -f target/rp2040.cfg \
        -c "adapter speed 5000" \
        -c "program $1 verify reset exit"
      '';

      serial-monitor-util = pkgs.writeShellScriptBin "monitor" ''
        minicom -D /dev/ttyACM0 -b 115200
      '';

      debug-util = pkgs.writeShellScriptBin "dbg-server" ''
        openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c 'adapter speed 5000'
      '';
    in {
      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          cmake
          gcc-arm-embedded
          gdb
          clang-tools
          libusb1
          openocd
          # pico-sdk is being built locally
          # pico-sdk-submodules
          picotool
          python3
          minicom
          bear

          glibc_multi

          flash-util
          serial-monitor-util
          debug-util
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
