add_definitions(-DSHIM_EXPORTS)

add_library(shim SHARED
	shim.cpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	${POPEN_PLUS_C}
	)
target_link_libraries(shim ${CMAKE_DL_LIBS})

install(TARGETS shim
	ARCHIVE DESTINATION "${INST_LIBDIR}/${CMAKE_LIBRARY_ARCHITECTURE}"
	LIBRARY DESTINATION "${INST_LIBDIR}/${CMAKE_LIBRARY_ARCHITECTURE}"
	RUNTIME DESTINATION bin)
