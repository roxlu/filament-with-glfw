#!/bin/bash

# ----------------------------------------------------

curr_dir=${PWD}
base_dir=${curr_dir}/..
src_dir=${base_dir}/src/
install_dir=${base_dir}/install/
extern_dir=${base_dir}/extern

# ----------------------------------------------------

is_debug="n"
build_dir="build_unix"
cmake_build_type="Release"
cmake_config="Release"
debug_flag=""
debugger=""
os_debugger=""
parallel_builds=""
cmake_generator="Unix Makefiles"
profile_dir=""
profile_enable="Off"

# ----------------------------------------------------

for var in "$@"
do
    if [ "${var}" = "debug" ] ; then
        is_debug="y"
        cmake_build_type="Debug"
        cmake_config="Debug"
        debug_flag="-debug"
        debugger="gdb --args"
        arg="debug"
    elif [ "${var}" = "profile" ] ; then
        profile_dir=".profile"
        profile_enable="On"
    fi
done

# ----------------------------------------------------

# Make sure CMake uses clang as the compiler.
if [ -f /usr/bin/clang ] ; then 
    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++
    export CXXFLAGS="--stdlib=libc++"
fi

# Create unique name for this build type.
build_dir="${curr_dir}/${build_dir}.${cmake_build_type}${profile_dir}"

if [ ! -d ${build_dir} ] ; then 
    mkdir ${build_dir}
fi

# ----------------------------------------------------

# Compile the library.
cd ${build_dir}
cmake -DCMAKE_INSTALL_PREFIX=${install_dir} \
      -DCMAKE_BUILD_TYPE=${cmake_build_type} \
      -DCMAKE_VERBOSE_MAKEFILE=On \
      -DCMAKE_C_COMPILER=${CC} \
      -DCMAKE_CXX_COMPILER=${CXX} \
      -DCMAKE_CXX_FLAGS=${CXXFLAGS} \
      -DENABLE_PROFILING=${profile_enable} \
      -G "${cmake_generator}" \
      ..


if [ $? -ne 0 ] ; then
    echo "Failed to configure"
    exit
fi

# ----------------------------------------------------

cmake --build . \
      --target install \
      --config ${cmake_build_type} 

if [ $? -ne 0 ] ; then
    echo "Failed to build"
    exit
fi

# ----------------------------------------------------

cd ${install_dir}/bin

#${debugger} ./test-compile${debug_flag}
#${debugger} ./test-shared-gl-context${debug_flag}
${debugger} ./test-shared-gl-context-with-fbo${debug_flag}

# ----------------------------------------------------
