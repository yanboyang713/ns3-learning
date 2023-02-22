include(FetchContent)

cmake_minimum_required (VERSION 3.10)
project(example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
  libpqxx
  GIT_REPOSITORY https://github.com/jtv/libpqxx.git
  #7.5.3
  GIT_TAG 7f3be09be1ab89351e7d7767de9a6bc6b1947761
  #7.6.1
  #GIT_TAG fca99f26b7abe9f1856d8e26d417949fdb81aadf
  #7.7.0
  #GIT_TAG 9a3a719952416e546ddf06229a61b01b8bb5bb15
  #7.7.2
  #GIT_TAG 07cca8df1b8029278e29b51eaee52ce6bde0f82a
  #7.7.3
  #GIT_TAG 90768b07f7feb55a9bd70cfaacb5543bfd074022
  #7.7.4
  #GIT_TAG 17e5a6c8ac3abc05329891aaa378bd6004b9c8ee
  # Current Ver
  #GIT_TAG 3d97c80bcde96fb70a21c1ae1cf92ad934818210
)
set(PQXX_LIBRARIES pqxx)

FetchContent_MakeAvailable(libpqxx)

set(source_files
    model/database.cc
)

set(header_files
    model/database.h
)

build_lib(
  add_link_options(-lpqxx -lpq)

  #add_link_options("${PQXX_LIBRARIES}")
  #add_subdirectory(deps/libpqxx build-pqxx)
  LIBNAME database
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK ${PQXX_LIBRARIES}
  #LIBRARIES_TO_LINK pqxx pq
)
