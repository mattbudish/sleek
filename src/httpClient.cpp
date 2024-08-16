// httpClient.cpp
#include <curl/curl.h>
#include <stdexcept>

#include "httpClient.hpp"

using namespace std;

static size_t buf_write(void *ptr, size_t size, size_t nmemb, void *stream)
{
	((streambuf*)stream)->sputn((char*)ptr, nmemb);
	return size * nmemb;
}

long sleek::net::httpClient(const string &url, ostream &http_data)
{
    CURL *curl_object = nullptr;
    CURLcode res;
    struct curl_slist *slist = nullptr;
    long timeout = 5000;
    long http_code;

    curl_object = curl_easy_init();

	curl_easy_setopt(curl_object, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_object, CURLOPT_TIMEOUT_MS, timeout);
	curl_easy_setopt(curl_object, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl_object, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_object, CURLOPT_WRITEFUNCTION, buf_write);
	curl_easy_setopt(curl_object, CURLOPT_WRITEDATA, http_data.rdbuf());

    res = curl_easy_perform(curl_object);
    if (CURLE_OK != res)
    {
        throw runtime_error(string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curl_object, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl_object);

    return http_code;
}