set(CMAKE_CXX_STANDARD 17)

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
  LIBRARIES_TO_LINK pqxx pq
)
