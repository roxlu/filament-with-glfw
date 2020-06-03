# ----------------------------------------------------

include(ExternalProject)
 
# ----------------------------------------------------

ExternalProject_Add(
  filament
  GIT_REPOSITORY git@github.com:roxlu/filament.git
  GIT_TAG rox/feature-get-texture-id
  CONFIGURE_COMMAND cmake
  -G Ninja
  -DCMAKE_BUILD_TYPE=Release 
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DFILAMENT_ENABLE_JAVA=Off
  <SOURCE_DIR>
  BUILD_COMMAND cd <BINARY_DIR> && ninja
  INSTALL_COMMAND cd <BINARY_DIR> && ninja install
  )

# ----------------------------------------------------

ExternalProject_Get_Property(filament install_dir)
include_directories(${install_dir}/include)
list(APPEND poly_deps filament)

list(APPEND poly_libs
  ${install_dir}/lib/x86_64/libbackend.a
  ${install_dir}/lib/x86_64/libbluegl.a
  ${install_dir}/lib/x86_64/libbluevk.a
  ${install_dir}/lib/x86_64/libfilabridge.a
  ${install_dir}/lib/x86_64/libfilaflat.a
  ${install_dir}/lib/x86_64/libfilamat.a
  ${install_dir}/lib/x86_64/libfilamat_lite.a
  ${install_dir}/lib/x86_64/libfilament.a
  ${install_dir}/lib/x86_64/libfilameshio.a
  ${install_dir}/lib/x86_64/libgeometry.a
  ${install_dir}/lib/x86_64/libgltfio.a
  ${install_dir}/lib/x86_64/libgltfio_core.a
  ${install_dir}/lib/x86_64/libibl.a
  ${install_dir}/lib/x86_64/libimage.a
  ${install_dir}/lib/x86_64/libmatdbg.a
  ${install_dir}/lib/x86_64/libmeshoptimizer.a
  ${install_dir}/lib/x86_64/libshaders.a
  ${install_dir}/lib/x86_64/libsmol-v.a
  ${install_dir}/lib/x86_64/libutils.a
  )

# ----------------------------------------------------
