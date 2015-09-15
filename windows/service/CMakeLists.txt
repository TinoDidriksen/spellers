add_executable(service
	service.cpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	)

add_executable(tester
	tester.cpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	)
