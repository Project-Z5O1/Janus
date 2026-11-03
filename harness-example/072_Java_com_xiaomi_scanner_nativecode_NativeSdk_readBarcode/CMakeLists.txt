cmake_minimum_required(VERSION 3.10)
project(NativeFuzzme)

include_directories(AFTER ${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)


add_executable(fuzz
  "fuzz.c"
  "object_recipes/android.graphics.Bitmap/construct.c"
  "object_recipes/com.xiaomi.scanner.nativecode.NativeSdk/construct.c"
)
set_property(TARGET fuzz APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-rpath=$ORIGIN -Wl,-rpath=$ORIGIN/lib")
target_link_libraries(fuzz jenv log)
