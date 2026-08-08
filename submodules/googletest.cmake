set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  googletest,
  GIT_REPOSITORY https://github.com/google/googletest.git,
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(googletest)