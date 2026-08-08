set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

# Enable AVX & SSE2 SIMD optimizations for FFTW3
set(ENABLE_SSE2 ON CACHE BOOL "Enable SSE2 optimizations" FORCE)
set(ENABLE_AVX ON CACHE BOOL "Enable AVX optimizations" FORCE)

FetchContent_Declare(
  fftw3
  URL https://fftw.org/fftw-3.3.11.tar.gz
)

FetchContent_MakeAvailable(fftw3)
