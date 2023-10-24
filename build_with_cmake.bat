@echo off

setlocal ENABLEDELAYEDEXPANSION

set origin_dir=%~dp0
set build_dir=%origin_dir%build
set dist_dir=%origin_dir%dist

cd %origin_dir%
RD /S /Q %build_dir%
RD /S /Q %dist_dir%

md %build_dir%
cd %build_dir%
cmake ^
    -S %origin_dir% -B %build_dir% ^
	-DBUILD_SHARED_LIBS=ON ^
	-DHACLOG_BUILD_EXAMPLE=OFF ^
	-DBUILD_TESTING=OFF ^
	-DHACLOG_BUILD_BENCHMARK=OFF ^
	-DHACLOG_BUILD_GBENCHMARK=OFF ^
	-DCMAKE_INSTALL_PREFIX=%dist_dir%
cmake --build %build_dir% --config Release
cmake --build %build_dir% --config Release --target install