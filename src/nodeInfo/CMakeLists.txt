set(source_files
    model/nodeInfo.cc
)

set(header_files
    model/nodeInfo.h
)

build_lib(
  LIBNAME nodeInfo
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  #LIBRARIES_TO_LINK ${PQXX_LIBRARIES}
)
