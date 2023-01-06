#!/bin/sh

CurrentDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
Platform="${1:-$OSTYPE}"

if [[ $Platform == "win32" || $Platform == "msys" ]]; then
	PlatformDependencyFolder="$CurrentDirectory/../Dependencies"
elif [[ $Platform == "darwin"* ]]; then
	CMakeGenerator="-G Xcode"
	PlatformDependencyFolder="$CurrentDirectory/../Dependencies"
elif [[ $Platform == "linux-gnu"* ]]; then
	CMakeGenerator="-G Ninja Multi-Config"
	PlatformDependencyFolder="$CurrentDirectory/../Dependencies"
else
	echo "[Error] Unknown platform. Supported values are: win32, darwin, linux-gnu; got $Platform."
	exit 1
fi