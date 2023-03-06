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
    //Also could be: pqxx::connection C{"postgresql://postgres:password@127.0.0.1:5432/anomalyprediction"};

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
   //Also could be: pqxx::connection C{"postgresql://postgres:password@127.0.0.1:5432/anomalyprediction"};
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
       std::cout << "Can't open database" << std::endl;
    } else {
       std::cout << "Opened database successfully: " << conn->dbname() << std::endl;
    }
    return conn;
}

bool database::runRecord (std::string runID, char hostname[1024], std::string dataMode, std::string controlMode, uint32_t numOfNodes,
                          double simulationTime, uint32_t payloadSize, std::string dataRate, uint16_t channelNumber,
                          double frequency, std::string standard, uint16_t channelWidth, uint16_t guardIntervalNs){
   /*
   //Print agus out
   std::cout << "RunID: " << runID << std::endl;
   std::cout << "Hostname: " << hostname << std::endl;
   std::cout << "Data Mode: " << dataMode << std::endl;
   std::cout << "Control Mode: " << controlMode << std::endl;
   std::cout << "Num Of Nodes: " << numOfNodes<< std::endl;
   std::cout << "Simulation time: " << simulationTime << std::endl;
   std::cout << "payloadSize (bytes)" << payloadSize << std::endl;
   std::cout << "dataRate: " << dataRate << std::endl;
   std::cout << "ChannelNumber: " << channelNumber << std::endl;
   std::cout << "frequency: " << frequency << std::endl;
   std::cout << "WIFI Standard: " << standard << std::endl;
   std::cout << "channelWidth: " << channelWidth << std::endl;
   std::cout << "guardIntervalNs: " << guardIntervalNs << std::endl;
   */
   try {
      // Create SQL statement
      std::string sql = "INSERT INTO runrecord (runid, hostname, datamode, controlmode,  numofnodes, simulationtime, payloadsize, datarate, channelnumber, frequency, standard, channelwidth, guardinterval ) "  \
         "VALUES (' " + runID + " ', ' " + hostname + " ', ' " + dataMode + " ', ' " + controlMode + " ', ' " + std::to_string(numOfNodes) + " ', ' " + std::to_string(simulationTime) + " ', ' " + std::to_string(payloadSize) + " ', ' " + dataRate + " ', ' " + std::to_string(channelNumber) + " ', ' " + std::to_string(frequency) + " ', ' " + standard + " ', ' " + std::to_string(channelWidth) + " ', ' " + std::to_string(guardIntervalNs) + " ');";

      /* Create a transactional object. */
      work W(*conn);

      /* Execute SQL query */
      W.exec( sql );
      W.commit();
   }
   catch (const std::exception &e) {
      std::cerr << "database runrecord insert cerr: " << e.what() << std::endl;
      return false;
   }
   return true;
}

bool database::RxPacketInfoRecord(std::string runID, char hostname[1024], std::string type, std::string timeString,
                                  std::string context, std::string nodeName, uint32_t nodeID, double signalDB,
                                  double noiseDB, uint16_t channelFreqMhz, uint8_t ness, uint8_t nss, uint8_t powerLevel,
                                  uint32_t packetSize, uint64_t packetUID){
   /*
   std::cout << "Run ID: " << runID << std::endl;
   std::cout << "Hostname: " << hostname << std::endl;
   std::cout << "type: " << type << std::endl;
   std::cout << "timeString: " << timeString << std::endl;
   std::cout << "context: " << context << std::endl;
   std::cout << "nodeName: " << nodeName << std::endl;
   std::cout << "nodeID: " << nodeID << std::endl;
   std::cout << "signalDB: " << signalDB << std::endl;
   std::cout << "noiseDB: " << noiseDB << std::endl;
   std::cout << "channelFreqMhz: " << channelFreqMhz << std::endl;
   std::cout << "ness: " << ness << std::endl;
   std::cout << "nss: " << nss << std::endl;
   std::cout << "powerLevel: " << powerLevel << std::endl;
   std::cout << "packetSize: " <<  packetSize << std::endl;
   std::cout << "packetUID: " <<  packetUID << std::endl;

   */

   try {
      // Create SQL statement
      std::string sql = "INSERT INTO rxpacketinfo (runid, hostname, type, time, context, nodename, nodeid, signaldb, noisedb, channelfreqmhz, ness, nss, powerlevel, packetsize, packetuid) "  \
         "VALUES (' " + runID + " ', ' " + hostname + " ', ' " + type + " ', ' " + timeString + " ', ' " + context + " ', ' " + nodeName + " ', ' " + std::to_string(nodeID) + " ', ' " + std::to_string(signalDB) + " ', ' " + std::to_string(noiseDB) + " ', ' " + std::to_string(channelFreqMhz) + " ', ' " + std::to_string(ness) + " ', ' " + std::to_string(nss) + " ', ' " + std::to_string(powerLevel) + " ', ' " + std::to_string(packetSize) + " ', ' " + std::to_string(packetUID) + " ');";

      /* Create a transactional object. */
      work W(*conn);

      /* Execute SQL query */
      W.exec( sql );
      W.commit();
   }
   catch (const std::exception &e) {
      std::cerr << "database rxpacketinfo insert cerr: " << e.what() << std::endl;
      return false;
   }

   return true;
}
    //(type, time.timeString, context, nodesRecords.getName(contextToNodeId(context)), contextToNodeId(context), signalNoise.signal, signalNoise.noise, channelFreqMhz, vector.ness, vector.nss, vector.powerLevel)
    //(std::string, std::string, std::string, std::string, uint32_t, double, double, uint16_t, uint8_t, uint8_t, uint8_t)
/*
    bool succ = dataOutput.TxPacketInfoRecord(runID, hostname, type, timestamp.getTimeString(), context,
                                              nodesRecords.getName(contextToNodeId(context)), contextToNodeId(context),
                                              channelFreqMhz, vector.ness, vector.nss, vector.powerLevel,
                                              packetResult.size, packetResult.UID);
 */
bool database::TxPacketInfoRecord(std::string runID, char hostname[1024], std::string type, std::string timeString,
                                  std::string context, std::string nodeName, uint32_t nodeID, uint16_t channelFreqMhz,
                                  uint8_t ness, uint8_t nss, uint8_t powerLevel,
                                  uint32_t packetSize, uint64_t packetUID){
    /*
   std::cout << "Run ID: " << runID << std::endl;
   std::cout << "Hostname: " << hostname << std::endl;
   std::cout << "type: " << type << std::endl;
   std::cout << "timeString: " << timeString << std::endl;
   std::cout << "context: " << context << std::endl;
   std::cout << "nodeName: " << nodeName << std::endl;
   std::cout << "nodeID: " << nodeID << std::endl;
   std::cout << "channelFreqMhz: " << channelFreqMhz << std::endl;
   std::cout << "ness: " << ness << std::endl;
   std::cout << "nss: " << nss << std::endl;
   std::cout << "powerLevel: " << powerLevel << std::endl;
   std::cout << "packetSize: " <<  packetSize << std::endl;
   std::cout << "packetUID: " <<  packetUID << std::endl;
     */

   try {
      // Create SQL statement
      std::string sql = "INSERT INTO txpacketinfo (runid, hostname, type, time, context, nodename, nodeid, channelfreqmhz, ness, nss, powerlevel, packetsize, packetuid) "  \
         "VALUES (' " + runID + " ', ' " + hostname + " ', ' " + type + " ', ' " + timeString + " ', ' " + context + " ', ' " + nodeName + " ', ' " + std::to_string(nodeID) + " ', ' " + std::to_string(channelFreqMhz) + " ', ' " + std::to_string(ness) + " ', ' " + std::to_string(nss) + " ', ' " + std::to_string(powerLevel) + " ', ' " + std::to_string(packetSize) + " ', ' " + std::to_string(packetUID) + " ');";

      // Create a transactional object.
      work W(*conn);

      // Execute SQL query
      W.exec( sql );
      W.commit();
   }
   catch (const std::exception &e) {
      std::cerr << "database txpacketinfo insert cerr: " << e.what() << std::endl;
      return false;
   }

   return true;
}
/*
    bool succ = dataOutput.PhyTxDropInfoRecord(runID, hostname, type, timestamp.getTimeString(), context,
                                              nodesRecords.getName(contextToNodeId(context)), contextToNodeId(context),
                                              packetResult.size, packetResult.UID);
 */
bool database::PhyTxDropInfoRecord(std::string runID, char hostname[1024], std::string type, std::string timeString,
                                   std::string context, std::string nodeName, uint32_t nodeID, uint32_t packetSize,
                                   uint64_t packetUID){
    /*
   std::cout << "Run ID: " << runID << std::endl;
   std::cout << "Hostname: " << hostname << std::endl;
   std::cout << "type: " << type << std::endl;
   std::cout << "timeString: " << timeString << std::endl;
   std::cout << "context: " << context << std::endl;
   std::cout << "nodeName: " << nodeName << std::endl;
   std::cout << "nodeID: " << nodeID << std::endl;
   std::cout << "packetSize: " <<  packetSize << std::endl;
   std::cout << "packetUID: " <<  packetUID << std::endl;
     */

   try {
      // Create SQL statement
      std::string sql = "INSERT INTO phytxdropinfo (runid, hostname, type, time, context, nodename, nodeid, packetsize, packetuid) "  \
         "VALUES (' " + runID + " ', ' " + hostname + " ', ' " + type + " ', ' " + timeString + " ', ' " + context + " ', ' " + nodeName + " ', ' " + std::to_string(nodeID) + " ', ' " + std::to_string(packetSize) + " ', ' " + std::to_string(packetUID) + " ');";

      // Create a transactional object.
      work W(*conn);

      // Execute SQL query
      W.exec( sql );
      W.commit();
   }
   catch (const std::exception &e) {
      std::cerr << "database insert PhyTxDropInfoRecord cerr: " << e.what() << std::endl;
      return false;
   }

   return true;
}
bool database::PhyRxDropInfoRecord(std::string runID, char hostname[1024], std::string type, std::string timeString,
                                   std::string context, std::string nodeName, uint32_t nodeID, uint32_t packetSize,
                                   uint64_t packetUID, std::string WifiPhyRxfailureReason){
    /*
   std::cout << "Run ID: " << runID << std::endl;
   std::cout << "Hostname: " << hostname << std::endl;
   std::cout << "type: " << type << std::endl;
   std::cout << "timeString: " << timeString << std::endl;
   std::cout << "context: " << context << std::endl;
   std::cout << "nodeName: " << nodeName << std::endl;
   std::cout << "nodeID: " << nodeID << std::endl;
   std::cout << "packetSize: " << packetSize << std::endl;
   std::cout << "packetUID: " << packetUID << std::endl;
   std::cout << " WifiPhyRxfailureReason: " <<  WifiPhyRxfailureReason << std::endl;
    */

   try {
      // Create SQL statement
      std::string sql = "INSERT INTO phyrxdropinfo (runid, hostname, type, time, context, nodename, nodeid, packetsize, packetuid, failurereason) "  \
         "VALUES (' " + runID + " ', ' " + hostname + " ', ' " + type + " ', ' " + timeString + " ', ' " + context + " ', ' " + nodeName + " ', ' " + std::to_string(nodeID) + " ', ' " + std::to_string(packetSize) + " ', ' " + std::to_string(packetUID) + " ', ' " + WifiPhyRxfailureReason + " ');";

      // Create a transactional object.
      work W(*conn);

      // Execute SQL query
      W.exec( sql );
      W.commit();
   }
   catch (const std::exception &e) {
      std::cerr << "database insert PhyTxDropInfoRecord cerr: " << e.what() << std::endl;
      return false;
   }

   return true;
}
