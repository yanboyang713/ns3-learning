set(source_files
    model/sha256.cc
)

set(header_files
    model/sha256.h
)
build_lib(
  LIBNAME sha256
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
)
