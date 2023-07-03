include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_library(Core STATIC 
	Board.cpp Board.h
        ColorAxisCounts.cpp ColorAxisCounts.h
        Defs.cpp Defs.h
        MpfWrapper.cpp MpfWrapper.h
        PuzzleDef.cpp PuzzleDef.h
        Stats.cpp Stats.h
)

target_link_libraries(Core ${CONAN_LIBS})
