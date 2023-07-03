include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(Swapper 
	Swapper.cpp Swapper.h
	main.cpp
)


include_directories(${CMAKE_SOURCE_DIR}/Core)

target_link_libraries(Swapper Core)
target_link_libraries(Swapper ${CONAN_LIBS})

