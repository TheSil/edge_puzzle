include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(BacktrackerFixedPath 
	Backtracker.cpp Backtracker.h
	main.cpp
	Stack.cpp Stack.h
)

include_directories(${CMAKE_SOURCE_DIR}/Core)

target_link_libraries(BacktrackerFixedPath Core)
target_link_libraries(BacktrackerFixedPath ${CONAN_LIBS})

