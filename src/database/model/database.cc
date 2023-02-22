#include "database.h"
//#include <cstring>

using namespace pqxx;

database::database():
   dbname("anomalyprediction"),
   userName("postgres"),
   password("password"),
   hostAddress("127.0.0.1"),
   port("5432")

{
    std::cout << "Constructor Called" << std::endl;
    this->url = "dbname=" + dbname + " user=" + userName + " password=" + password + " hostaddr=" + hostAddress + " port=" + port;

    conn = new pqxx::connection(this->url);
}

database::database(std::string dbname, std::string userName, std::string password, std::string hostAddress, std::string port):
   dbname("anomalyprediction"),
   userName("postgres"),
   password("password"),
   hostAddress("127.0.0.1"),
   port("5432")

{
   std::cout << "Constructor with Args Called" << std::endl;

   this->dbname = dbname;
   this->userName = userName;
   this->password = password;
   this->hostAddress = hostAddress;
   this->port = port;

   this->url = "dbname=" + dbname + " user=" + userName + " password=" + password + " hostaddr=" + hostAddress + " port=" + port;

   conn = new pqxx::connection(this->url);

}

database::database(std::string url) {
    this->url = url;
    conn = new pqxx::connection(url);
}

database::~database(){
    std::cout << "De-constructor Called" << std::endl;

    conn->close ();
    if (conn != nullptr) {
        delete conn;
        conn = nullptr;
    }
}

pqxx::connection * database::getConnection() {
    //if its not open
    if ( not conn->is_open() ){
       conn = new pqxx::connection(this->url);
    } else {
       std::cout << "Opened database successfully: " << conn->dbname() << std::endl;
    }
    return conn;
}

bool database::connect(){

   try {
      connection databaseConnector("dbname=" + dbname + " user=" + userName + " password=" + password + " hostaddr=" + hostAddress + " port=" + port);
      //Also could be: pqxx::connection C{"postgresql://postgres:password@127.0.0.1:5432/anomalyprediction"};

      if ( databaseConnector.is_open()) {
         std::cout << "Opened database successfully: " <<  databaseConnector.dbname() << std::endl;
      } else {
         std::cout << "Can't open database" << std::endl;
         return false;
      }
      databaseConnector.close ();
   } catch (const std::exception &e) {
      std::cerr << "database cerr: " << e.what() << std::endl;
      return false;
   }
   return true;
}

bool database::runRecord (std::string runID, char hostname[1024], std::string dataMode, std::string controlMode, uint32_t numOfNodes,
                          double totalTime, double step, uint32_t payloadSize, std::string dataRate, uint16_t ChannelNumber,
                          double frequency, std::string standard, uint16_t channelWidth, uint16_t guardIntervalNs){

   /*
   //Print agus out
   std::cout << "RunID: " << runID << std::endl;
   std::cout << "Hostname: " << hostname << std::endl;
   std::cout << "Data Mode: " << dataMode << std::endl;
   std::cout << "Control Mode: " << controlMode << std::endl;
   std::cout << "Num Of Nodes: " << numOfNodes<< std::endl;
   std::cout << "Simulation time: " << totalTime << std::endl;
   std::cout << "Grid step (m): " << step << std::endl;
   std::cout << "payloadSize (bytes)" << payloadSize << std::endl;
   std::cout << "dataRate: " << dataRate << std::endl;
   std::cout << "ChannelNumber: " << ChannelNumber << std::endl;
   std::cout << "frequency: " << frequency << std::endl;
   std::cout << "WIFI Standard: " << standard << std::endl;
   std::cout << "channelWidth: " << channelWidth << std::endl;
   std::cout << "guardIntervalNs: " << guardIntervalNs << std::endl;
   */
   try {
      // Handle pqxx database business here
   }
   catch (const std::exception &e) {
      std::cerr << "database cerr: " << e.what() << std::endl;
      return false;
   }
   return true;
}
