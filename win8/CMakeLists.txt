# This part is only for Windows 8 and newer
add_definitions(-DWINVER=0x0602 -D_WIN32_WINNT=0x0602)

add_definitions(-DDLL_EXPORTS)

add_library(speller-win8 SHARED
	ClassFactory.cpp
	DLL.cpp
	Speller.cpp
	SpellerFactory.cpp

	ClassFactory.hpp
	DLL.hpp
	EnumString.hpp
	Speller.hpp
	SpellerFactory.hpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp

	Speller.def
	)
