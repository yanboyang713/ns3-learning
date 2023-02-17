find_package(libpqxx REQUIRED)
#add_subdirectory(deps/libpqxx build-pqxx)

if ( LIBPQXX_FOUND )
  message ( "PQXX Found" )
else ()
  message( "PQXX NOT FOUND" )
endif ()
