@echo off

setlocal ENABLEDELAYEDEXPANSION

set origin_dir=%~dp0
set build_dir=%origin_dir%build
set dist_dir=%origin_dir%dist

md %build_dir%

cmake ^
	-S %origin_dir% -B %build_dir% ^
    -DCMAKE_BUILD_TYPE=Release ^
	-DHACLOG_BUILD_EXAMPLE=ON ^
	-DBUILD_TESTING=ON ^
	-DHACLOG_BUILD_BENCHMARK=ON ^
	-DHACLOG_BUILD_GBENCHMARK=ON ^
	-DCMAKE_INSTALL_PREFIX=%dist_dir%

cmake --build %build_dir% --config Release

cd %build_dir%
cmake --build %build_dir% --config Release --target RUN_TESTS