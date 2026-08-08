FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG        master
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(implot)

if(NOT TARGET implot)
  add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
  )

  target_include_directories(implot PUBLIC
    ${implot_SOURCE_DIR}
  )

  target_link_libraries(implot PUBLIC imgui)
endif()
