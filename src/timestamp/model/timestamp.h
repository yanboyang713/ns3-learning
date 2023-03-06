#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include <string>
//Ceil and Floor functions
#include <cmath>
//std::setw() std::setfill()
#include <iomanip>
#include <chrono>

struct timeFormat {
    std::string micro;
    std::string seconds;
    std::string minutes;
    std::string hours;
    std::string year;
    std::string month;
    std::string day;
    std::string timeString;
};

class timestamp{
        //protected:

        private:
                timeFormat time;
                std::string padLeadingZeros(std::string, size_t);
                std::string padLeadingZeros(int, int);
                void timeStampGenerate();
                void removeAfterDotPosition (std::string *);
        public:
                timestamp();
                ~timestamp();

                std::string getTimeString () const;
};
#endif
