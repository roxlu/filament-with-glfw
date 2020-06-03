# ----------------------------------------------------

include(ExternalProject)

# ----------------------------------------------------

if (UNIX)
  set(glfw_lib_file lib/libglfw3.a)
elseif(WIN32)
  set(glfw_lib_file lib/glfw3.lib)
endif()

# ----------------------------------------------------

ExternalProject_Add(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_SHALLOW 1
  UPDATE_COMMAND ""
  CMAKE_ARGS
    -DGLFW_BUILD_TESTS=Off
    -DGLFW_BUILD_DOCS=Off
    -DGLFW_BUILD_EXAMPLES=Off
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DUSE_MSVC_RUNTIME_LIBRARY_DLL=On
  BUILD_BYPRODUCTS <INSTALL_DIR>/${glfw_lib_file}
  )

# ----------------------------------------------------

ExternalProject_Get_Property(glfw install_dir)
include_directories(${install_dir}/include)

# ----------------------------------------------------

list(APPEND poly_deps glfw)

if (UNIX)
  list(APPEND poly_libs
    X11
    dl
    Xxf86vm
    Xrandr
    pthread
    dl
    Xi
    Xinerama
    Xcursor
    ${install_dir}/${glfw_lib_file}
    )
endif()

if(WIN32)
  list(APPEND poly_libs
    ${install_dir}/${glfw_lib_file}
    Opengl32.lib
    )
endif()

# ----------------------------------------------------
