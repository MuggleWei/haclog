#!/bin/bash

origin_dir="$(dirname "$(readlink -f "$0")")"
build_dir=$origin_dir/build
dist_dir=$origin_dir/dist

cd $origin_dir
mkdir -p $build_dir

cmake \
	-S $origin_dir -B $build_dir \
	-DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS=ON \
	-DHACLOG_BUILD_EXAMPLE=OFF \
	-DBUILD_TESTING=OFF \
	-DHACLOG_BUILD_BENCHMARK=OFF \
	-DHACLOG_BUILD_GBENCHMARK=OFF \
	-DCMAKE_INSTALL_PREFIX=$dist_dir
cmake --build $build_dir
cmake --build $build_dir --target install
