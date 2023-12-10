#include "resourcesMaster.h"
#include "vanilla.h"

std::string ResourcesConfig::RESOURCES_DIR = "resources_dir";

std::unordered_map<std::string,std::string> ResourcesConfig::config;

void ResourcesConfig::loadConfig()
{
    std::string configContent = readFile("./viewport.conf").first; //read config file

    std::regex rgx("([^\/]+)=([^\/]+)"); //parse config file
    for (std::sregex_iterator it = std::sregex_iterator(configContent.begin(), configContent.end(), rgx);
    it != std::sregex_iterator(); it++)
    {
         //set our values into "config"
        config[(*it)[1]] = (*it)[2];
    }
    //no matter what, the "resources_dir" variable should be set
    if (config.find(RESOURCES_DIR) == config.end())
    {
        config[RESOURCES_DIR] = "../../resources";
    }
}
