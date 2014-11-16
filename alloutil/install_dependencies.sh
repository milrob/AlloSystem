#!/bin/sh
# AlloUtil dependencies install script

# Helper functions.
binary_exists(){
	command -v "$1" >/dev/null 2>&1;
}
# End helper functions.

ROOT=$(pwd)
PLATFORM=$(uname)
echo "Installing for ${PLATFORM} from ${ROOT}"

if binary_exists 'apt-get'; then
	echo 'Found apt-get'
	sudo apt-get update
	sudo apt-get install libjsoncpp-dev

elif binary_exists "brew"; then
	echo 'Found Homebrew'
	brew update
	brew install jsoncpp

elif binary_exists "port"; then
	echo 'Found MacPorts'
	sudo port selfupdate
	sudo port install jsoncpp

# TODO: Install jsoncpp.
elif uname | grep "MINGW"; then
	nop

else
	echo 'Error: No suitable package manager found.'
	echo 'Error: Install apt-get, MacPorts, or Homebrew and try again.'
fi
