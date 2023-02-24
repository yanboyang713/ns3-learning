#include "nodeInfo.h"
#include <iostream>
//#include <cstring>
nodeInfo::nodeInfo(){

}

nodeInfo::~nodeInfo(){

}

bool nodeInfo::insertName(int ID, std::string name){

    //std::map<int, node> nodes;

    std::map<int, node>::iterator it;
    it = nodes.find(ID);
    if (it != nodes.end()){
        std::cout << "Node already Exist !!!" << std::endl;
        return false;
    } else {
        node newNode;
        newNode.name = name;
        newNode.Xlocation = 0.0;
        newNode.Ylocation = 0.0;

        nodes[ID] = newNode;
    }
    return true;
}

std::string nodeInfo::getName (int ID){
    return nodes.find(ID)->second.name;
    //std::cout << "a => " << mymap.find('a')->second << '\n';
}

bool nodeInfo::updateXlocation(int ID, double location){

}
double nodeInfo::getXlocation(int ID){

}

bool nodeInfo::updateYlocation(int ID, double location){

}

double nodeInfo::getYlocation(int ID){

}
