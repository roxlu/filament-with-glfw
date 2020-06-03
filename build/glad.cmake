# ----------------------------------------------------

set(ext_dir ${CMAKE_CURRENT_LIST_DIR}/../extern)

# ----------------------------------------------------

include_directories(${ext_dir}/glad/include)
list(APPEND poly_sources  ${ext_dir}/glad/src/glad.c)

# ----------------------------------------------------

