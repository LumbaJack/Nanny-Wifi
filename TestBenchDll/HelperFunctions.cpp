#include "HelperFunctions.h"

#include <sstream>
#include <curl/curl.h>

//////////////////////////////////////////////////////////////////////////////

HtmlRetreive::HtmlRetreive()
{
}

HtmlRetreive::~HtmlRetreive()
{
}

static size_t
data_write(void* buf, size_t size, size_t nmemb, void* userp)
{
    if(userp) {
        std::ostream& os = *static_cast<std::ostream*>(userp);
        std::streamsize len = size * nmemb;

        if(os.write(static_cast<char*>(buf), len))
            return len;
    }

    return 0;
}

std::string
HtmlRetreive::UrlRetrieval(std::string url)
{
    std::string html;
    std::ostringstream oss;
    curl_global_init(CURL_GLOBAL_ALL);

    if(CURLE_OK == curl_read(url.c_str(), oss)) {
        html = oss.str();
    }

    curl_global_cleanup();

    return html;
}

CURLcode
HtmlRetreive::curl_read(const std::string& url, std::ostream& os)
{
    long timeout = 30;
    CURLcode code(CURLE_FAILED_INIT);
    CURL* curl = curl_easy_init();

    if(curl)
    {
        if(CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &data_write))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
        && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str())))
        {
            code = curl_easy_perform(curl);
        }
        curl_easy_cleanup(curl);
    }
    return code;
}
