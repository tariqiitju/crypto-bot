conan search cpprestsdk --remote=conancenter
conan inspect cpprestsdk/2.10.18


    conan profile new default --detect
    conan profile update settings.compiler.libcxx=libstdc++11 default 

mkdir build && cd build
conan install ..
conan install .. --build=missing


(linux, mac)
$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
$ cmake --build .