add_link_options(-lpqxx -lpq)

set(source_files
    model/database.cc
)

set(header_files
    model/database.h
)
build_lib(
  LIBNAME database
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
)
