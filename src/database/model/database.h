#ifndef DATABASE_H
#define DATABASE_H
#include <string>

class database{
    //protected:

    private:
        std::string dbname;
        std::string userName;
        std::string password;
        std::string hostAddress;
        std::string port;

        std::string url;

    public:
        database();
        database(std::string url);
        ~database();
        bool connect();
        bool runRecord (std::string, char [1024], std::string, std::string, uint32_t, double, double, uint32_t, std::string,
                        uint16_t, double, std::string, uint16_t, uint16_t);
};

#endif
