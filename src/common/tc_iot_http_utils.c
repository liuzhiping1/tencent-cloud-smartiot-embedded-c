#include "tc_iot_inc.h"

int tc_iot_http_request_init(tc_iot_http_request* request, const char* method,
                             const char* abs_path, int abs_path_len,
                             const char* http_version) {
    char* current = NULL;
    int buffer_left;
    int ret;

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(abs_path, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(http_version, TC_IOT_NULL_POINTER);
    tc_iot_yabuffer_reset(&(request->buf));

    current = tc_iot_yabuffer_current(&(request->buf));
    buffer_left = tc_iot_yabuffer_left(&(request->buf));
    ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_REQUEST_LINE_FMT,
                              method, abs_path, http_version);
    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }
    return ret;
}

int tc_iot_http_request_append_header(tc_iot_http_request* request,
                                      const char* header, const char* val) {
    char* current;
    int buffer_left, ret;

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(header, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(val, TC_IOT_NULL_POINTER);

    current = tc_iot_yabuffer_current(&(request->buf));
    buffer_left = tc_iot_yabuffer_left(&(request->buf));
    ret = tc_iot_hal_snprintf(current, buffer_left, "%s: %s\r\n", header, val);
    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }

    return ret;
}

int tc_iot_http_request_n_append_header(tc_iot_http_request* request,
                                        const char* header, const char* val,
                                        int val_len) {
    char* current = NULL;
    int buffer_left = 0;
    int ret = 0;
    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(header, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(val, TC_IOT_NULL_POINTER);

    current = tc_iot_yabuffer_current(&(request->buf));
    buffer_left = tc_iot_yabuffer_left(&(request->buf));
    ret = tc_iot_hal_snprintf(current, buffer_left, "%s: ", header);
    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }

    current = tc_iot_yabuffer_current(&(request->buf));
    buffer_left = tc_iot_yabuffer_left(&(request->buf));
    if (buffer_left > val_len  + 2) {
        memcpy(current, val, val_len);
        memcpy(current + val_len, "\r\n", 2);
        tc_iot_yabuffer_forward(&(request->buf), val_len + 2);
        ret += val_len + 2;
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }

    return ret;
}

int tc_iot_http_request_append_body(tc_iot_http_request* request,
                                    const char* body) {
    char* current = tc_iot_yabuffer_current(&(request->buf));
    int buffer_left = tc_iot_yabuffer_left(&(request->buf));

    int ret;

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    if (body) {
        ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_BODY_FMT, body);
    } else {
        ret = tc_iot_hal_snprintf(current, buffer_left, HTTP_BODY_FMT, "");
    }

    if (ret > 0) {
        tc_iot_yabuffer_forward(&(request->buf), ret);
    }

    return ret;
}

int tc_iot_create_http_request(tc_iot_http_request* request, const char* host,
                               int host_len, const char* method,
                               const char* abs_path, int abs_path_len,
                               const char* http_version, const char* user_agent,
                               const char* content_type, const char* body) {
    int body_len;
    char body_len_str[20];

    IF_NULL_RETURN(request, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(host, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(abs_path, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(http_version, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(user_agent, TC_IOT_NULL_POINTER);

    tc_iot_http_request_init(request, method, abs_path, abs_path_len,
                             http_version);
    tc_iot_http_request_append_header(request, HTTP_HEADER_USER_AGENT,
                                      user_agent);
    tc_iot_http_request_n_append_header(request, HTTP_HEADER_HOST, host,
                                        host_len);
    tc_iot_http_request_append_header(request, HTTP_HEADER_ACCEPT, "*/*");
    if (content_type) {
        tc_iot_http_request_append_header(request, HTTP_HEADER_CONTENT_TYPE,
                                          content_type);
    }
    tc_iot_http_request_append_header(
        request, HTTP_HEADER_ACCEPT_ENCODING,
        "identity"); /* accept orignal content only, no zip */

    if (body) {
        body_len = strlen(body);
        if (body_len) {
            tc_iot_hal_snprintf(body_len_str, sizeof(body_len_str), "%d",
                                body_len);
            tc_iot_http_request_append_header(
                request, HTTP_HEADER_CONTENT_LENGTH, body_len_str);
        }
    }
    tc_iot_http_request_append_body(request, body);
    return TC_IOT_SUCCESS;
}

int tc_iot_create_post_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len,
                               const char* body, const char * content_type) {
    return tc_iot_create_http_request(request, host, host_len, HTTP_POST, abs_path,
                               abs_path_len, HTTP_VERSION_1_0,TC_IOT_USER_AGENT,
                               content_type, body);
}

int tc_iot_create_get_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len) {
    return tc_iot_create_http_request(request, host, host_len, HTTP_GET, abs_path,
                               abs_path_len, HTTP_VERSION_1_0,TC_IOT_USER_AGENT,
                               NULL, NULL);
}

int tc_iot_create_head_request(tc_iot_http_request* request,
                               const char* abs_path, int abs_path_len,
                               const char* host, int host_len) {
    return tc_iot_create_http_request(request, host, host_len, HTTP_HEAD, abs_path,
                               abs_path_len, HTTP_VERSION_1_0,TC_IOT_USER_AGENT,
                               NULL, NULL);
}

static int add_tc_iot_url_encoded_field(tc_iot_yabuffer_t* buffer,
                                        const char* prefix, const char* val,
                                        int val_len) {
    int total = 0;
    int ret = 0;

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(prefix, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(val, TC_IOT_NULL_POINTER);

    total = tc_iot_yabuffer_append(buffer, prefix);
    ret = tc_iot_url_encode(val, val_len, tc_iot_yabuffer_current(buffer),
                            tc_iot_yabuffer_left(buffer));

    tc_iot_yabuffer_forward(buffer, ret);
    total += ret;
    return total;
}

static int add_url_long_field(tc_iot_yabuffer_t* buffer, const char* prefix,
                              long val) {
    int total = 0;
    int ret;
    char* current;
    int buffer_left;

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(prefix, TC_IOT_NULL_POINTER);

    total = tc_iot_yabuffer_append(buffer, prefix);
    current = tc_iot_yabuffer_current(buffer);
    buffer_left = tc_iot_yabuffer_left(buffer);

    ret = tc_iot_hal_snprintf(current, buffer_left, "%d", (int)val);

    if (ret > 0) {
        tc_iot_yabuffer_forward(buffer, ret);
        total += ret;
        return total;
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
}

int tc_iot_create_auth_request_form(char* form, int max_form_len,
                                    const char* secret,
                                    const char* client_id,
                                    const char* device_name,
                                    long expire,
                                    long nonce,
                                    const char* product_id,
                                    long timestamp) {
    tc_iot_yabuffer_t form_buf;
    int total = 0;

    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(client_id, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    tc_iot_yabuffer_init(&form_buf, form, max_form_len);
    total += add_tc_iot_url_encoded_field(&form_buf, "clientId=", client_id,
                                          strlen(client_id));
    total += add_tc_iot_url_encoded_field(&form_buf, "&deviceName=",
                                          device_name, strlen(device_name));
    total += add_url_long_field(&form_buf, "&expire=", expire);
    total += add_url_long_field(&form_buf, "&nonce=", nonce);
    total += add_tc_iot_url_encoded_field(&form_buf, "&productId=", product_id,
                                          strlen(product_id));
    total += add_url_long_field(&form_buf, "&timestamp=", timestamp);
    total += add_tc_iot_url_encoded_field(&form_buf, "&signature=", "", 0);

    total += tc_iot_calc_auth_sign(
        tc_iot_yabuffer_current(&form_buf), tc_iot_yabuffer_left(&form_buf),
        secret, client_id, device_name,
        expire, nonce, product_id, timestamp);
    return total;
}

int tc_iot_create_active_device_form(char* form, int max_form_len,
                                    const char* product_secret, 
                                    const char* device_name,  
                                    const char* product_id,
                                    long nonce, long timestamp) {
    tc_iot_yabuffer_t form_buf;
    int total = 0;
    
    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_secret, TC_IOT_NULL_POINTER);
    
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    tc_iot_yabuffer_init(&form_buf, form, max_form_len);
    
    total += add_tc_iot_url_encoded_field(&form_buf, "productId=", product_id,
                                      strlen(product_id));
    total += add_tc_iot_url_encoded_field(&form_buf, "&deviceName=",
                                          device_name, strlen(device_name));
    
    total += add_url_long_field(&form_buf, "&nonce=", nonce);

    total += add_url_long_field(&form_buf, "&timestamp=", timestamp);
    total += add_tc_iot_url_encoded_field(&form_buf, "&signature=", "", 0);

    total += tc_iot_calc_active_device_sign(
        tc_iot_yabuffer_current(&form_buf), tc_iot_yabuffer_left(&form_buf),
        product_secret,  
        device_name, product_id, 
        nonce, timestamp);
    return total;
}

int tc_iot_parse_http_response_code(const char * resp) {
    int ret;
    int i;

    IF_NULL_RETURN(resp, TC_IOT_NULL_POINTER);
    if (strncmp(HTTP_RESPONSE_STATE_PREFIX, resp, HTTP_RESPONSE_STATE_PREFIX_LEN) != 0) {
        return TC_IOT_HTTP_RESPONSE_INVALID;
    }

    resp+= HTTP_RESPONSE_STATE_PREFIX_LEN;
    if (*resp != '0' && *resp != '1') {
        TC_IOT_LOG_TRACE("http minor version invalid: %s", tc_iot_log_summary_string(resp, 5));
        return TC_IOT_HTTP_RESPONSE_INVALID;
    }
    resp++;
    if (*resp != ' ') {
        TC_IOT_LOG_TRACE("http stat line invalid: %s", tc_iot_log_summary_string(resp, 5));
        return TC_IOT_HTTP_RESPONSE_INVALID;
    }
    resp++;
    ret = 0;
    for ( i = 0; i < 3; i++, resp++) {
        if (*resp < '0') {
            return TC_IOT_HTTP_RESPONSE_INVALID;
        }
        if (*resp > '9') {
            return TC_IOT_HTTP_RESPONSE_INVALID;
        }
        ret = ret*10 +((*resp) - '0');
    }

    return ret;
}

int tc_iot_http_get(tc_iot_network_t* network,
                         tc_iot_http_request* request,
                         const char* url, 
                         int timeout_ms, const char * extra_header) {
    tc_iot_url_parse_result_t result;
    char temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH];
    int written_len;
    int ret = tc_iot_url_parse(url, strlen(url), &result);

    if (ret < 0) {
        return ret;
    }

    if (result.host_len >= sizeof(temp_host)) {
        TC_IOT_LOG_ERROR("host address too long.");
        return -1;
    }

    if (result.over_tls != network->net_context.use_tls) {
        TC_IOT_LOG_WARN("network type not match: url tls=%d, context tls=%d",
                 (int)result.over_tls, (int)network->net_context.use_tls);
        return -1;
    }

    strncpy(temp_host, url + result.host_start, result.host_len);
    temp_host[result.host_len] = '\0';
    tc_iot_mem_usage_log("temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH]", sizeof(temp_host), result.host_len);

    TC_IOT_LOG_TRACE("remote=%s:%d", temp_host, result.port);

    network->do_connect(network, temp_host, result.port);

    if (result.path_len) {
        ret = tc_iot_create_get_request(
            request, url + result.path_start, result.path_len,
            url + result.host_start, result.host_len);
    } else {
        ret = tc_iot_create_get_request(request, "/", 1, url + result.host_start,
                                       result.host_len);
    }

    if (extra_header != NULL && extra_header[0] != '\0') {
        tc_iot_yabuffer_forward(&request->buf, -2);
        /* Range: bytes=%d- */
        ret = tc_iot_hal_snprintf(tc_iot_yabuffer_current(&request->buf), 
                tc_iot_yabuffer_left(&request->buf),
                "%s\r\n\r\n", extra_header
                );
        tc_iot_yabuffer_forward(&request->buf, ret);
        if (tc_iot_yabuffer_left(&request->buf) <= 0) {
            TC_IOT_LOG_ERROR("request buffer size=%d not enough", 
                    tc_iot_yabuffer_len(&request->buf));
            return TC_IOT_BUFFER_OVERFLOW;
        }
    }
    written_len = network->do_write(network, (unsigned char *)request->buf.data,
                                    request->buf.pos, timeout_ms);
    TC_IOT_LOG_TRACE("request with:\n%s", request->buf.data);
    if (written_len == request->buf.pos) {
        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_FAILURE;
    }
}

int tc_iot_http_head(tc_iot_network_t* network,
                         tc_iot_http_request* request, const char* url,
                         int timeout_ms) {
    tc_iot_url_parse_result_t result;
    char temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH];
    int written_len;
    int ret = tc_iot_url_parse(url, strlen(url), &result);

    if (ret < 0) {
        return ret;
    }

    if (result.path_len) {
        ret = tc_iot_create_head_request(
            request, url + result.path_start, result.path_len,
            url + result.host_start, result.host_len);
    } else {
        ret =
            tc_iot_create_head_request(request, "/", 1, url + result.host_start,
                                       result.host_len);
    }

    if (result.host_len >= sizeof(temp_host)) {
        TC_IOT_LOG_ERROR("host address too long.");
        return -1;
    }

    tc_iot_mem_usage_log("temp_host[TC_IOT_HTTP_MAX_HOST_LENGTH]", sizeof(temp_host), result.host_len);

    if (result.over_tls != network->net_context.use_tls) {
        TC_IOT_LOG_WARN("network type not match: url tls=%d, context tls=%d",
                 (int)result.over_tls, (int)network->net_context.use_tls);
        return -1;
    }

    strncpy(temp_host, url + result.host_start, result.host_len);
    temp_host[result.host_len] = '\0';

    TC_IOT_LOG_TRACE("remote=%s:%d", temp_host, result.port);

    network->do_connect(network, temp_host, result.port);
    written_len = network->do_write(network, (unsigned char *)request->buf.data,
                                    request->buf.pos, timeout_ms);
    TC_IOT_LOG_TRACE("request with:\n%s", request->buf.data);
    if (written_len == request->buf.pos) {
        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_FAILURE;
    }
}


bool tc_iot_http_has_line_ended(const char * str) {
    while (*str) {
        if (*str == '\r') {
            return true;
        }
        str ++;
    }
    return false;
}

void tc_iot_http_parser_init(tc_iot_http_response_parser * parser) {
    if (parser) {
        parser->state = _PARSER_START;
        parser->version = 0;
        parser->status_code = 0;
        parser->content_length = 0;
        parser->location = NULL;
        /* parser->body = NULL; */
    }
}

int tc_iot_http_parser_analysis(tc_iot_http_response_parser * parser, const char * buffer, int buffer_len) {
    bool header_complete = false;
    const char * pos = NULL;
    int buffer_parsed = 0;
    int i = 0;

    IF_NULL_RETURN(parser, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);

    pos = buffer;
start:
    if (buffer_parsed >= buffer_len) {
        return buffer_parsed;
    }

    switch(parser->state) {
        case _PARSER_START:
            /* Head should alwarys start with: HTTP/1.x COD */
		    if ( buffer_len < sizeof("HTTP/1.x M0N")) {
                return 0;
            }

            if (!tc_iot_str7equal(pos, 'H', 'T', 'T', 'P', '/', '1', '.')) {
                TC_IOT_LOG_ERROR("Response header not start with HTTP/1.[x] .");
                return TC_IOT_FAILURE;
            } else {
                buffer_parsed += 7;
                pos = buffer + buffer_parsed;
            }

            if (pos[0] != '0' && pos[0] != '1') {
                TC_IOT_LOG_ERROR("HTTP version 1.%c not supported", pos[0]);
                return TC_IOT_FAILURE;
            } else {
                parser->version = pos[0] - '0';
            }

            if (' ' != pos[1]) {
                TC_IOT_LOG_ERROR("space not found");
                return TC_IOT_FAILURE;
            }

            buffer_parsed += 2;
            pos = buffer + buffer_parsed;
            parser->status_code = 0;
            for (i = 0; i < 3; i++) {
                if (pos[i] > '9' || pos[i] < '0') {
                    return TC_IOT_HTTP_INVALID_STATUS_CODE;
                } else {
                    parser->status_code = parser->status_code * 10 + pos[i] - '0';
                }
            }
            TC_IOT_LOG_TRACE("version: 1.%d, status code: %d",parser->version, parser->status_code);
            buffer_parsed += 3;
            pos = buffer + buffer_parsed;
            parser->state = _PARSER_IGNORE_TO_RETURN_CHAR;
            goto start;
        case _PARSER_IGNORE_TO_RETURN_CHAR:
            while (buffer_parsed < buffer_len) {
                if ('\r' == (*pos)) {
                    buffer_parsed++;
                    pos++;
                    parser->state = _PARSER_SKIP_NEWLINE_CHAR;
                    goto start;
                    break;
                }
                buffer_parsed++;
                pos++;
            }
            return buffer_parsed;
        case _PARSER_SKIP_NEWLINE_CHAR:
            if ('\n' != *pos) {
                TC_IOT_LOG_ERROR("expecting \\n");
                return TC_IOT_FAILURE;
            }
            buffer_parsed++;
            pos++;
            parser->state = _PARSER_HEADER;
            goto start;
        case _PARSER_HEADER:
            /* TC_IOT_LOG_TRACE("pos=%s",pos); */
            if ('\r' == (*pos)) {
                /* status line\r\n */
                /* http headers\r\n */
                /* \r\nbody */
                /* ^ */
                /* | we are here */
                buffer_parsed += 1;
                pos = buffer + buffer_parsed;
                parser->state = _PARSER_IGNORE_TO_BODY_START;
                goto start;
            } else {
                for (i = 0; i < (buffer_len-buffer_parsed); i++) {
                    if (':' == pos[i]) {
                        if ((i == tc_iot_const_str_len(HTTP_HEADER_CONTENT_LENGTH)) 
                                && (0 == memcmp(pos, HTTP_HEADER_CONTENT_LENGTH, i))) {
                            header_complete = tc_iot_http_has_line_ended(pos+i+1);
                            if (header_complete) {
                                /* TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_CONTENT_LENGTH, pos+i+2); */
                                parser->content_length = tc_iot_try_parse_int(pos+i+2, NULL);
                            } else {
                                TC_IOT_LOG_TRACE("%s not complete, continue reading:%s",HTTP_HEADER_CONTENT_LENGTH, pos+i+2);
                                return buffer_parsed;
                            }

                        } else if ((i == tc_iot_const_str_len(HTTP_HEADER_LOCATION)) 
                                && (0 == memcmp(pos, HTTP_HEADER_LOCATION, i))) {
                            header_complete = tc_iot_http_has_line_ended(pos+i+1);
                            if (header_complete) {
                                TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_LOCATION, pos+i+2);
                                parser->location = pos+i+2;
                            } else {
                                TC_IOT_LOG_TRACE("%s not complete, continue reading:%s",HTTP_HEADER_LOCATION, pos+i+2);
                                return buffer_parsed;
                            }
                        } else if ((i == tc_iot_const_str_len(HTTP_HEADER_CONTENT_TYPE)) 
                                && (0 == memcmp(pos, HTTP_HEADER_CONTENT_TYPE, i))) {
                            /* TC_IOT_LOG_TRACE("%s found:%s",HTTP_HEADER_CONTENT_TYPE, pos+i+2); */
                        } else {
                            /* TC_IOT_LOG_TRACE("ignore i=%d,pos=%s",i, pos); */
                        }
                        buffer_parsed += i+1;
                        pos = buffer + buffer_parsed;
                        parser->state = _PARSER_IGNORE_TO_RETURN_CHAR;
                        goto start;
                    }
                }
                TC_IOT_LOG_TRACE("buffer_parsed=%d, buffer_len=%d,left=%s", buffer_parsed, buffer_len, pos);
                return buffer_parsed;
            }
            break;
        case _PARSER_IGNORE_TO_BODY_START:
            if ('\n' != *pos) {
                TC_IOT_LOG_ERROR("expecting \\n");
                return TC_IOT_FAILURE;
            }
            buffer_parsed++;
            pos++;
            /* TC_IOT_LOG_TRACE("body=%s", pos); */
            parser->state = _PARSER_END;
            /* parser->body = pos; */
            return buffer_parsed;
        case _PARSER_END:
            return buffer_len;
        default:
            TC_IOT_LOG_ERROR("invalid parse state=%d", parser->state);
            return TC_IOT_FAILURE;
    }
}


int tc_iot_http_client_init(tc_iot_http_client * c, const char * method) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);

    memset(c, 0, sizeof(*c));
    if (!method) {
        c->method = HTTP_POST;
    } else {
        c->method = method;
    }
    c->version = HTTP_VERSION_1_1;
    c->abs_path = "/";
    c->extra_headers = "";
    c->content_type = HTTP_CONTENT_FORM_URLENCODED;

    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_version(tc_iot_http_client * c, const char * version) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->version = version;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_host(tc_iot_http_client * c, const char * host) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->host = host;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_abs_path(tc_iot_http_client * c, const char * abs_path) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->abs_path = abs_path;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_content_type(tc_iot_http_client * c, const char * content_type) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->content_type = content_type;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_extra_headers(tc_iot_http_client * c, const char * extra_headers) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->extra_headers = extra_headers;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_set_body(tc_iot_http_client * c, const char * body) {
    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    c->body = body;
    return TC_IOT_SUCCESS;
}

int tc_iot_http_client_format_buffer(char * buffer, int buffer_len, tc_iot_http_client * c) {
    int ret = 0;
    int content_length = 0;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_LESS_RETURN(buffer_len, 0, TC_IOT_INVALID_PARAMETER);

    if (c->body) {
        content_length = strlen(c->body);
    }

    ret = tc_iot_hal_snprintf(buffer, buffer_len,
                              "%s %s HTTP/%s\r\n"
                              "User-Agent: tciotclient/1.0\r\n"
                              "Host: %s\r\n"
                              "Accept: */*\r\n"
                              "Content-Type: %s\r\n"
                              "Accept-Encoding: identity\r\n"
                              "Content-Length: %d\r\n%s\r\n%s",
                              c->method, c->abs_path, c->version,
                              c->host,c->content_type,content_length,
                              c->extra_headers,c->body);
    if (ret > 0) {
        return ret;
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
}


int tc_iot_http_client_perform(char * buffer, int buffer_used, int buffer_len,
                            const char * host, uint16_t port,
                            bool secured, int timeout_ms) {
    const int timer_tick = 100;
    int ret = 0;
    int write_ret = 0;
    int read_ret  = 0;
    int parse_ret = 0;
    tc_iot_network_t network;
    tc_iot_timer timer;
    int parse_left = 0;
    int content_length = 0;
    int received_bytes = 0;
    tc_iot_http_response_parser parser;

    ret = tc_iot_network_prepare(&network, TC_IOT_SOCK_STREAM, TC_IOT_PROTO_HTTP, secured);
    if (ret < 0) {
        return ret;
    }

    tc_iot_hal_timer_init(&timer);
    tc_iot_hal_timer_countdown_ms(&timer,timeout_ms);
    ret = network.do_connect(&network, host, port);
    write_ret = network.do_write(&network, (unsigned char *)buffer, buffer_used, timeout_ms);
    if (write_ret != buffer_used) {
        TC_IOT_LOG_ERROR("send packet failed: expect len=%d, write return=%d", buffer_used, write_ret)
        return TC_IOT_SEND_PACK_FAILED;
    }

    tc_iot_http_parser_init(&parser);
    do {
        read_ret = network.do_read(&network, (unsigned char *)buffer+parse_left, buffer_len-parse_left, timer_tick);
        if (read_ret > 0) {
            TC_IOT_LOG_TRACE("read_ret=%d,parse_left=%d", read_ret, parse_left);

            read_ret += parse_left;
            if (read_ret < buffer_len) {
                buffer[read_ret] = '0';
            }

            parse_ret = tc_iot_http_parser_analysis(&parser, buffer, read_ret);
            if (parse_ret < 0) {
                TC_IOT_LOG_ERROR("read from request host=%s:%d failed, ret=%d", host, port, ret);
                network.do_disconnect(&network);
                return parse_ret;
            }

            if (parse_ret > read_ret) {
                TC_IOT_LOG_ERROR("tc_iot_http_parser_analysis parse_ret=%d too large, ret=%d", parse_ret, ret);
                network.do_disconnect(&network);
                return TC_IOT_FAILURE;
            }

            parse_left = read_ret - parse_ret;
            if (parse_left > 0) {
                memmove(buffer, buffer+parse_ret, parse_left);
                buffer[parse_left] = '\0';
            }

            if (_PARSER_END == parser.state) {
                content_length = parser.content_length;
                received_bytes = parse_left;
                TC_IOT_LOG_TRACE("ver=1.%d, code=%d,content_length=%d, received_bytes=%d",
                                 parser.version, parser.status_code, parser.content_length, received_bytes);
                if (content_length > buffer_len) {
                    TC_IOT_LOG_ERROR("buffer not enough: content_length=%d, buffer_len=%d", content_length, buffer_len);
                    network.do_disconnect(&network);
                    return TC_IOT_BUFFER_OVERFLOW;
                }

                while (received_bytes < content_length) {
                    read_ret = network.do_read(&network, (unsigned char *)buffer+parse_left, buffer_len-parse_left, timer_tick);
                    if (read_ret > 0) {
                        received_bytes += read_ret;
                        TC_IOT_LOG_TRACE("read=%d,total_read=%d, total=%d", read_ret, received_bytes, content_length);
                    } else if (read_ret == TC_IOT_NET_NOTHING_READ) {
                        continue;
                    } else {
                        TC_IOT_LOG_ERROR("read buffer error:%d", read_ret);
                        network.do_disconnect(&network);
                        return read_ret;
                    }
                }
                network.do_disconnect(&network);
                return received_bytes;
            }
        } else if (read_ret == TC_IOT_NET_NOTHING_READ) {
            continue;
        } else {
            TC_IOT_LOG_ERROR("read buffer error:%d", read_ret);
            network.do_disconnect(&network);
            return read_ret;
        }

        if (read_ret <= 0) {
            TC_IOT_LOG_TRACE("ret=%d, len=%d", read_ret, buffer_len);
            network.do_disconnect(&network);
            return TC_IOT_SUCCESS;
        }

    } while(!tc_iot_hal_timer_is_expired(&timer) && (received_bytes < buffer_len));

    network.do_disconnect(&network);

    if (received_bytes > 0) {
        return received_bytes;
    } else {
        return read_ret;
    }
}

