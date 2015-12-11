/* 
 * File:   HelperFunctions.h
 * Author: jacksalien
 *
 * Created on January 25, 2015, 1:38 PM
 */

#ifndef HELPERFUNCTIONS_H
#define	HELPERFUNCTIONS_H

#include <string>
#include <curl/curl.h>

class HtmlRetreive
{
    HtmlRetreive( const HtmlRetreive& );
    HtmlRetreive& operator=( const HtmlRetreive& );

public:
    HtmlRetreive();
    ~HtmlRetreive();

    std::string UrlRetrieval(std::string url);
    CURLcode curl_read(const std::string& url, std::ostream& os);
};

//////////////////////////////////////////////////////////////////////////////
#endif	/* HELPERFUNCTIONS_H */
