#!/bin/bash

origin_dir="$(dirname "$(readlink -f "$0")")"
build_dir=$origin_dir/build
dist_dir=$origin_dir/dist

cd $origin_dir
rm -rf $build_dir
rm -rf $dist_dir

meson setup \
	--buildtype=release \
	--default-library=shared \
	--libdir=lib \
	--prefix=$dist_dir \
	$build_dir

meson configure \
	-DHACLOG_BUILD_TESTING=true \
	-DHACLOG_BUILD_EXAMPLE=true \
	-DHACLOG_BUILD_BENCHMARK=true \
	-DHACLOG_BUILD_GBENCHMARK=true \
	$build_dir

meson compile -C $build_dir
meson install -C $build_dir
