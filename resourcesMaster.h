#ifndef RESOURCESMASTER_H_INCLUDED
#define RESOURCESMASTER_H_INCLUDED

#include <iostream>
#include <unordered_map>

struct ResourcesConfig
{
    static std::string RESOURCES_DIR; //the config variable name that denotes where our resources folder is
    static std::unordered_map<std::string,std::string> config; //a set of configuration variables
    static void loadConfig(); //loads config file variables

};

#endif // RESOURCESMASTER_H_INCLUDED
