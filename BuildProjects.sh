#!/usr/bin/env bash

c_compiler=""
cxx_compiler=""

while getopts 'cg' flag; do
  case "${flag}" in
    c)
      c_compiler="-D CMAKE_C_COMPILER=clang"
      cxx_compiler="-D CMAKE_CXX_COMPILER=clang++"
      ;;
    g)
      c_compiler="-D CMAKE_C_COMPILER=gcc"
      cxx_compiler="-D CMAKE_CXX_COMPILER=g++"
      ;;
    *) error "Unexpected option ${flag}" ;;
  esac
done

buildtype=""

PS3="Select build type:"
options=("Debug" "Release")
select opt in "${options[@]}"

do
  case $opt in
    "Debug")
      buildtype="-DCMAKE_BUILD_TYPE=Debug"
      break
      ;;
    "Release")
      buildtype="-DCMAKE_BUILD_TYPE=Release"
      break
      ;;
    *) echo "invalid option $REPLY";;
  esac
done

cmake -S . -B ./build "${buildtype}" "${c_compiler}" "${cxx_compiler}"
(cd build && make -j)
