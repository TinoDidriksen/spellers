#!/bin/bash -e
args=()

while [[ $# > 0 ]];
do
	case "$1" in
	--prefix)
		args+=("-DCMAKE_INSTALL_PREFIX=$2")
		shift 2
		;;
	--prefix=*)
		args+=("-DCMAKE_INSTALL_PREFIX=${1#*=}")
		shift
		;;
	*)
		args+=("$1")
		shift
		;;
	esac
done

set -- "${args[@]}"

echo "- rm -rf CMake caches"
for P in CMakeFiles _CPack_Packages Testing
do
	find . -type d -name "$P" -print0 | xargs -0rn1 rm -rfv
done
for P in install_manifest.txt CMakeCache.txt Makefile '*.cmake'
do
	find . -type f -name "$P" -print0 | xargs -0rn1 rm -rfv
done
echo "- cmake " "$@" "."
cmake "$@" .
echo "- You may now perform: make -j3"
