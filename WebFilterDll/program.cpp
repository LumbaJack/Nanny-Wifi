/* 
 * File:   program.cpp
 * Author: jacksalien
 *
 * Created on December 22, 2014, 10:06 AM
 */

#include "program.h"

#include <fstream>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sys/stat.h>

//////////////////////////////////////////////////////////////////////////////

Program::Program()
    : m_rvalue( 0 )
    , m_tolaranceLevel( 50 )
    , m_currentSiteGoodWeightLevel( 0 )
{
    //**************Configuration file loading**************
    std::ifstream stream("configFile.conf");
    
    if(!stream){
        std::cout << "No configuration file found." << std::endl;
    } else{
        parseConfig(stream);
    }
    //******************************************************    
    
    //**************Load enabled categories**************
    loadCategories();
    //***************************************************   
}

Program::~Program()
{
}

int
Program::GetRvalue()
{
    return m_rvalue;
}

int
Program::execute( std::string siteContents )
{
    m_rvalue = RETURN_SUCCESS;
    std::transform(siteContents.begin(), siteContents.end(), siteContents.begin(), ::toupper);
    siteContents = stripHtmlTags(siteContents);
    
    if(siteContents.length() > 0){
        if(validateAgainstCategories(siteContents)){
            m_rvalue = ADULT_CONTENT;
        }
    } else{
        std::cout<<"No Contents Found"<<std::endl;
        m_rvalue = MISSING_SITE_CONTENTS;
    }

    return m_rvalue;
}

bool
Program::validateAgainstCategories(std::string contents)
{
    bool results = false;
    bool goodSectionSet = false;

    for(int i=0; i<catMainContainer.size(); i++){
        m_currentSiteWeightLevel = 0;
        if(options[catMainContainer[i].categoryClass] == "true"){
            if(checkSingleLevelWeights(contents, catMainContainer[i].singleContainerVec)){
                failedSections.push_back(catMainContainer[i].categoryClass);
                results = true;
#ifdef DEBUG
std::cout<<"Found inner section: "<<catMainContainer[i].categoryClass<<std::endl;
std::cout<<"Total: "<<m_currentSiteWeightLevel<<std::endl;
#endif
                break;
            }
        } else{
            continue;
        }
        
        checkMultiLevelWeights(contents, catMainContainer[i].multiContainerVec);
        
        if (m_currentSiteWeightLevel > m_tolaranceLevel){
            if(goodSectionSet){
                m_currentSiteWeightLevel += m_currentSiteGoodWeightLevel;
                
                if (m_currentSiteWeightLevel > m_tolaranceLevel){
                    failedSections.push_back(catMainContainer[i].categoryClass);
                    results = true;
                }
            } else{
                goodSectionSet = true;
                int currentValue = m_currentSiteWeightLevel;
                m_currentSiteWeightLevel = 0;
                
                checkSingleLevelWeights(contents, goodMainContainer[0].singleContainerVec);
                checkMultiLevelWeights(contents, goodMainContainer[0].multiContainerVec);
                
                m_currentSiteGoodWeightLevel += m_currentSiteWeightLevel;
                m_currentSiteWeightLevel += currentValue;
                
                if (m_currentSiteWeightLevel > m_tolaranceLevel){
                    failedSections.push_back(catMainContainer[i].categoryClass);
                    results = true;
#ifdef DEBUG
std::cout<<"Found section: "<<catMainContainer[i].categoryClass<<std::endl;
std::cout<<"Total: "<<m_currentSiteWeightLevel<<std::endl;
#endif
                }
            }
        }
    }
    
    return results;
}

int
Program::checkSingleLevelWeights(std::string inputcontents, std::vector<std::string> inputsingleLevelWeights)
{
    std::string contents;
    int inputsize = inputcontents.size();
    
    for(int x=0; x<inputsize;x+=8192){
        int sessionscore = 0;
        std::vector<std::string> singleLevelWeights = inputsingleLevelWeights;

        if ((inputsize-x) > 8192){
            contents = inputcontents.substr(x, 8192);
        } else{
            contents = inputcontents.substr(x, inputsize-x);
        }
        
        for(int i=0; i<singleLevelWeights.size(); i++){
            int weightValue = 0;
            std::string weightString;
            std::string delimiter = "><";
            std::string::size_type start_position = 0;

            std::string token;
            while ((start_position = singleLevelWeights[i].find(delimiter)) != std::string::npos) {
                token = singleLevelWeights[i].substr(0, start_position);
                weightString = token.erase(0, 1);
                singleLevelWeights[i].erase(0, start_position + delimiter.length());
            }
            weightValue = atoi(singleLevelWeights[i].erase(singleLevelWeights[i].size()-1).c_str());

            if(weightString.length() > 0 and weightValue != 0){
                if(contents.find(weightString) != std::string::npos){
    #ifdef DEBUG
    std::cout<<"'"<<weightString<<"'"<<":"<<weightValue<<std::endl;
    #endif
                    m_currentSiteWeightLevel += weightValue;
                    sessionscore += weightValue;
                }
            }
        }
        
        if (sessionscore >= m_tolaranceLevel*2){
#ifdef DEBUG
std::cout<<"Session total: "<<sessionscore<<std::endl;
#endif
            return 1;
        }
    }
    return 0;
}

void
Program::checkMultiLevelWeights(std::string contents, std::vector<std::string> multiLevelWeights)
{
    int weightValue;
    std::string token;
    std::string tempWeightString;
    std::vector<std::string> weightString;
            
    for(int i=0; i<multiLevelWeights.size(); i++){
        bool found = true;
        std::string delimiter = "><";
        std::string seconddelimiter = ">,<";
        std::string::size_type start_position = 0;

        while ((start_position = multiLevelWeights[i].find(delimiter)) != std::string::npos) {
            token = multiLevelWeights[i].substr(0, start_position);
            tempWeightString = token.erase(0, 1);
            multiLevelWeights[i].erase(0, start_position + delimiter.length());
        }
        weightValue = atoi(multiLevelWeights[i].erase(multiLevelWeights[i].size()-1).c_str());
        
        while ((start_position = tempWeightString.find(seconddelimiter)) != std::string::npos) {
            token = tempWeightString.substr(0, start_position);
            weightString.push_back(token);
            tempWeightString.erase(0, start_position + seconddelimiter.length());
        }
        weightString.push_back(tempWeightString);

        if(weightString.size() > 0 and weightValue > 0){
            for(int i=0;i<weightString.size();i++){
                if(contents.find(weightString[i]) == std::string::npos){
                    found = false;
                    break;
                }
            }
            if(found){
                m_currentSiteWeightLevel += weightValue;
#ifdef DEBUG
for(int i=0;i<weightString.size();i++){
    std::cout<<"'"<<weightString[i]<<"'"<<":";
}
std::cout<<weightValue<<std::endl;
#endif
            }
        }
    }
}

void
Program::parseConfig(std::ifstream & cfgfile)
{
    options.clear();
    std::string id, eq, val;

     while(cfgfile >> id >> eq >> val) {
        if (id == "config"){
            continue;
        } else if (eq == "tolaranceLevel"){
            m_tolaranceLevel = atoi(val.substr(1, val.size()-2).c_str());
        } else{
            options[eq] = val.substr(1, val.size()-2);
        }
    }
}

void
Program::loadCategories()
{
    //good words
    goodMainContainer.push_back(catGoodHolder());
    goodMainContainer[0].singleContainerVec = cat_good_words_vec;
    goodMainContainer[0].multiContainerVec = cat_good_words_double_vec;
    
    //categories
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "pornography";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_pornography_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_pornography_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "proxies";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_proxies_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_proxies_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "weapons";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_weapons_vec;
        
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "malware";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_malware_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_malware_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "illegaldrugs";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_illegaldrugs_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "personals";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_personals_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_personals_double_vec;
            
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "badwords";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_badwords_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "chat";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_chat_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_chat_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "conspiracy";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_conspiracy_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "drugadvocacy";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_drugadvocacy_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "gambling";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_gambling_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_gambling_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "games";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_games_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_games_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "gore";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_gore_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_gore_double_vec;

    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "intolerance";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_intolerance_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "legaldrugs";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_legaldrugs_vec;

    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "nudism";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_nudism_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_nudism_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "peer2peer";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_peer2peer_vec;

    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "secretsocieties";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_secretsocieties_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_secretsocieties_double_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "violence";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_violence_vec;
    
    catMainContainer.push_back(catHolder());
    catMainContainer[catMainContainer.size()-1].categoryClass = "warezhacking";
    catMainContainer[catMainContainer.size()-1].singleContainerVec = cat_warezhacking_vec;
    catMainContainer[catMainContainer.size()-1].multiContainerVec = cat_warezhacking_double_vec;
}

finderStruct
Program::findLocationCats(std::string className)
{
    finderStruct returnType;
    
    if(catMainContainer.size() == 0){
        returnType.found = false;
        returnType.location = 0;
        
        return returnType;
    }
    
    for(int i=0;i<catMainContainer.size();i++){
        if(catMainContainer[i].categoryClass == className){
            returnType.found = true;
            returnType.location = i;
            
            return returnType;
        }
    }
    
    returnType.found = false;
    returnType.location = catMainContainer.size();
    
    return returnType;
}

bool
Program::isDir(std::string dir){
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);

    if (S_ISDIR(fileInfo.st_mode)){
        return true;
    }else{
        return false;
    }
}

bool
BothAreSpaces(char lhs, char rhs) {
    return (lhs == rhs) && (lhs == ' ');
}

std::string
Program::stripHtmlTags(std::string html)
{
    std::string text;
    std::string original = html;
    
    try {
        for(;;) {
            std::string::size_type  startpos;

            startpos = html.find('<');
            if(startpos == std::string::npos) {
                text += html;
                break;
            }

            if(0 != startpos) {
                text += html.substr(0, startpos);
                html = html.substr(startpos, html.size() - startpos);
                startpos = 0;
            }

            std::string::size_type endpos;
            for(endpos = startpos; endpos < html.size() && html[endpos] != '>'; ++endpos) {
                if(html[endpos] == '"') {
                    endpos++;
                    while(endpos < html.size() && html[endpos] != '"') {
                        endpos++;
                    }
                }
            }

            if(endpos == html.size()) {
                html = html.substr(endpos, html.size() - endpos);
                break;
            } else {
                endpos++;
                html = html.substr(endpos, html.size() - endpos);
            }
        }
    } catch ( const std::exception& ex ) {
        return original;
    }
    
    std::string::iterator new_end = std::unique(text.begin(), text.end(), BothAreSpaces);
    text.erase(new_end, text.end());
    text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
    text.erase(std::remove(text.begin(), text.end(), '\t'), text.end());

    return text;
}

extern "C" int dllExecute(char * siteContents);

int dllExecute(char * input)
{
    Program program;
    std::string siteContents(input);
    int rvalue = program.execute(siteContents);
    return rvalue;
}

//////////////////////////////////////////////////////////////////////////////
