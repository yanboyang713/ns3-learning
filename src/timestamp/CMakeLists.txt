set(source_files
    model/timestamp.cc
)

set(header_files
    model/timestamp.h
)

build_lib(
  LIBNAME timestamp
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK ${libcore}
)
