add_subdirectory(buffer)
add_subdirectory(storage)

add_library(CSC443 STATIC ${ALL_OBJECT_FILES})

set(CSC443_LIBS
      csc443_buffer
      csc443_storage
    )
set(CSC443_THIRDPARTY_LIBS
      csc443_murmur3
      Threads::Threads
      libfort::fort
    )
target_link_libraries(
      CSC443
      ${CSC443_LIBS}
      ${CSC443_THIRDPARTY_LIBS}
)

target_include_directories(
  csc443 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)