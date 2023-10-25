cmake_minimum_required (VERSION 3.18.6)
project(haclog)

################################
# general config
################################

message("-- use c compiler ${CMAKE_C_COMPILER}")
message("-- use c++ compiler ${CMAKE_CXX_COMPILER}")

# set compile parameter
if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif (${CMAKE_C_COMPILER_ID} MATCHES Clang)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-missing-field-initializers")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

	# see: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
	# The /Zc:__cplusplus compiler option enables updated __cplusplus macro
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/std:c++20>")
endif()

# Mac M1 clang compiler default c++ version set to c++97, it will lead 
# gbenchmark_haclog compile error
if (APPLE)
	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
		message("# detect APPLE arm, set c&cxx standard to 11")
		set(CMAKE_C_STANDARD 11)
		set(CMAKE_C_STANDARD_REQUIRED ON)
		set(CMAKE_CXX_STANDARD 11)
		set(CMAKE_CXX_STANDARD_REQUIRED ON)
	endif()
endif()

message("-- c compiler support features: ")
foreach(feature ${CMAKE_C_COMPILE_FEATURES})
	message("support feature: ${feature}")
endforeach()

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################
# options
################################

set(CMAKE_BUILD_TYPE "Release" CACHE STRING
	"Build type (Debug, RelWithDebInfo, Release, MinSizeRel or Coverage)")
option(BUILD_SHARED_LIBS "Build shared or static library" ON)
option(BUILD_TESTING "Build testing" OFF)
option(HACLOG_BUILD_EXAMPLE "Build haclog examples" OFF)
option(HACLOG_BUILD_BENCHMARK "Build haclog benchmark" OFF)
option(HACLOG_BUILD_GBENCHMARK "Build haclog google benchmark" OFF)
option(HACLOG_DOWNLOAD_TEST_LIB "Download test lib if build testing is ON" ON)
option(HACLOG_DOWNLOAD_GBENCHMARK_LIB "Download google benchmark lib if build gbenchmark is ON" ON)
option(HACLOG_BUILD_SANITIZER "Compile haclog with sanitizer" OFF)
option(HACLOG_BUILD_COVERAGE "Build haclog coverage" OFF)
if (BUILD_SHARED_LIBS)
	set(HACLOG_LIB_TYPE SHARED)
	set(HACLOG_USE_DLL ON)
else()
	set(HACLOG_LIB_TYPE STATIC)
	set(HACLOG_USE_DLL OFF)
endif()

################################
# coverage
################################

if (${CMAKE_BUILD_TYPE} MATCHES "[Cc]overage")
	set(BUILD_TESTING ON)
	if (${CMAKE_C_COMPILER_ID} MATCHES Clang)
		set(HACLOG_BUILD_COVERAGE ON)
		set(COVERAGE_COMPILER_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
	elseif(${CMAKE_C_COMPILER_ID} MATCHES GNU)
		set(HACLOG_BUILD_COVERAGE ON)
		set(COVERAGE_COMPILER_FLAGS "--coverage")
	else()
		message(FATAL_ERROR "haclog not support coverage with compile: ${CMAKE_C_COMPILER_ID}")
	endif()
endif()

if (HACLOG_BUILD_COVERAGE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}")

	if (${CMAKE_C_COMPILER_ID} MATCHES Clang)
		find_program(LLVM_PROFDATA llvm-profdata)
		find_program(LLVM_COV_PATH llvm-cov)
		if (LLVM_PROFDATA AND LLVM_COV_PATH)
			set(COV_OUT_NAME haclog_coverage)
			add_custom_target(
				coverage
				COMMAND echo "run converage"
				COMMAND rm -rf ${COV_OUT_NAME}
				COMMAND rm -rf haclog-*profraw
				COMMAND rm -rf haclog.profdata
				COMMAND LLVM_PROFILE_FILE=haclog-%m.profraw make test
				COMMAND ${LLVM_PROFDATA} merge haclog-*.profraw -o haclog.profdata
				COMMAND ${LLVM_COV_PATH} report ./lib/libhaclog.* -instr-profile=haclog.profdata
				COMMAND ${LLVM_COV_PATH} show ./lib/libhaclog.* -instr-profile=haclog.profdata -output-dir=${COV_OUT_NAME} -format=html
				)
		else()
			message(SEND_ERROR "failed generate coverage report, llvm-profdata or llvm-cov not found")
		endif()
	elseif(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
		find_program(LCOV_PATH lcov)
		find_program(GENHTML_PATH genhtml)
		if (LCOV_PATH AND GENHTML_PATH)
			set(COV_OUT_NAME haclog_coverage)
			set(COV_DIR ./CMakeFiles/haclog.dir)
			add_custom_target(
				coverage
				COMMAND echo "run converage"
				COMMAND rm -rf ${COV_OUT_NAME}.info ${COV_OUT_NAME}
				COMMAND ${LCOV_PATH} --zerocounters -d ${COV_DIR}
				COMMAND rm -rf ${COV_OUT_NAME} ${COV_OUT_NAME}.info
				COMMAND make test
				COMMAND ${LCOV_PATH} -d ${COV_DIR} -c -o ${COV_OUT_NAME}.info
				COMMAND ${GENHTML_PATH} ${COV_OUT_NAME}.info -o ${COV_OUT_NAME}
				)
		else()
			message(SEND_ERROR "failed generate coverage report, lcov or genhtml not found")
		endif()
	endif()
endif()

################################
# version
################################

file(STRINGS "version.txt" haclog_version)
string(REPLACE "-" ";" haclog_semver_ext ${haclog_version})
list(GET haclog_semver_ext 0 haclog_semver)
string(REPLACE "." ";" haclog_semver_list ${haclog_semver})

list(GET haclog_semver_list 0 HACLOG_VERSION_MAJOR)
list(GET haclog_semver_list 1 HACLOG_VERSION_MINOR)
list(GET haclog_semver_list 2 HACLOG_VERSION_PATCH)

set(HACLOG_VERSION "${haclog_version}")
set(HACLOG_SOVERSION "${HACLOG_VERSION_MAJOR}")

################################
# output options
################################

message("--------------------------------")
message("# haclog version: ${HACLOG_VERSION}")
message("# haclog c compiler flags: ${CMAKE_C_FLAGS}")
message("# haclog cxx compiler flags: ${CMAKE_CXX_FLAGS}")
message("# haclog CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("# haclog BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message("# haclog BUILD_TESTING: ${BUILD_TESTING}")
message("# haclog HACLOG_BUILD_EXAMPLE: ${HACLOG_BUILD_EXAMPLE}")
message("# haclog HACLOG_BUILD_BENCHMARK: ${HACLOG_BUILD_BENCHMARK}")
message("# haclog HACLOG_BUILD_GBENCHMARK: ${HACLOG_BUILD_GBENCHMARK}")
message("# haclog HACLOG_DOWNLOAD_TEST_LIB: ${HACLOG_DOWNLOAD_TEST_LIB}")
message("# haclog HACLOG_DOWNLOAD_GBENCHMARK_LIB: ${HACLOG_DOWNLOAD_GBENCHMARK_LIB}")
message("# haclog HACLOG_BUILD_SANITIZER: ${HACLOG_BUILD_SANITIZER}")
message("# haclog HACLOG_BUILD_COVERAGE: ${HACLOG_BUILD_COVERAGE}")
message("# haclog HACLOG_LIB_TYPE: ${HACLOG_LIB_TYPE}")
message("# haclog HACLOG_USE_DLL: ${HACLOG_USE_DLL}")
message("--------------------------------")

################################
# configure
################################

if (CMAKE_HOST_UNIX)
	find_package(Backtrace)
	if (Backtrace_FOUND)
		set(HACLOG_HAVE_BACKTRACE 1)
		set(HACLOG_BACKTRACE_HEADER ${Backtrace_HEADER})
	endif()
endif()

set(haclog_src_dir ${CMAKE_CURRENT_LIST_DIR})
set(haclog_gen_dir ${CMAKE_BINARY_DIR}/gen)
configure_file(
	"${haclog_src_dir}/haclog/haclog_config.h.in"
	"${haclog_gen_dir}/haclog/haclog_config.h")

################################
# haclog
################################

set(haclog haclog)

FILE(GLOB_RECURSE haclog_src "${haclog_src_dir}/haclog/*.c")
FILE(GLOB_RECURSE haclog_h "${haclog_src_dir}/haclog/*.h")
FILE(GLOB_RECURSE haclog_gen_h "${haclog_gen_dir}/haclog/*.h")
add_library(${haclog} ${HACLOG_LIB_TYPE}
	${haclog_src}
	${haclog_h}
	${haclog_gen_h})
target_include_directories(${haclog} PUBLIC
	$<BUILD_INTERFACE:${haclog_src_dir}>
	$<BUILD_INTERFACE:${haclog_gen_dir}>
	$<INSTALL_INTERFACE:include>)
target_compile_definitions(${haclog} PRIVATE HACLOG_EXPORTS)
set_target_properties(${haclog} PROPERTIES
	LINKER_LANGUAGE C
	DEBUG_POSTFIX d
	VERSION ${HACLOG_VERSION}
	SOVERSION ${HACLOG_VERSION_MAJOR})
if (NOT ${BUILD_SHARED_LIBS})
	set_target_properties(${haclog} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
	)
endif()

if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
	(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
	target_compile_options(${haclog} PUBLIC -save-temps=obj)
endif()

# threads
find_package(Threads)
if (Threads_FOUND)
	message("-- haclog link threads: ${CMAKE_THREAD_LIBS_INIT}")
	target_link_libraries(${haclog} ${CMAKE_THREAD_LIBS_INIT})
else()
	message(FATAL_ERROR "Failed found threads")
endif()

# backtrace
if (Backtrace_FOUND)
	message("-- haclog link backtrace lib: ${Backtrace_LIBRARIES}")

	target_include_directories(${haclog} PRIVATE ${Backtrace_INCLUDE_DIRS})
	target_link_libraries(${haclog} ${Backtrace_LIBRARIES})
endif()

# sanitizer
if (HACLOG_BUILD_SANITIZER)
	if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
		(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
		target_compile_options(${haclog} PUBLIC
			-fsanitize=undefined
			-fsanitize=address
			-fsanitize=leak)
		target_link_options(${haclog} PUBLIC
			-fsanitize=undefined
			-fsanitize=address
			-fsanitize=leak)
	endif()
endif()

################################
# install
################################

include(GNUInstallDirs)

# force set install libdir to "lib", avoid of being set to "lib64" in some situation
set(CMAKE_INSTALL_LIBDIR "lib")

install(TARGETS ${haclog}
	EXPORT haclogTargets
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY "${haclog_src_dir}/haclog" "${haclog_gen_dir}/haclog"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	FILES_MATCHING
	PATTERN "*.h")

set(haclog_export_dir "${CMAKE_INSTALL_LIBDIR}/cmake/haclog")
install(EXPORT haclogTargets
	FILE haclogTargets.cmake
	DESTINATION ${haclog_export_dir})

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_LIST_DIR}/cmake/haclogConfig.cmake.in"
    "${haclog_gen_dir}/haclogConfig.cmake"
    INSTALL_DESTINATION ${haclog_export_dir}
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(
    "${haclog_gen_dir}/haclogConfigVersion.cmake"
	VERSION ${HACLOG_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES
	"${haclog_gen_dir}/haclogConfig.cmake"
	"${haclog_gen_dir}/haclogConfigVersion.cmake"
	DESTINATION ${haclog_export_dir})

# pkgconfig
configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/cmake/haclog.pc.in"
	"${haclog_gen_dir}/haclog.pc" @ONLY)
install(FILES
	"${haclog_gen_dir}/haclog.pc"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

################################
# examples
################################

function(add_app name folder)
	file(GLOB_RECURSE tmp_h ${folder}/*.h)
	file(GLOB_RECURSE tmp_c ${folder}/*.c)
	file(GLOB_RECURSE tmp_cpp ${folder}/*.cpp)
	file(GLOB_RECURSE tmp_cc ${folder}/*.cc)

	if (MSVC OR MINGW)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "example"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		if (APPLE)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "@executable_path/../lib"
			)
		elseif (UNIX)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "\$ORIGIN/../lib"
			)
		endif()
	endif(MSVC OR MINGW)
	target_include_directories(${name} PUBLIC
		${folder}
		${haclog_src_dir}
		${haclog_gen_dir})
	target_link_libraries(${name} ${haclog})

	if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
		(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
		target_compile_options(${name} PUBLIC -save-temps=obj)
	endif()
endfunction()

function(add_example name folder)
	message("add example ${name} ${folder}")
	set(name example_${name})
	add_app(${name} ${folder})
endfunction()

if (HACLOG_BUILD_EXAMPLE)
	set(example_root_dir ${CMAKE_CURRENT_LIST_DIR}/example)
	FILE(GLOB subdirs RELATIVE ${example_root_dir} ${example_root_dir}/*)
	FOREACH(subdir ${subdirs})
		IF(IS_DIRECTORY ${example_root_dir}/${subdir})
			add_example(${subdir} ${example_root_dir}/${subdir})
		ENDIF()
	ENDFOREACH()

	set(dev_example_root_dir ${CMAKE_CURRENT_LIST_DIR}/dev_test)
	FILE(GLOB subdirs RELATIVE ${dev_example_root_dir} ${dev_example_root_dir}/*)
	FOREACH(subdir ${subdirs})
		IF(IS_DIRECTORY ${dev_example_root_dir}/${subdir})
			add_example(${subdir} ${dev_example_root_dir}/${subdir})
		ENDIF()
	ENDFOREACH()
endif()

################################
# benchmark
################################

function(add_benchmark name folder)
	message("add benchmark ${name} ${folder}")
	set(name benchmark_${name})
	add_app(${name} ${folder})
endfunction()

if (HACLOG_BUILD_BENCHMARK)
	set(benchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/benchmark)
	FILE(GLOB subdirs RELATIVE ${benchmark_root_dir} ${benchmark_root_dir}/*)
	FOREACH(subdir ${subdirs})
		IF(IS_DIRECTORY ${benchmark_root_dir}/${subdir})
			add_benchmark(${subdir} ${benchmark_root_dir}/${subdir})
		ENDIF()
	ENDFOREACH()
endif()


################################
# deps
################################
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

################################
# gbenchmark
################################

function(add_gbenchmark name folder)
	message("add gbenchmark ${name} ${folder}")
	set(name gbenchmark_${name})

	add_app(${name} ${folder})
	target_link_libraries(${name} benchmark)
	target_include_directories(${name} PRIVATE
		${FETCHCONTENT_BASE_DIR}/benchmark-src)
	if (HACLOG_DOWNLOAD_GBENCHMARK_LIB)
		add_dependencies(${name} benchmark)
	endif()
endfunction()

if (HACLOG_BUILD_GBENCHMARK)
	if (HACLOG_DOWNLOAD_GBENCHMARK_LIB)
		message("-- fetch google benchmark")
		set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
		set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
		FetchContent_Declare(
			benchmark
			GIT_REPOSITORY https://github.com/google/benchmark.git
			GIT_TAG v1.8.3
			GIT_SHALLOW TRUE
			GIT_PROGRESS TRUE
			)
		FetchContent_MakeAvailable(benchmark)
	else()
		find_package(benchmark)
		if (NOT benchmark_FOUND)
			message(FATAL_ERROR "Failed found google benchmark")
		endif()
		message("find google benchmark: ${benchmark_DIR}")
	endif()

	set(gbenchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/gbenchmark)
	FILE(GLOB subdirs RELATIVE ${gbenchmark_root_dir} ${gbenchmark_root_dir}/*)
	FOREACH(subdir ${subdirs})
		IF(IS_DIRECTORY ${gbenchmark_root_dir}/${subdir})
			add_gbenchmark(${subdir} ${gbenchmark_root_dir}/${subdir})
		ENDIF()
	ENDFOREACH()
endif()

################################
# tests
################################

if (BUILD_TESTING)
	if (HACLOG_DOWNLOAD_TEST_LIB)
		message("-- fetch Unity Test")
		FetchContent_Declare(
			unity
			GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
			GIT_TAG v2.5.2
			GIT_SHALLOW TRUE
			)
		FetchContent_MakeAvailable(unity)
	else()
		find_package(unity)
		if (NOT unity_FOUND)
			message(FATAL_ERROR "Failed found Unity Test")
		endif()
		message("find Unity Test: ${unity_DIR}")
	endif()
endif()

function(add_unittest name folder)
	message("add test ${name} ${folder}")
	set(name test_${name})

	file(GLOB_RECURSE tmp_h ${folder}/*.h)
	file(GLOB_RECURSE tmp_c ${folder}/*.c)
	file(GLOB_RECURSE tmp_cpp ${folder}/*.cpp)
	file(GLOB_RECURSE tmp_cc ${folder}/*.cc)

	if (MSVC OR MINGW)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "example"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		if (APPLE)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "@executable_path/../lib"
			)
		elseif (UNIX)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "\$ORIGIN/../lib"
			)
		endif()
	endif(MSVC OR MINGW)
	add_dependencies(${name} ${haclog})
	if (HACLOG_DOWNLOAD_TEST_LIB)
		add_dependencies(${name} unity::framework)
	else()
		target_include_directories(${name} PUBLIC
			${FETCHCONTENT_BASE_DIR}/unity-src/src)
	endif()
	target_link_libraries(${name} ${haclog} unity::framework)

	add_test(NAME ${name} COMMAND ${name})
endfunction()

if (BUILD_TESTING)
	#enable_testing()
	include(CTest)

	set(test_root_dir ${CMAKE_CURRENT_LIST_DIR}/test)
	FILE(GLOB subdirs RELATIVE ${test_root_dir} ${test_root_dir}/*)
	FOREACH(subdir ${subdirs})
		IF(IS_DIRECTORY ${test_root_dir}/${subdir})
			add_unittest(${subdir} ${test_root_dir}/${subdir})
		ENDIF()
	ENDFOREACH()
endif()
