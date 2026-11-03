cmake_minimum_required(VERSION 3.10)
project(NativeFuzzme)

include_directories(AFTER ${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${CMAKE_SOURCE_DIR}/../lib/arm64-v8a)


add_executable(fuzz "fuzz.c"
  "object_recipes/DummyOpenArchiveCallback/construct.c"
  "object_recipes/RandomAccessFileInStream/construct.c"
  "object_recipes/SequentialOutStream/construct.c"
  "object_recipes/ExtractSlowCallback/construct.c")
set_property(TARGET fuzz APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-rpath=$ORIGIN -Wl,-rpath=$ORIGIN/lib")
target_link_libraries(fuzz jenv 7-Zip-JBinding)
