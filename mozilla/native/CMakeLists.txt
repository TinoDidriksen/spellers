add_definitions(-DDLL_EXPORTS)

add_library(shim SHARED
	shim.cpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	)
