{
	# Source
	# <https://github.com/Xe/douglas-adams-quotes/blob/main/flake.nix>

	description = "NoteSeek";

	inputs = {
		nixpkgs.url = "nixpkgs/nixos-24.11";
	};

	outputs = { self, nixpkgs }: let
		version = builtins.substring 0 8 self.lastModifiedDate;
		supportedSystems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];
		forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
		allpkgs = forAllSystems (system: import nixpkgs { inherit system; });
	in
	{
		# nix build .#<name>
		packages = forAllSystems (system:
			let pkgs = allpkgs.${system}; in
			{
				ytseeker = pkgs.stdenv.mkDerivation {
					pname = "ytseeker";
					inherit version;
					src = ./.;

					nativeBuildInputs = with pkgs; [ gcc gnumake ];
					buildInputs = with pkgs; [ openssl ];

					buildPhase = ''
						make bin/ytseeker
					'';

					installPhase = ''
						mkdir -p $out/bin
						mv bin/ytseeker $out/bin
					'';
				};
			}
		);

		# nix run .#<name>
		apps = forAllSystems(system: {
			ytseeker = {
				program = "${self.packages.${system}.ytseeker}/bin/ytseeker";
				type = "app";
			};
		});

		# nix develop
		devShell = forAllSystems(system:
			let pkgs = allpkgs.${system}; in
			pkgs.mkShell {
				shellHook = ''
					PROMPT=$'%F{8} %~ %B%F{6}$%f%b ' zsh && exit
				'';

				# <https://discourse.nixos.org/t/use-buildinputs-or-nativebuildinputs-for-nix-shell/8464/2>

				# Build time - cross compilation
				nativeBuildInputs = with pkgs; [ gcc ];

				# Run time
				buildInputs = with pkgs; [ openssl ];
			}
		);
	};
}
