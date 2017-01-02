if(NOT MSVC)
	include_directories("${CMAKE_SOURCE_DIR}/include/mingw")
endif()

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
	${CMAKE_SOURCE_DIR}/include/COM.hpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	Speller.def
	)
target_link_libraries(speller-win8 ${HFSTOSPELL_LIBRARIES})

add_executable(test-speller
	test-speller.cpp
	ClassFactory.cpp
	DLL.cpp
	Speller.cpp
	SpellerFactory.cpp

	ClassFactory.hpp
	DLL.hpp
	EnumString.hpp
	Speller.hpp
	SpellerFactory.hpp
	${CMAKE_SOURCE_DIR}/include/COM.hpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	)
target_link_libraries(test-speller ${HFSTOSPELL_LIBRARIES})

install(TARGETS speller-win8
	ARCHIVE DESTINATION "${INST_LIBDIR}/${CMAKE_LIBRARY_ARCHITECTURE}"
	LIBRARY DESTINATION "${INST_LIBDIR}/${CMAKE_LIBRARY_ARCHITECTURE}"
	RUNTIME DESTINATION bin)
