#!/bin/bash

build=""

PS3="Select build type:"
options=("Debug" "Release")
select opt in "${options[@]}"

do
  case $opt in
    "Debug")
      build="-DCMAKE_BUILD_TYPE=Debug"
      break
      ;;
    "Release")
      build="-DCMAKE_BUILD_TYPE=Release"
      break
      ;;
    *) echo "invalid option $REPLY";;
  esac
done

cmake -S . -B ./build "${build}"
(cd build && make)
