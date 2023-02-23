#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include <cstring>
#include <pqxx/pqxx>
class database{
    //protected:

    private:
        std::string dbname;
        std::string userName;
        std::string password;
        std::string hostAddress;
        std::string port;

        //connection string
        std::string url;

        pqxx::connection * conn = nullptr;

    public:
        database();
        database(std::string);
        database(std::string, std::string, std::string, std::string, std::string);
        ~database();
        pqxx::connection * getConnection();
        bool runRecord (std::string, char [1024], std::string, std::string, uint32_t, double, uint32_t, std::string,
                        uint16_t, double, std::string, uint16_t, uint16_t);
};
#endif
