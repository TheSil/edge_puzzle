include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(Backtracker 
	Backtracker.cpp Backtracker.h
	main.cpp
	Stack.cpp Stack.h
)

include_directories(${CMAKE_SOURCE_DIR}/Core)

target_link_libraries(Backtracker Core)
target_link_libraries(Backtracker ${CONAN_LIBS})

