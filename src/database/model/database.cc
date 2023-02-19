#include "database.h"
#include <pqxx/pqxx>
using namespace pqxx;

database::database(){
    std::cout << "Constructor" << std::endl;
}

bool database::connect(){
   try {
      connection C("dbname = testdb user = postgres password = password \
      hostaddr = 127.0.0.1 port = 5432");
      if (C.is_open()) {
         std::cout << "Opened database successfully: " << C.dbname() << std::endl;
      } else {
         std::cout << "Can't open database" << std::endl;
         return false;
      }
      C.close ();
   } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
   }
}
