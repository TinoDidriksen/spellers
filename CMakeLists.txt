set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(spellers CXX C)

# Release or Debug
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc /MP")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Ox /Ot /GL /GS-")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
	set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
	# When building on VS 2015, this ensures we use the WinXP compatible toolchain
	set(CMAKE_GENERATOR_TOOLSET "v140_xp")
else()
	set(_FLAGS_COMMON "-Wall -Wextra -Wno-missing-field-initializers -Wno-deprecated -Wno-unused-result -Wno-unused-value -Wno-unused-parameter -fPIC -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_FLAGS_COMMON}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_FLAGS_COMMON}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

	# Enable latest possible C++ standard
	include(CheckCXXCompilerFlag)
	foreach(flag "-std=c++17" "-std=c++1z" "-std=c++14" "-std=c++1y" "-std=c++11")
		string(REGEX REPLACE "[^a-z0-9]" "-" _flag ${flag})
		CHECK_CXX_COMPILER_FLAG(${flag} COMPILER_SUPPORTS_${_flag})
		if(COMPILER_SUPPORTS_${_flag})
			message(STATUS "Enabling ${flag}")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
			break()
		endif()
	endforeach()

	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	endif()
endif()

if(WIN32)
	add_definitions(-DUNICODE -D_UNICODE -D_SECURE_SCL=0 -D_ITERATOR_DEBUG_LEVEL=0 -D_CRT_SECURE_NO_DEPRECATE -DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DNOMINMAX)
	# 0x0501 means target Windows XP
	add_definitions(-DWINVER=0x0501 -D_WIN32_WINNT=0x0501)
else()
	set(POPEN_PLUS_C "${CMAKE_CURRENT_SOURCE_DIR}/include/posix/popen_plus.c" "${CMAKE_CURRENT_SOURCE_DIR}/include/posix/popen_plus.h")
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/posix")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

include(FindPkgConfig)
pkg_check_modules(HFSTOSPELL REQUIRED hfstospell)
link_directories(${HFSTOSPELL_LIBRARY_DIRS})
include_directories(${HFSTOSPELL_INCLUDE_DIRS})

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/msoffice")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mozilla/native")

if(WIN32)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/windows")
endif()
