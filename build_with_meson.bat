@echo off

setlocal ENABLEDELAYEDEXPANSION

set origin_dir=%~dp0
set build_dir=%origin_dir%build
set dist_dir=%origin_dir%dist

cd %origin_dir%
RD /S /Q %build_dir%
RD /S /Q %dist_dir%

meson setup ^
    --buildtype=release ^
	--default-library=shared ^
	--libdir=lib ^
	--prefix=%dist_dir% ^
    %build_dir%

cd %build_dir%
meson compile -C %build_dir%
meson install -C %build_dir%