# Enable Tracy profiling automatically when build type is RelWithDebInfo
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy Profiling" FORCE)
else()
    set(TRACY_ENABLE OFF CACHE BOOL "Enable Tracy Profiling" FORCE)
endif()

FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG        v0.11.1
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(tracy)
