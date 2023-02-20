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
    bool databaseConnect = connect();
}

database::database(std::string url) {

}

database::~database(){
    std::cout << "De-constructor Called" << std::endl;

}

bool database::connect(){

   //const std::string connectString = "dbname=" + dbname + " user=" + userName + " password=" + password + " hostaddr=" + hostAddress + " port=" + port;
   //std::cout << "connectString: " << connectString << std::endl;

   //std::cout << connectString.size() << std::endl;
   try {
      //connection C("dbname=" + dbname + " user=" + userName + " password=" + password + " hostaddr=" + hostAddress + " port=" + port);
      //connection C(connectString);
      //worked
      connection C("dbname = anomalyprediction user = postgres password = password \
      hostaddr = 127.0.0.1 port = 5432");
      if (C.is_open()) {
         std::cout << "Opened database successfully: " << C.dbname() << std::endl;
      } else {
         std::cout << "Can't open database" << std::endl;
         return false;
      }

      /*
      // Create SQL statement
      sql = "CREATE TABLE COMPANY("  \
      "ID INT PRIMARY KEY     NOT NULL," \
      "NAME           TEXT    NOT NULL," \
      "AGE            INT     NOT NULL," \
      "ADDRESS        CHAR(50)," \
      "SALARY         REAL );";

      // Create a transactional object
      work W(C);

      // Execute SQL query
      W.exec( sql );
      W.commit();
      cout << "Table created successfully" << endl;
      */
      C.close ();
   } catch (const std::exception &e) {
      std::cerr << "database cerr: " << e.what() << std::endl;
      return false;
   }
}
