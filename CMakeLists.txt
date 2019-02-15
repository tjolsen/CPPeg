cmake_minimum_required(VERSION 3.8)
project(CPPeg)

set(CMAKE_CXX_STANDARD 17)

find_package(tmpl REQUIRED)


set(SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/include/cppeg.hpp
  ${CMAKE_SOURCE_DIR}/include/cppeg_common.hpp
  ${CMAKE_SOURCE_DIR}/include/input_stream.hpp
  ${CMAKE_SOURCE_DIR}/include/rule.hpp
  )

add_library(cppeg INTERFACE)
target_sources(cppeg INTERFACE $<BUILD_INTERFACE:${SOURCE_FILES}>)
target_link_libraries(cppeg INTERFACE tmpl::tmpl)


target_include_directories(cppeg INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/cppeg>)

install(TARGETS cppeg EXPORT cppegConfig)
install(
  DIRECTORY include/
  DESTINATION include/cppeg
  FILES_MATCHING PATTERN "*.hpp")

install(
  EXPORT cppegConfig
  DESTINATION share/cppeg/cmake
  NAMESPACE cppeg::)

#------------------------------------------------------------
# Unittests
#------------------------------------------------------------
option(BUILD_TESTS "Build cppeg unittests" ON)
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
