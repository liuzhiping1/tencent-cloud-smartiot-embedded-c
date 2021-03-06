#include "tc_iot_inc.h"

int tc_iot_refresh_auth_token(long timestamp, long nonce, tc_iot_device_info* p_device_info, long expire) {
    char sign_out[TC_IOT_HTTP_TOKEN_REQUEST_FORM_LEN];
    char http_buffer[TC_IOT_HTTP_TOKEN_RESPONSE_LEN];
    int sign_len;
    int ret;
    char* rsp_body;
    tc_iot_http_client *p_http_client, http_client;

    jsmn_parser p;
    jsmntok_t t[20];

    char temp_buf[TC_IOT_HTTP_MAX_URL_LENGTH];
    int returnCodeIndex = 0;
    char num_buf[25];
    int expire_index;
    long ret_expire;
    int password_index;
    int r;
    int username_index;
    const int timeout_ms = TC_IOT_API_TIMEOUT_MS;
    bool secured = false;
    uint16_t port = HTTP_DEFAULT_PORT;
#if defined(ENABLE_TLS)
    secured = true;
    port = HTTPS_DEFAULT_PORT;
#endif


    if (expire > TC_IOT_TOKEN_MAX_EXPIRE_SECOND) {
        TC_IOT_LOG_WARN("expire=%d to large, setting to max value = %d", (int)expire, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);
        expire = TC_IOT_TOKEN_MAX_EXPIRE_SECOND;
    }

    IF_NULL_RETURN(p_device_info, TC_IOT_NULL_POINTER);


    sign_len = tc_iot_create_auth_request_form(
        sign_out, sizeof(sign_out), p_device_info->device_secret,
         p_device_info->client_id,
         p_device_info->device_name,
         expire, nonce,
        p_device_info->product_id,
        timestamp);

    tc_iot_mem_usage_log("sign_out[TC_IOT_HTTP_TOKEN_REQUEST_FORM_LEN]", sizeof(sign_out), sign_len);

    TC_IOT_LOG_TRACE("signed request form:\n%s", sign_out);

    p_http_client = &http_client;
    tc_iot_http_client_init(p_http_client, HTTP_POST);
    tc_iot_http_client_set_body(p_http_client, sign_out);
    tc_iot_http_client_set_host(p_http_client, p_device_info->http_host);
    tc_iot_http_client_set_abs_path(p_http_client, TC_IOT_API_TOKEN_PATH);
    tc_iot_http_client_set_content_type(p_http_client, HTTP_CONTENT_FORM_URLENCODED);

    tc_iot_http_client_format_buffer(http_buffer, sizeof(http_buffer), p_http_client);

    TC_IOT_LOG_TRACE("http_buffer:\n%s", http_buffer);
    ret = tc_iot_http_client_perform(http_buffer,strlen(http_buffer), sizeof(http_buffer),
                                     p_device_info->http_host, port, secured, timeout_ms);
    tc_iot_mem_usage_log("http_buffer[TC_IOT_HTTP_TOKEN_RESPONSE_LEN]", sizeof(http_buffer), strlen(http_buffer));

    if (ret < 0) {
        return ret;
    }

    rsp_body = http_buffer;

    if (rsp_body) {
        jsmn_init(&p);

        TC_IOT_LOG_TRACE("\nbody:\n%s\n", rsp_body);

        r = jsmn_parse(&p, rsp_body, strlen(rsp_body), t,
                       sizeof(t) / sizeof(t[0]));
        if (r < 0) {
            TC_IOT_LOG_ERROR("Failed to parse JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }

        if (r < 1 || t[0].type != JSMN_OBJECT) {
            TC_IOT_LOG_ERROR("Invalid JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }

        returnCodeIndex = tc_iot_json_find_token(rsp_body, t, r, "returnCode",
                                                 temp_buf, sizeof(temp_buf));
        if (returnCodeIndex <= 0 || strlen(temp_buf) != 1 ||
            temp_buf[0] != '0') {
            TC_IOT_LOG_ERROR("failed to fetch token %d/%s: %s", returnCodeIndex,
                      temp_buf, rsp_body);
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        username_index = tc_iot_json_find_token(rsp_body, t, r, "data.id",
                                                p_device_info->username,
                                                TC_IOT_MAX_USER_NAME_LEN);
        if (username_index <= 0) {
            TC_IOT_LOG_TRACE("data.id not found in response.");
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        password_index = tc_iot_json_find_token(rsp_body, t, r, "data.secret",
                                                p_device_info->password,
                                                TC_IOT_MAX_PASSWORD_LEN);
        if (password_index <= 0) {
            TC_IOT_LOG_TRACE("data.secret not found in response.");
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        expire_index = tc_iot_json_find_token(rsp_body, t, r, "data.expire",
                                              num_buf, sizeof(num_buf));
        if (expire_index <= 0) {
            TC_IOT_LOG_TRACE("data.expire not found in response.");
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        ret_expire = atol(num_buf);
        p_device_info->token_expire_time = timestamp + ret_expire;

        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
    }
}

