cmake_minimum_required(VERSION 3.7)
project(filamentsharedgl C CXX)
set(CMAKE_CXX_STANDARD 14)

# ----------------------------------------------------

set(base_dir ${CMAKE_CURRENT_LIST_DIR}/../)
set(ext_dir ${CMAKE_CURRENT_LIST_DIR}/../extern)
set(src_dir ${CMAKE_CURRENT_LIST_DIR}/../src)
set(inc_dir ${CMAKE_CURRENT_LIST_DIR}/../include)
set(inst_dir ${CMAKE_INSTALL_PREFIX})

# ----------------------------------------------------

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
if (${build_type} MATCHES "debug")
  set(debug_flag "-debug")
endif()

# ----------------------------------------------------

include(glfw.cmake)
include(filament.cmake)
include(glad.cmake)

# ----------------------------------------------------

add_library(poly${debug_flag} STATIC ${poly_sources})
add_dependencies(poly${debug_flag} ${poly_deps})

# ----------------------------------------------------

macro(create_test name)
  
  set(test_name "test-${name}${debug_flag}")
  add_executable(${test_name} ${src_dir}/test/test-${name}.cpp)
  add_dependencies(${test_name} ${poly_deps})
  target_link_libraries(${test_name} poly${debug_flag} ${poly_libs})
  install(TARGETS ${test_name} DESTINATION bin/)
  set_property(TARGET ${test_name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  
endmacro()

# ----------------------------------------------------

create_test("compile")
create_test("shared-gl-context")
create_test("shared-gl-context-with-fbo") 

# ----------------------------------------------------
