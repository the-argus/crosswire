{
  description = "Crosswire Game for 2024 new year new skils jam";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }: let
    supportedSystems = let
      inherit (flake-utils.lib) system;
    in [
      system.aarch64-linux
      system.x86_64-linux
    ];
  in
    flake-utils.lib.eachSystem supportedSystems (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      devShell =
        pkgs.mkShell
        {
          packages =
            (with pkgs; [
              python3Minimal # for running web builds
              gdb
              valgrind
              pkg-config
              libGL
              zig_0_11
              SDL2
              SDL2_image
            ])
            ++ (with pkgs.xorg; [
              libX11
              libXrandr
              libXinerama
              libXcursor
              libXi
            ]);
          shellHook = ''
            export EMSDK="${pkgs.callPackage ./build/nix/emsdk {}}"
          '';
        };

      formatter = pkgs.alejandra;
    });
}
