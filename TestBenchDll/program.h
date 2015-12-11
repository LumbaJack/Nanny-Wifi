/* 
 * File:   program.h
 * Author: jacksalien
 *
 * Created on December 22, 2014, 10:09 AM
 */

#ifndef PROGRAM_H
#define	PROGRAM_H

#include "cat_badwords.h"
#include "cat_chat.h"
#include "cat_conspiracy.h"
#include "cat_drugadvocacy.h"
#include "cat_gambling.h"
#include "cat_games.h"
#include "cat_gore.h"
#include "cat_illegaldrugs.h"
#include "cat_intolerance.h"
#include "cat_legaldrugs.h"
#include "cat_malware.h"
#include "cat_nudism.h"
#include "cat_peer2peer.h"
#include "cat_personals.h"
#include "cat_pornography.h"
#include "cat_proxies.h"
#include "cat_secretsocieties.h"
#include "cat_violence.h"
#include "cat_warezhacking.h"
#include "cat_weapons.h"
#include "good_cat_words.h"

#include <map>
#include <string>
#include <vector>

struct finderStruct {
    bool found;
    int location;
};

class Program
{
    Program( const Program& );
    Program& operator=( const Program& );

public:
    Program();
    ~Program();

    enum RETURN_VALUE {
        RETURN_SUCCESS = 0,
        ADULT_CONTENT = 1,
        BAD_CONFIG_FILE = 2,
        MISSING_PHRASES_DIRECTORY = 3,
        INVALID_ARGUMENT_COUNT = 4,
        MISSING_SITE_CONTENTS = 5,
        GENERAL_ERROR = 255
    };

    int GetRvalue();
    bool isDir(std::string dir);
    int execute( std::string siteContents );
    void parseConfig(std::ifstream & cfgfile);
    void loadCategories();
    void loadContentsHelper(std::string fileName, std::string className, bool goodSection);
    finderStruct findLocationCats(std::string className);
    bool validateAgainstCategories(std::string contents);
    int checkSingleLevelWeights(std::string contents, std::vector<std::string> singleLevelWeights);
    void checkMultiLevelWeights(std::string contents, std::vector<std::string> multiLevelWeights);
    std::string stripHtmlTags(std::string html);
    
    //////////////////////////////////////////////////////////////////////////
private:
    int m_rvalue;
    int m_tolaranceLevel;
    int m_currentSiteWeightLevel;
    int m_currentSiteGoodWeightLevel;
    std::vector<std::string> failedSections;
    std::map<std::string, std::string> options;
    
    struct catHolder {
      std::string categoryClass;
      std::vector<std::string> singleContainerVec;
      std::vector<std::string> multiContainerVec;
    };
    std::vector<catHolder> catMainContainer;
    
    struct catGoodHolder {
      std::vector<std::string> singleContainerVec;
      std::vector<std::string> multiContainerVec;
    };
    std::vector<catGoodHolder> goodMainContainer;
};

//////////////////////////////////////////////////////////////////////////////

#endif	/* PROGRAM_H */