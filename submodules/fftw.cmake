set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

FetchContent_Declare(
  fftw3
  GIT_REPOSITORY https://github.com/FFTW/fftw3.git
  GIT_TAG        fftw-3.3.10
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(fftw3)
