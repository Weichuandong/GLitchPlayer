# 在根目录执行

if [ -z "$BUILD_DIR"  ];then
    BUILD_DIR=mac_build
    mkdir $BUILD_DIR
fi

conan install . --output-folder=${BUILD_DIR} --build=missing --settings=build_type=RelWithDebInfo
cmake -S . -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=${BUILD_DIR}/conan_toolchain.cmake
cmake --build ${BUILD_DIR} -- -j 8