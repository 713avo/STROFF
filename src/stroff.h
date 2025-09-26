#ifndef STROFF_H
#define STROFF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_TITLE_LENGTH 256
#define MAX_COMMAND_LENGTH 64
#define MAX_CHAPTERS 100
#define MAX_TABLES 50
#define MAX_LIST_ITEMS 100
#define MAX_TABLE_COLS 20
#define MAX_TABLE_ROWS 100

typedef enum {
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER,
    ALIGN_FULL
} align_t;

typedef enum {
    LIST_NONE,
    LIST_BULLET,
    LIST_NUMBER,
    LIST_RNUMBER
} list_type_t;

typedef struct {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_TITLE_LENGTH];
    char date[MAX_TITLE_LENGTH];
    int page_width;
    int page_height;
    int left_margin;
    int right_margin;
    int indent;
    int tab_size;
    align_t justify;
    int line_space;
    char header[MAX_TITLE_LENGTH];
    align_t head_align;
    char footer[MAX_TITLE_LENGTH];
    align_t foot_align;
} document_params_t;

typedef struct {
    char title[MAX_TITLE_LENGTH];
    int level;
    int page;
} chapter_t;

typedef struct {
    char name[MAX_TITLE_LENGTH];
    int page;
} table_ref_t;

typedef struct {
    int cols;
    int widths[MAX_TABLE_COLS];
    align_t aligns[MAX_TABLE_COLS];
    char name[MAX_TITLE_LENGTH];
    char headers[MAX_TABLE_COLS][MAX_TITLE_LENGTH];
    char data[MAX_TABLE_ROWS][MAX_TABLE_COLS][MAX_TITLE_LENGTH];
    int row_count;
    int tline_after_row[MAX_TABLE_ROWS];  // -1 = after headers, 0+ = after row N
    int tline_count;
} table_t;

typedef struct {
    list_type_t type;
    char bullet_char;
    int indent;
    char items[MAX_LIST_ITEMS][MAX_TITLE_LENGTH];
    int item_count;
} list_t;

typedef struct {
    document_params_t params;
    chapter_t chapters[MAX_CHAPTERS];
    int chapter_count;
    table_ref_t table_refs[MAX_TABLES];
    int table_ref_count;
    int current_page;
    int total_pages;
    int current_line;
    char current_chapter[MAX_TITLE_LENGTH];
    char current_subchap[MAX_TITLE_LENGTH];
    char current_subsubchap[MAX_TITLE_LENGTH];
    int in_document;
    int in_code_block;
    int in_chapters;
    int generate_toc;
    int generate_tot;
    list_t current_list;
    table_t current_table;
    align_t current_paragraph_align;
    int first_line_of_paragraph;
    FILE *output;
} stroff_context_t;

void init_context(stroff_context_t *ctx);
void process_file(stroff_context_t *ctx, const char *filename);
void process_line(stroff_context_t *ctx, const char *line);
void process_command(stroff_context_t *ctx, const char *line);
void process_text(stroff_context_t *ctx, const char *text);
void output_text(stroff_context_t *ctx, const char *text, align_t align);
void output_list_item(stroff_context_t *ctx, const char *prefix, const char *text);
void output_header(stroff_context_t *ctx);
void output_footer(stroff_context_t *ctx);
void output_toc(stroff_context_t *ctx);
void output_tot(stroff_context_t *ctx);
void new_page(stroff_context_t *ctx);
void check_page_break(stroff_context_t *ctx, int lines_needed);
void output_line(stroff_context_t *ctx, const char *text);
char *trim_whitespace(char *str);
char *extract_string_param(const char *line, const char *param);
int extract_int_param(const char *line, const char *param);
align_t parse_align(const char *align_str);
int utf8_display_width(const char *str);
void substitute_variables(stroff_context_t *ctx, char *text);

#endif