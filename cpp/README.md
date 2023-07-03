Make solution for VS 2022:

    conan install . --install-folder build --build=missing
    cmake . -G "Visual Studio 17 2022" -B build

Solution file generated in build/EdgePuzzle.sln.

