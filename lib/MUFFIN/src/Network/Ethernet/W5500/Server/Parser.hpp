/**
 * @file Parser.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-09-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../Socket.h"

#include "Common/Time/ScopedTimer.h"



namespace muffin { namespace w5500 {


    bool ParseRequest(Socket& socket)
    {
        std::string requset;
        {
            ScopedTimer scopedTimer(__FILE__, __FUNCTION__);

            while (socket.Available() > 0)
            {
                uint8_t data[1] = { 0 };
                size_t actualLength;
                socket.Receive(sizeof(data), &actualLength, data);

                if (data[0] == '\r')
                {
                    uint8_t data[1] = { 0 };
                    size_t actualLength;
                    socket.Receive(sizeof(data), &actualLength, data);

                    if (data[0] == '\n')
                    {
                        break;
                    }
                }
                requset += static_cast<char>(data[0]);
            }
        }
        
        LOG_DEBUG(logger, "Request: %s", requset.c_str());
        return false;

/*
        // Read the first line of HTTP request
        String req = client.readStringUntil('\r');
        client.readStringUntil('\n');
        // reset header value
        for (int i = 0; i < _headerKeysCount; ++i)
        {
            _currentHeaders[i].value = String();
        }

        // First line of HTTP request looks like "GET /path HTTP/1.1"
        // Retrieve the "/path" part by finding the spaces
        int addr_start = req.indexOf(' ');
        int addr_end = req.indexOf(' ', addr_start + 1);
        if (addr_start == -1 || addr_end == -1)
        {
            log_e("Invalid request: %s", req.c_str());
            return false;
        }

        String methodStr = req.substring(0, addr_start);
        String url = req.substring(addr_start + 1, addr_end);
        String versionEnd = req.substring(addr_end + 8);
        _currentVersion = atoi(versionEnd.c_str());
        String searchStr = "";
        int hasSearch = url.indexOf('?');
        if (hasSearch != -1)
        {
            searchStr = url.substring(hasSearch + 1);
            url = url.substring(0, hasSearch);
        }
        _currentUri = url;
        _chunked = false;
        _clientContentLength = 0; // not known yet, or invalid

        HTTPMethod method = HTTP_ANY;
        size_t num_methods = sizeof(_http_method_str) / sizeof(const char *);
        for (size_t i = 0; i < num_methods; i++)
        {
            if (methodStr == _http_method_str[i])
            {
                method = (HTTPMethod)i;
                break;
            }
        }
        if (method == HTTP_ANY)
        {
            log_e("Unknown HTTP Method: %s", methodStr.c_str());
            return false;
        }
        _currentMethod = method;

        log_v("method: %s url: %s search: %s", methodStr.c_str(), url.c_str(), searchStr.c_str());

        // attach handler
        RequestHandler *handler;
        for (handler = _firstHandler; handler; handler = handler->next())
        {
            if (handler->canHandle(_currentMethod, _currentUri))
                break;
        }
        _currentHandler = handler;

        String formData;
        // below is needed only when POST type request
        if (method == HTTP_POST || method == HTTP_PUT || method == HTTP_PATCH || method == HTTP_DELETE)
        {
            String boundaryStr;
            String headerName;
            String headerValue;
            bool isForm = false;
            bool isEncoded = false;
            // parse headers
            while (1)
            {
                req = client.readStringUntil('\r');
                client.readStringUntil('\n');
                if (req == "")
                    break; // no moar headers
                int headerDiv = req.indexOf(':');
                if (headerDiv == -1)
                {
                    break;
                }
                headerName = req.substring(0, headerDiv);
                headerValue = req.substring(headerDiv + 1);
                headerValue.trim();
                _collectHeader(headerName.c_str(), headerValue.c_str());

                log_v("headerName: %s", headerName.c_str());
                log_v("headerValue: %s", headerValue.c_str());

                if (headerName.equalsIgnoreCase(FPSTR(Content_Type)))
                {
                    using namespace mime;
                    if (headerValue.startsWith(FPSTR(mimeTable[txt].mimeType)))
                    {
                        isForm = false;
                    }
                    else if (headerValue.startsWith(F("application/x-www-form-urlencoded")))
                    {
                        isForm = false;
                        isEncoded = true;
                    }
                    else if (headerValue.startsWith(F("multipart/")))
                    {
                        boundaryStr = headerValue.substring(headerValue.indexOf('=') + 1);
                        boundaryStr.replace("\"", "");
                        isForm = true;
                    }
                }
                else if (headerName.equalsIgnoreCase(F("Content-Length")))
                {
                    _clientContentLength = headerValue.toInt();
                }
                else if (headerName.equalsIgnoreCase(F("Host")))
                {
                    _hostHeader = headerValue;
                }
            }

            if (!isForm)
            {
                size_t plainLength;
                char *plainBuf = readBytesWithTimeout(client, _clientContentLength, plainLength, HTTP_MAX_POST_WAIT);
                if (plainLength < _clientContentLength)
                {
                    free(plainBuf);
                    return false;
                }
                if (_clientContentLength > 0)
                {
                    if (isEncoded)
                    {
                        // url encoded form
                        if (searchStr != "")
                            searchStr += '&';
                        searchStr += plainBuf;
                    }
                    _parseArguments(searchStr);
                    if (!isEncoded)
                    {
                        // plain post json or other data
                        RequestArgument &arg = _currentArgs[_currentArgCount++];
                        arg.key = F("plain");
                        arg.value = String(plainBuf);
                    }

                    log_v("Plain: %s", plainBuf);
                    free(plainBuf);
                }
                else
                {
                    // No content - but we can still have arguments in the URL.
                    _parseArguments(searchStr);
                }
            }
            else
            {
                // it IS a form
                _parseArguments(searchStr);
                if (!_parseForm(client, boundaryStr, _clientContentLength))
                {
                    return false;
                }
            }
        }
        else
        {
            String headerName;
            String headerValue;
            // parse headers
            while (1)
            {
                req = client.readStringUntil('\r');
                client.readStringUntil('\n');
                if (req == "")
                    break; // no moar headers
                int headerDiv = req.indexOf(':');
                if (headerDiv == -1)
                {
                    break;
                }
                headerName = req.substring(0, headerDiv);
                headerValue = req.substring(headerDiv + 2);
                _collectHeader(headerName.c_str(), headerValue.c_str());

                log_v("headerName: %s", headerName.c_str());
                log_v("headerValue: %s", headerValue.c_str());

                if (headerName.equalsIgnoreCase("Host"))
                {
                    _hostHeader = headerValue;
                }
            }
            _parseArguments(searchStr);
        }
        client.flush();

        log_v("Request: %s", url.c_str());
        log_v(" Arguments: %s", searchStr.c_str());

        return true;
*/
    }
}}