# ----------------------------------------------------
#
# We use my `feature-get-texture-id` branch which adds
# a function that allows us to get the native (OpenGL)
# texture handle.
#
# ----------------------------------------------------

ExternalProject_Add(
  filament
  GIT_REPOSITORY git@github.com:roxlu/filament.git
  GIT_TAG rox/feature-get-texture-id
  CONFIGURE_COMMAND cmake
  -G Ninja
  -DCMAKE_BUILD_TYPE=Release 
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_LINKER=${CMAKE_LINKER}
  -DFILAMENT_ENABLE_JAVA=Off
  -DFILAMENT_SKIP_EXAMPLES=On
  -DUSE_STATIC_CRT=Off
  <SOURCE_DIR>
  BUILD_COMMAND cd <BINARY_DIR> && ninja
  INSTALL_COMMAND cd <BINARY_DIR> && ninja install
  BUILD_BYPRODUCTS <INSTALL_DIR>/lib/x86_64/backend.lib
  UPDATE_COMMAND ""
  )

# ----------------------------------------------------

ExternalProject_Get_Property(filament install_dir)
set(lib_dir ${install_dir}/lib/x86_64/)
set(inc_dir ${install_dir}/include)
include_directories(${inc_dir})

list(APPEND poly_libs
  ${lib_dir}/backend.lib
  ${lib_dir}/bluegl.lib
  ${lib_dir}/camutils.lib
  ${lib_dir}/dracodec.lib
  ${lib_dir}/filabridge.lib
  ${lib_dir}/filaflat.lib
  ${lib_dir}/filamat.lib
  ${lib_dir}/filamat_lite.lib
  ${lib_dir}/filament.lib
  ${lib_dir}/filameshio.lib
  ${lib_dir}/geometry.lib
  ${lib_dir}/gltfio_core.lib
  ${lib_dir}/gltfio.lib
  ${lib_dir}/ibl.lib
  ${lib_dir}/image.lib
  ${lib_dir}/matdbg.lib
  ${lib_dir}/meshoptimizer.lib
  ${lib_dir}/shaders.lib
  ${lib_dir}/smol-v.lib
  ${lib_dir}/utils.lib

  Shlwapi.lib
  )

list(APPEND poly_deps filament)

if (EXISTS ${lib_dir}/backend.lib)
  return()
endif()

# ----------------------------------------------------
