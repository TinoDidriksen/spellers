add_executable(service
	service.cpp
	ServiceFactory.cpp

	service.hpp
	ServiceFactory.hpp
	${CMAKE_SOURCE_DIR}/include/COM.hpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	)

add_executable(tester
	tester.cpp
	${CMAKE_SOURCE_DIR}/include/COM.hpp
	${CMAKE_SOURCE_DIR}/include/debugp.hpp
	${CMAKE_SOURCE_DIR}/include/shared.hpp
	)
