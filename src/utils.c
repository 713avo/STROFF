#include "stroff.h"

char *trim_whitespace(char *str) {
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return str;
}

char *extract_string_param(const char *line, const char *param) {
    const char *param_pos = strstr(line, param);
    if (!param_pos) return NULL;

    const char *quote_start = strchr(param_pos, '"');
    if (!quote_start) return NULL;

    quote_start++;
    const char *quote_end = strchr(quote_start, '"');
    if (!quote_end) return NULL;

    int len = quote_end - quote_start;
    char *result = malloc(len + 1);
    strncpy(result, quote_start, len);
    result[len] = '\0';

    return result;
}

int extract_int_param(const char *line, const char *param) {
    const char *param_pos = strstr(line, param);
    if (!param_pos) return 0;

    param_pos += strlen(param);
    while (*param_pos && !isdigit(*param_pos) && *param_pos != '-') {
        param_pos++;
    }

    return atoi(param_pos);
}

align_t parse_align(const char *align_str) {
    if (strncmp(align_str, "LEFT", 4) == 0) {
        return ALIGN_LEFT;
    } else if (strncmp(align_str, "RIGHT", 5) == 0) {
        return ALIGN_RIGHT;
    } else if (strncmp(align_str, "CENTER", 6) == 0) {
        return ALIGN_CENTER;
    } else if (strncmp(align_str, "FULL", 4) == 0) {
        return ALIGN_FULL;
    }
    return ALIGN_LEFT;
}

int utf8_display_width(const char *str) {
    int width = 0;
    const unsigned char *s = (const unsigned char *)str;

    while (*s) {
        if (*s < 0x80) {
            // ASCII character (1 byte)
            s++;
            width++;
        } else if (*s < 0xC0) {
            // Invalid UTF-8 continuation byte, skip
            s++;
        } else if (*s < 0xE0) {
            // 2-byte UTF-8 character
            s += 2;
            width++;
        } else if (*s < 0xF0) {
            // 3-byte UTF-8 character
            s += 3;
            width++;
        } else {
            // 4-byte UTF-8 character
            s += 4;
            width++;
        }
    }

    return width;
}