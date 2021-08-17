/*
 * Licensed to Elasticsearch B.V. under one or more contributor
 * license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright
 * ownership. Elasticsearch B.V. licenses this file to you under
 * the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <sys/socket.h>
#include <sys/un.h>
#include "backend_comm.h"
#include "elastic_apm_version.h"
#if defined(PHP_WIN32) && ! defined(CURL_STATICLIB)
#   define CURL_STATICLIB
#endif
#include <curl/curl.h>

#define ELASTIC_APM_CURRENT_LOG_CATEGORY ELASTIC_APM_LOG_CATEGORY_BACKEND_COMM

// Log response
static
size_t logResponse( void* data, size_t unusedSizeParam, size_t dataSize, void* unusedUserDataParam )
{
    // https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    // size (unusedSizeParam) is always 1
    ELASTIC_APM_UNUSED( unusedSizeParam );
    ELASTIC_APM_UNUSED( unusedUserDataParam );

    ELASTIC_APM_LOG_DEBUG( "APM Server's response body [length: %"PRIu64"]: %.*s", (UInt64) dataSize, (int) dataSize, (const char*) data );
    return dataSize;
}

#define ELASTIC_APM_CURL_EASY_SETOPT( curl, curlOptionId, ... ) \
    do { \
        CURLcode curl_easy_setopt_ret_val = curl_easy_setopt( curl, curlOptionId, __VA_ARGS__ ); \
        if ( curl_easy_setopt_ret_val != CURLE_OK ) \
        { \
            ELASTIC_APM_LOG_ERROR( "Failed to set cUrl option. curlOptionId: %d.", curlOptionId ); \
            resultCode = resultCurlFailure; \
            goto failure; \
        } \
    } while ( false ) \
    /**/

ResultCode sendEventsToApmServer( double serverTimeoutMilliseconds, const ConfigSnapshot* config, StringView serializedEvents )
{
    struct sockaddr_un server;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        php_printf("Error init socket");
        return resultFailure;
    }
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "/apm/apmdaemon.sock");
    if (connect(fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        php_printf("Error connect socket");
        close(fd);
        return resultFailure;
    }

    if (write(fd, serializedEvents.begin, serializedEvents.length) < 0) {
        php_printf("Error send data");
        return resultFailure;
    }

    return resultSuccess;

    // long serverTimeoutMillisecondsLong = (long) ceil( serverTimeoutMilliseconds );
    // php_printf("%s\n", serializedEvents.begin);
    // ResultCode resultCode;
    // CURL* curl = NULL;
    // CURLcode result;
    // enum
    // {
    //     authBufferSize = 256
    // };
    // char auth[authBufferSize];
    // enum
    // {
    //     userAgentBufferSize = 100
    // };
    // char userAgent[userAgentBufferSize];
    // enum
    // {
    //     urlBufferSize = 256
    // };
    // char url[urlBufferSize];
    // struct curl_slist* chunk = NULL;
    // int snprintfRetVal;
    // const char* authKind = NULL;
    // const char* authValue = NULL;

    // /* get a curl handle */
    // curl = curl_easy_init();
    // if ( curl == NULL )
    // {
    //     ELASTIC_APM_LOG_ERROR( "curl_easy_init() returned NULL" );
    //     resultCode = resultFailure;
    //     goto failure;
    // }


    // // Authorization with API key or secret token if present
    // if ( ! isNullOrEmtpyString( config->apiKey ) )
    // {
    //     authKind = "ApiKey";
    //     authValue = config->apiKey;
    // }
    // else if ( ! isNullOrEmtpyString( config->secretToken ) )
    // {
    //     authKind = "Bearer";
    //     authValue = config->secretToken;
    // }

    // if ( authValue != NULL )
    // {
    //     snprintfRetVal = snprintf( auth, authBufferSize, "Authorization: %s %s", authKind, authValue );
    //     if ( snprintfRetVal < 0 || snprintfRetVal >= authBufferSize )
    //     {
    //         php_printf( "Failed to build Authorization header."
    //                                " snprintfRetVal: %d. authKind: %s. authValue: %s.\n", snprintfRetVal, authKind, authValue );
    //         resultCode = resultFailure;
    //         goto failure;
    //     }
    //     php_printf( "Adding header: %s\n", auth );
    //     chunk = curl_slist_append( chunk, auth );
    // }
    // chunk = curl_slist_append( chunk, "Content-Type: application/x-ndjson" );
    // ELASTIC_APM_CURL_EASY_SETOPT( curl, CURLOPT_HTTPHEADER, chunk );

    // // User agent - "elasticapm-<language>/<version>" (no separators between "elastic" and "apm" is on purpose)
    // // For example, "elasticapm-java/1.2.3" or "elasticapm-dotnet/1.2.3"
    // snprintfRetVal = snprintf( userAgent, userAgentBufferSize, "elasticapm-php/%s", PHP_ELASTIC_APM_VERSION );
    // if ( snprintfRetVal < 0 || snprintfRetVal >= authBufferSize )
    // {
    //     php_printf( "Failed to build User-Agent header. snprintfRetVal: %d\n", snprintfRetVal );
    //     resultCode = resultFailure;
    //     goto failure;
    // }
    // ELASTIC_APM_CURL_EASY_SETOPT( curl, CURLOPT_USERAGENT, userAgent );

    // snprintfRetVal = snprintf( url, urlBufferSize, "%s/intake/v2/events", config->serverUrl );
    // if ( snprintfRetVal < 0 || snprintfRetVal >= authBufferSize )
    // {
    //     php_printf( "Failed to build full URL to APM Server's intake API. snprintfRetVal: %d\n", snprintfRetVal );
    //     resultCode = resultFailure;
    //     goto failure;
    // }
    // ELASTIC_APM_CURL_EASY_SETOPT( curl, CURLOPT_URL, url );

    // result = curl_easy_perform( curl );
    // if ( result != CURLE_OK )
    // {
    //     php_printf( "Sending events to APM Server failed. URL: `%s'. Error message: `%s'.\n", url, curl_easy_strerror( result ) );
    //     resultCode = resultFailure;
    //     goto failure;
    // }

    // long responseCode;
    // curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &responseCode );
    // php_printf( "Sent events to APM Server. Response HTTP code: %ld. URL: `%s'.\n", responseCode, url );

    // resultCode = resultSuccess;

    // finally:
    // if ( curl != NULL ) curl_easy_cleanup( curl );

    // ELASTIC_APM_LOG_DEBUG_FUNCTION_EXIT_MSG( "resultCode: %s (%d)", resultCodeToString( resultCode ), resultCode );
    // return resultCode;

    // failure:
    // goto finally;
}

#undef ELASTIC_APM_CURL_EASY_SETOPT
