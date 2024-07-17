# A configuration for the shell needed for this project
{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell
{
  nativeBuildInputs = with pkgs; [
    SDL2
    SDL2_ttf
    SDL2_image
  ];

  shellHook = ''
    echo "Welcome to the SDL2 devlopment shell"
    '';

}
