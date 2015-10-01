add_definitions(-DDLL_EXPORTS)

add_library(shim SHARED
	shim.cpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	${POPEN_PLUS_C}
	)
