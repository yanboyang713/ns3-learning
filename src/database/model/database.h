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

};

//std::string sha256(std::string input);

#endif
