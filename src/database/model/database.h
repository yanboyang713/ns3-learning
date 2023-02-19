#ifndef DATABASE_H
#define DATABASE_H
#include <string>

class database{
    //protected:

    private:
        std::string dbname;

    public:
        database();
        bool connect();

};

//std::string sha256(std::string input);

#endif
