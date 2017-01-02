# This part is only for Windows 8 and newer
remove_definitions(-DWINVER=0x0501 -D_WIN32_WINNT=0x0501)
add_definitions(-DWINVER=0x0602 -D_WIN32_WINNT=0x0602)

add_executable(resolve-locale
	resolve-locale.cpp
	)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/win8")
