#ifndef SERVERHELPERS_H
#define SERVERHELPERS_H

#include <iostream>

class ServerHelper{

public:
    static bool InputCheck(int argc, char** argv){
        if(argc != 3){
            std::cout<<"Usage: ./progName <ipAddress> <port>"<<std::endl;
            return false;
        }

        return true;
    }

private:
    ServerHelper() {}
};


#endif // SERVERHELPERS_H
