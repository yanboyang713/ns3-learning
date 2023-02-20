#include "database.h"
#include <cstring>
#include <pqxx/pqxx>
using namespace pqxx;

database::database():
   dbname("anomalyprediction"),
   userName("postgres"),
   password("password"),
   hostAddress("127.0.0.1"),
   port("5432")

{
    std::cout << "Constructor Called" << std::endl;
    //bool databaseConnect = connect();
}

database::database(std::string url) {

}

database::~database(){
    std::cout << "De-constructor Called" << std::endl;

}

bool database::connect(){
   /*
   pqxx::connection C{"postgresql://postgres:password@127.0.0.1:5432/anomalyprediction"};
   //worked
   //connection C("dbname = anomalyprediction user = postgres password = password \
   //hostaddr = 127.0.0.1 port = 5432");
   if (C.is_open()) {
      std::cout << "Opened database successfully: " << C.dbname() << std::endl;
   } else {
      std::cout << "Can't open database" << std::endl;
      return false;
   }
   C.close ();
       */

   const std::string connectString = "dbname=" + dbname + " user=" + userName + " password=" + password + " hostaddr=" + hostAddress + " port=" + port;
   std::cout << "connectString: " << connectString << std::endl;

   //std::cout << connectString.size() << std::endl;
   try {
      connection C("dbname=" + dbname + " user=" + userName + " password=" + password + " hostaddr=" + hostAddress + " port=" + port);
      //connection C(connectString);

      //pqxx::connection C{"postgresql://postgres:password@127.0.0.1:5432/anomalyprediction"};
      //worked
      //connection C("dbname = anomalyprediction user = postgres password = password \
      //hostaddr = 127.0.0.1 port = 5432");
      if (C.is_open()) {
         std::cout << "Opened database successfully: " << C.dbname() << std::endl;
      } else {
         std::cout << "Can't open database" << std::endl;
         return false;
      }
      C.close ();
   } catch (const std::exception &e) {
      std::cerr << "database cerr: " << e.what() << std::endl;
      return false;
   }
   return true;
}
