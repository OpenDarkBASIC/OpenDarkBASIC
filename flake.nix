{
  description = "OpenDarkBASIC Compiler";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };

  outputs = { self, nixpkgs }: let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      odb = pkgs.callPackage ./nix/odb.nix {};
    in {
      packages.${system} = {
        odb = odb;
        default = odb;
      };
      devShells.${system} = pkgs.mkShell {
        buildInputs = [ odb ];
      };
    };
}

