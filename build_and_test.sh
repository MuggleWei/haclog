#!/bin/bash

origin_dir="$(dirname "$(readlink -f "$0")")"
build_dir=$origin_dir/build
dist_dir=$origin_dir/dist

cd $origin_dir
mkdir -p $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=Release \
	-DHACLOG_BUILD_EXAMPLE=ON \
	-DBUILD_TESTING=ON \
	-DHACLOG_BUILD_BENCHMARK=ON \
	-DHACLOG_BUILD_GBENCHMARK=ON \
	-DCMAKE_INSTALL_PREFIX=$dist_dir
cmake --build $build_dir

cd $build_dir
cmake --build $build_dir --target test
