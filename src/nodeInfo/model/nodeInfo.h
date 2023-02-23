#ifndef NODEINFO_H
#define NODEINFO_H
#include <string>
#include <cstring>
#include <map>
struct node{
        std::string name;
        double Xlocation;
        double Ylocation;
};
class nodeInfo{
        //protected:

        private:
                std::map<int, node> nodes;

        public:
                nodeInfo();
                ~nodeInfo();
                bool insertName(int, std::string);
                std::string getName (int);

                bool updateXlocation(int, double);
                double getXlocation(int);

                bool updateYlocation(int, double);
                double getYlocation(int);

};
#endif
