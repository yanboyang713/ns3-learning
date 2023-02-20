include(FetchContent)

cmake_minimum_required (VERSION 3.10)
project(example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
  libpqxx
  GIT_REPOSITORY https://github.com/jtv/libpqxx.git
  GIT_TAG 3d97c80bcde96fb70a21c1ae1cf92ad934818210
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
  #add_link_options(-lpqxx -lpq)

  LIBNAME database
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK ${PQXX_LIBRARIES} pq
  #LIBRARIES_TO_LINK pqxx pq
)
