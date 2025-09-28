#include "stroff.h"

static int is_absolute_path(const char *path) {
    if (!path || !path[0]) {
        return 0;
    }

#ifdef _WIN32
    if (path[0] == '\\' || path[0] == '/') {
        return 1;
    }
    if (strlen(path) > 1 && path[1] == ':') {
        return 1;
    }
    return 0;
#else
    return path[0] == '/';
#endif
}

static void join_paths(const char *base, const char *relative, char *out) {
    if (!base || !base[0]) {
        strncpy(out, relative, MAX_PATH_LENGTH - 1);
        out[MAX_PATH_LENGTH - 1] = '\0';
        return;
    }

    size_t base_len = strlen(base);
    int has_separator = base_len > 0 && (base[base_len - 1] == '/' || base[base_len - 1] == '\\');
    if (has_separator) {
        snprintf(out, MAX_PATH_LENGTH, "%s%s", base, relative);
    } else {
        snprintf(out, MAX_PATH_LENGTH, "%s/%s", base, relative);
    }
}

static void get_directory(const char *path, char *dir_out) {
    if (!path || !path[0]) {
        dir_out[0] = '\0';
        return;
    }

    const char *last_sep = NULL;
    for (const char *p = path; *p; p++) {
        if (*p == '/' || *p == '\\') {
            last_sep = p;
        }
    }

    if (!last_sep) {
        dir_out[0] = '\0';
        return;
    }

    size_t dir_len = (size_t)(last_sep - path);

    if (dir_len == 0) {
        dir_out[0] = *last_sep;
        dir_out[1] = '\0';
        return;
    }

    if (dir_len >= MAX_PATH_LENGTH) {
        dir_len = MAX_PATH_LENGTH - 1;
    }

    strncpy(dir_out, path, dir_len);
    dir_out[dir_len] = '\0';
}

static void resolve_include_path(stroff_context_t *ctx, const char *filename, char *resolved) {
    if (is_absolute_path(filename)) {
        strncpy(resolved, filename, MAX_PATH_LENGTH - 1);
        resolved[MAX_PATH_LENGTH - 1] = '\0';
        return;
    }

    if (ctx->include_depth > 0) {
        const char *base = ctx->include_stack[ctx->include_depth - 1];
        if (base[0]) {
            join_paths(base, filename, resolved);
            return;
        }
    }

    strncpy(resolved, filename, MAX_PATH_LENGTH - 1);
    resolved[MAX_PATH_LENGTH - 1] = '\0';
}

void init_context(stroff_context_t *ctx) {
    strcpy(ctx->params.title, "");
    strcpy(ctx->params.author, "");
    strcpy(ctx->params.date, "");
    ctx->params.page_width = 80;
    ctx->params.page_height = 40;
    ctx->params.left_margin = 0;
    ctx->params.right_margin = 0;
    ctx->params.indent = 0;
    ctx->params.tab_size = 4;
    ctx->params.justify = ALIGN_LEFT;
    ctx->params.line_space = 1;
    strcpy(ctx->params.header, "");
    ctx->params.head_align = ALIGN_LEFT;
    strcpy(ctx->params.footer, "");
    ctx->params.foot_align = ALIGN_LEFT;

    ctx->chapter_count = 0;
    ctx->table_ref_count = 0;
    ctx->current_page = 1;
    ctx->total_pages = 1;
    ctx->current_line = 0;
    strcpy(ctx->current_chapter, "");
    strcpy(ctx->current_subchap, "");
    strcpy(ctx->current_subsubchap, "");
    ctx->in_document = 0;
    ctx->in_code_block = 0;
    ctx->in_chapters = 0;
    ctx->generate_toc = 0;
    ctx->generate_tot = 0;
    ctx->current_list.type = LIST_NONE;
    ctx->current_list.item_count = 0;
    ctx->current_table.row_count = 0;
    ctx->current_paragraph_align = ALIGN_LEFT;
    ctx->first_line_of_paragraph = 0;
    ctx->output = NULL;
    ctx->include_depth = 0;
    for (int i = 0; i < MAX_INCLUDE_DEPTH; i++) {
        ctx->include_stack[i][0] = '\0';
    }
}

void process_file(stroff_context_t *ctx, const char *filename) {
    char resolved_path[MAX_PATH_LENGTH];
    resolve_include_path(ctx, filename, resolved_path);

    FILE *file = fopen(resolved_path, "r");
    if (!file) {
        fprintf(stderr, "Error: No se puede abrir el archivo '%s'\n", resolved_path);
        return;
    }

    if (ctx->include_depth >= MAX_INCLUDE_DEPTH) {
        fprintf(stderr, "Error: Límite de inclusión excedido (%d niveles)\n", MAX_INCLUDE_DEPTH);
        fclose(file);
        return;
    }

    char current_dir[MAX_PATH_LENGTH];
    get_directory(resolved_path, current_dir);
    strncpy(ctx->include_stack[ctx->include_depth], current_dir, MAX_PATH_LENGTH - 1);
    ctx->include_stack[ctx->include_depth][MAX_PATH_LENGTH - 1] = '\0';
    ctx->include_depth++;

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        process_line(ctx, line);
    }

    ctx->include_depth--;
    ctx->include_stack[ctx->include_depth][0] = '\0';
    fclose(file);
}

void process_line(stroff_context_t *ctx, const char *line) {
    char working[MAX_LINE_LENGTH];
    strncpy(working, line, MAX_LINE_LENGTH - 1);
    working[MAX_LINE_LENGTH - 1] = '\0';

    char *trimmed = trim_whitespace(working);

    if (ctx->in_code_block && strcmp(trimmed, ".ECODE") != 0) {
        process_text(ctx, line);
        return;
    }

    if (strlen(trimmed) == 0) {
        return;
    }

    if (trimmed[0] == '#') {
        return;
    }

    if (trimmed[0] == '.') {
        process_command(ctx, trimmed);
    } else {
        process_text(ctx, trimmed);
    }
}

void process_command(stroff_context_t *ctx, const char *line) {
    char command[MAX_COMMAND_LENGTH];
    sscanf(line, ".%s", command);

    if (strcmp(command, "TITLE") == 0) {
        char *title = extract_string_param(line, "TITLE");
        if (title) {
            strncpy(ctx->params.title, title, MAX_TITLE_LENGTH - 1);
            free(title);
        }
    }
    else if (strcmp(command, "AUTH") == 0) {
        char *auth = extract_string_param(line, "AUTH");
        if (auth) {
            strncpy(ctx->params.author, auth, MAX_TITLE_LENGTH - 1);
            free(auth);
        }
    }
    else if (strcmp(command, "DATE") == 0) {
        char *date = extract_string_param(line, "DATE");
        if (date) {
            strncpy(ctx->params.date, date, MAX_TITLE_LENGTH - 1);
            free(date);
        }
    }
    else if (strcmp(command, "PAGEWIDTH") == 0) {
        ctx->params.page_width = extract_int_param(line, "PAGEWIDTH");
    }
    else if (strcmp(command, "PAGEHEIGHT") == 0) {
        ctx->params.page_height = extract_int_param(line, "PAGEHEIGHT");
    }
    else if (strcmp(command, "LMARGIN") == 0) {
        ctx->params.left_margin = extract_int_param(line, "LMARGIN");
    }
    else if (strcmp(command, "RMARGIN") == 0) {
        ctx->params.right_margin = extract_int_param(line, "RMARGIN");
    }
    else if (strcmp(command, "INDENT") == 0) {
        ctx->params.indent = extract_int_param(line, "INDENT");
    }
    else if (strcmp(command, "TABSIZE") == 0) {
        ctx->params.tab_size = extract_int_param(line, "TABSIZE");
    }
    else if (strcmp(command, "JUSTIFY") == 0) {
        const char *align_start = strstr(line, "JUSTIFY") + 7;
        while (*align_start && isspace(*align_start)) align_start++;
        ctx->params.justify = parse_align(align_start);
    }
    else if (strcmp(command, "LINESPACE") == 0) {
        ctx->params.line_space = extract_int_param(line, "LINESPACE");
    }
    else if (strcmp(command, "HEADER") == 0) {
        char *header = extract_string_param(line, "HEADER");
        if (header) {
            strncpy(ctx->params.header, header, MAX_TITLE_LENGTH - 1);
            free(header);
        }
    }
    else if (strcmp(command, "HEADALIGN") == 0) {
        const char *align_start = strstr(line, "HEADALIGN") + 9;
        while (*align_start && isspace(*align_start)) align_start++;
        ctx->params.head_align = parse_align(align_start);
    }
    else if (strcmp(command, "FOOTER") == 0) {
        char *footer = extract_string_param(line, "FOOTER");
        if (footer) {
            strncpy(ctx->params.footer, footer, MAX_TITLE_LENGTH - 1);
            free(footer);
        }
    }
    else if (strcmp(command, "FOOTALIGN") == 0) {
        const char *align_start = strstr(line, "FOOTALIGN") + 9;
        while (*align_start && isspace(*align_start)) align_start++;
        ctx->params.foot_align = parse_align(align_start);
    }
    else if (strcmp(command, "DOCUMENT") == 0) {
        ctx->in_document = 1;
        ctx->current_line = 0;

        // Header solo en capítulos, no en página de título
        if (strlen(ctx->params.header) > 0 && ctx->in_chapters) {
            output_header(ctx);
            fprintf(ctx->output, "\n");
            ctx->current_line++;
        }

        fprintf(ctx->output, "\n");
        ctx->current_line++;
        if (strlen(ctx->params.title) > 0) {
            output_text(ctx, ctx->params.title, ALIGN_CENTER);
            check_page_break(ctx, 1);
            fprintf(ctx->output, "\n");
            ctx->current_line++;
        }
        if (strlen(ctx->params.author) > 0) {
            output_text(ctx, ctx->params.author, ALIGN_CENTER);
            check_page_break(ctx, 1);
            fprintf(ctx->output, "\n");
            ctx->current_line++;
        }
        if (strlen(ctx->params.date) > 0) {
            output_text(ctx, ctx->params.date, ALIGN_CENTER);
            check_page_break(ctx, 1);
            fprintf(ctx->output, "\n");
            ctx->current_line++;
        }
        check_page_break(ctx, 1);
        fprintf(ctx->output, "\n");
        ctx->current_line++;
    }
    else if (strcmp(command, "EDOC") == 0) {
        // Procesar la última página antes de cerrar documento
        if (strlen(ctx->params.footer) > 0) {
            // Llenar líneas hasta el final de la página
            while (ctx->current_line < ctx->params.page_height - 3) {
                fprintf(ctx->output, "\n");
                ctx->current_line++;
            }
            output_footer(ctx);
        } else {
            // Sin footer, llenar hasta el final completo
            while (ctx->current_line < ctx->params.page_height - 1) {
                fprintf(ctx->output, "\n");
                ctx->current_line++;
            }
        }
        ctx->in_document = 0;
    }
    else if (strcmp(command, "MAKETOC") == 0) {
        output_toc(ctx);
    }
    else if (strcmp(command, "MAKETOT") == 0) {
        output_tot(ctx);
    }
    else if (strcmp(command, "PAGEBREAK") == 0) {
        new_page(ctx);
    }
    else if (strcmp(command, "CHAP") == 0) {
        char *title = extract_string_param(line, "CHAP");
        if (title && ctx->chapter_count < MAX_CHAPTERS) {
            ctx->in_chapters = 1;
            check_page_break(ctx, 4);

            strncpy(ctx->chapters[ctx->chapter_count].title, title, MAX_TITLE_LENGTH - 1);
            ctx->chapters[ctx->chapter_count].level = 1;
            ctx->chapters[ctx->chapter_count].page = ctx->current_page;
            strncpy(ctx->current_chapter, title, MAX_TITLE_LENGTH - 1);
            ctx->chapter_count++;

            fprintf(ctx->output, "\n%s\n", title);
            ctx->current_line += 2;
            for (int i = 0; i < strlen(title); i++) {
                fprintf(ctx->output, "=");
            }
            fprintf(ctx->output, "\n\n");
            ctx->current_line += 2;
            free(title);
        }
    }
    else if (strcmp(command, "SUBCHAP") == 0) {
        char *title = extract_string_param(line, "SUBCHAP");
        if (title && ctx->chapter_count < MAX_CHAPTERS) {
            check_page_break(ctx, 4);

            strncpy(ctx->chapters[ctx->chapter_count].title, title, MAX_TITLE_LENGTH - 1);
            ctx->chapters[ctx->chapter_count].level = 2;
            ctx->chapters[ctx->chapter_count].page = ctx->current_page;
            strncpy(ctx->current_subchap, title, MAX_TITLE_LENGTH - 1);
            ctx->chapter_count++;

            fprintf(ctx->output, "\n%s\n", title);
            ctx->current_line += 2;
            for (int i = 0; i < strlen(title); i++) {
                fprintf(ctx->output, "-");
            }
            fprintf(ctx->output, "\n\n");
            ctx->current_line += 2;
            free(title);
        }
    }
    else if (strcmp(command, "SUBSUBCHAP") == 0) {
        char *title = extract_string_param(line, "SUBSUBCHAP");
        if (title && ctx->chapter_count < MAX_CHAPTERS) {
            check_page_break(ctx, 3);

            strncpy(ctx->chapters[ctx->chapter_count].title, title, MAX_TITLE_LENGTH - 1);
            ctx->chapters[ctx->chapter_count].level = 3;
            ctx->chapters[ctx->chapter_count].page = ctx->current_page;
            strncpy(ctx->current_subsubchap, title, MAX_TITLE_LENGTH - 1);
            ctx->chapter_count++;

            fprintf(ctx->output, "\n%s\n\n", title);
            ctx->current_line += 3;
            free(title);
        }
    }
    else if (strcmp(command, "P") == 0) {
        check_page_break(ctx, 1);
        fprintf(ctx->output, "\n");
        ctx->current_line++;
        ctx->current_paragraph_align = ctx->params.justify;
        ctx->first_line_of_paragraph = 1;

        if (strstr(line, "LEFT")) {
            ctx->current_paragraph_align = ALIGN_LEFT;
        } else if (strstr(line, "RIGHT")) {
            ctx->current_paragraph_align = ALIGN_RIGHT;
        } else if (strstr(line, "CENTER")) {
            ctx->current_paragraph_align = ALIGN_CENTER;
        } else if (strstr(line, "FULL")) {
            ctx->current_paragraph_align = ALIGN_FULL;
        }
    }
    else if (strcmp(command, "BREAK") == 0) {
        check_page_break(ctx, 1);
        fprintf(ctx->output, "\n");
        ctx->current_line++;
    }
    else if (strcmp(command, "CODE") == 0) {
        ctx->in_code_block = 1;
        fprintf(ctx->output, "\n");
    }
    else if (strcmp(command, "ECODE") == 0) {
        ctx->in_code_block = 0;
        fprintf(ctx->output, "\n");
    }
    else if (strcmp(command, "LIST") == 0) {
        if (strstr(line, "BULLET")) {
            ctx->current_list.type = LIST_BULLET;
            ctx->current_list.bullet_char = '*';
        } else if (strstr(line, "RNUMBER")) {
            ctx->current_list.type = LIST_RNUMBER;
        } else if (strstr(line, "NUMBER")) {
            ctx->current_list.type = LIST_NUMBER;
        }
        ctx->current_list.item_count = 0;
        ctx->current_list.indent = ctx->params.indent;
        fprintf(ctx->output, "\n");
    }
    else if (strcmp(command, "BULLET") == 0) {
        const char *bullet_pos = strchr(line, '"');
        if (bullet_pos) {
            ctx->current_list.bullet_char = bullet_pos[1];
        }
    }
    else if (strcmp(command, "ITEM") == 0) {
        char *item = extract_string_param(line, "ITEM");
        if (item && ctx->current_list.item_count < MAX_LIST_ITEMS) {
            // Crear el prefijo del item (bullet/número)
            char prefix[32] = "";

            if (ctx->current_list.type == LIST_BULLET) {
                snprintf(prefix, sizeof(prefix), "%c ", ctx->current_list.bullet_char);
            } else if (ctx->current_list.type == LIST_NUMBER) {
                // Use consistent 3-character width: "1. " becomes "1.  " for single digits
                int num = ctx->current_list.item_count + 1;
                if (num < 10) {
                    snprintf(prefix, sizeof(prefix), "%d.  ", num);
                } else {
                    snprintf(prefix, sizeof(prefix), "%d. ", num);
                }
            } else if (ctx->current_list.type == LIST_RNUMBER) {
                const char *roman[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X",
                                      "XI", "XII", "XIII", "XIV", "XV", "XVI", "XVII", "XVIII", "XIX", "XX"};
                if (ctx->current_list.item_count < 20) {
                    // Use consistent 5-character width for Roman numerals
                    snprintf(prefix, sizeof(prefix), "%-4s ", roman[ctx->current_list.item_count]);
                }
            }

            // Usar la función especializada para items de lista
            output_list_item(ctx, prefix, item);

            ctx->current_list.item_count++;
            free(item);
        }
    }
    else if (strcmp(command, "ELIST") == 0) {
        ctx->current_list.type = LIST_NONE;
        ctx->current_list.item_count = 0;
        fprintf(ctx->output, "\n");
    }
    else if (strncmp(command, "TABLE", 5) == 0) {
        ctx->current_table.cols = extract_int_param(line, "COLS");
        ctx->current_table.row_count = 0;

        // Inicializar headers como cadenas vacías y TLINE tracking
        for (int i = 0; i < ctx->current_table.cols; i++) {
            ctx->current_table.headers[i][0] = '\0';
        }
        ctx->current_table.tline_count = 0;

        const char *widths_pos = strstr(line, "WIDTHS=");
        if (widths_pos) {
            widths_pos += 7;
            char *widths_str = malloc(strlen(widths_pos) + 1);
            strcpy(widths_str, widths_pos);
            char *end = strchr(widths_str, ' ');
            if (end) *end = '\0';

            char *token = strtok(widths_str, ",");
            int col = 0;
            while (token && col < ctx->current_table.cols) {
                ctx->current_table.widths[col] = atoi(token);
                col++;
                token = strtok(NULL, ",");
            }
            free(widths_str);
        }

        const char *aligns_pos = strstr(line, "ALIGNS=");
        if (aligns_pos) {
            aligns_pos += 7;
            for (int i = 0; i < ctx->current_table.cols && i < 20; i++) {
                if (aligns_pos[i*2] == 'L') ctx->current_table.aligns[i] = ALIGN_LEFT;
                else if (aligns_pos[i*2] == 'C') ctx->current_table.aligns[i] = ALIGN_CENTER;
                else if (aligns_pos[i*2] == 'R') ctx->current_table.aligns[i] = ALIGN_RIGHT;
            }
        }

        char *name = extract_string_param(line, "NAME");
        if (name) {
            strncpy(ctx->current_table.name, name, MAX_TITLE_LENGTH - 1);
            if (ctx->table_ref_count < MAX_TABLES) {
                strncpy(ctx->table_refs[ctx->table_ref_count].name, name, MAX_TITLE_LENGTH - 1);
                ctx->table_refs[ctx->table_ref_count].page = ctx->current_page;
                ctx->table_ref_count++;
            }
            free(name);
        }

        fprintf(ctx->output, "\n");
    }
    else if (strcmp(command, "TH") == 0) {
        const char *quote_start = strchr(line, '"');
        if (quote_start) {
            quote_start++;
            int col = 0;
            const char *current = quote_start;

            while (*current && col < ctx->current_table.cols) {
                const char *quote_end = strchr(current, '"');
                if (!quote_end) break;

                int len = quote_end - current;
                if (len < MAX_TITLE_LENGTH - 1) {
                    strncpy(ctx->current_table.headers[col], current, len);
                    ctx->current_table.headers[col][len] = '\0';
                }

                col++;
                current = quote_end + 1;

                while (*current && (*current == ' ' || *current == '|')) current++;
                if (*current == '"') current++;
            }
        }
    }
    else if (strcmp(command, "TLINE") == 0) {
        // Almacenar información de TLINE para renderizar en ETABLE
        if (ctx->current_table.tline_count < MAX_TABLE_ROWS) {
            ctx->current_table.tline_after_row[ctx->current_table.tline_count] = ctx->current_table.row_count - 1;
            ctx->current_table.tline_count++;
        }
    }
    else if (strcmp(command, "TR") == 0) {
        if (ctx->current_table.row_count < MAX_TABLE_ROWS) {
            const char *quote_start = strchr(line, '"');
            if (quote_start) {
                quote_start++;
                int col = 0;
                const char *current = quote_start;

                while (*current && col < ctx->current_table.cols) {
                    const char *quote_end = strchr(current, '"');
                    if (!quote_end) break;

                    int len = quote_end - current;
                    if (len < MAX_TITLE_LENGTH - 1) {
                        strncpy(ctx->current_table.data[ctx->current_table.row_count][col], current, len);
                        ctx->current_table.data[ctx->current_table.row_count][col][len] = '\0';
                    }

                    col++;
                    current = quote_end + 1;

                    while (*current && (*current == ' ' || *current == '|')) current++;
                    if (*current == '"') current++;
                }
            }
            ctx->current_table.row_count++;
        }
    }
    else if (strcmp(command, "ETABLE") == 0) {
        // Verificar si hay headers definidos
        int has_headers = 0;
        for (int col = 0; col < ctx->current_table.cols; col++) {
            if (strlen(ctx->current_table.headers[col]) > 0) {
                has_headers = 1;
                break;
            }
        }

        // Renderizar headers si existen
        if (has_headers) {
            for (int i = 0; i < ctx->params.left_margin; i++) {
                fprintf(ctx->output, " ");
            }

            for (int col = 0; col < ctx->current_table.cols; col++) {
                int width = ctx->current_table.widths[col];
                char *header = ctx->current_table.headers[col];

                if (ctx->current_table.aligns[col] == ALIGN_CENTER && width > strlen(header)) {
                    int padding = (width - strlen(header)) / 2;
                    for (int i = 0; i < padding; i++) fprintf(ctx->output, " ");
                    fprintf(ctx->output, "%s", header);
                    int remaining = width - strlen(header) - padding;
                    for (int i = 0; i < remaining && i >= 0; i++) fprintf(ctx->output, " ");
                } else if (ctx->current_table.aligns[col] == ALIGN_RIGHT && width > strlen(header)) {
                    int spaces = width - strlen(header);
                    for (int i = 0; i < spaces && i >= 0; i++) fprintf(ctx->output, " ");
                    fprintf(ctx->output, "%s", header);
                } else {
                    fprintf(ctx->output, "%s", header);
                    for (int i = strlen(header); i < width; i++) fprintf(ctx->output, " ");
                }

                // Espaciado entre columnas (sin marcos verticales)
                if (col < ctx->current_table.cols - 1) {
                    fprintf(ctx->output, "  ");
                }
            }
            fprintf(ctx->output, "\n");

            // Verificar si hay TLINE después de headers (row_index -1)
            for (int i = 0; i < ctx->current_table.tline_count; i++) {
                if (ctx->current_table.tline_after_row[i] == -1) {
                    // Renderizar TLINE
                    for (int j = 0; j < ctx->params.left_margin; j++) {
                        fprintf(ctx->output, " ");
                    }
                    int total_width = 0;
                    for (int j = 0; j < ctx->current_table.cols; j++) {
                        total_width += ctx->current_table.widths[j];
                    }
                    if (ctx->current_table.cols > 1) {
                        total_width += (ctx->current_table.cols - 1) * 2;
                    }
                    for (int j = 0; j < total_width; j++) {
                        fprintf(ctx->output, "-");
                    }
                    fprintf(ctx->output, "\n");
                    break;
                }
            }
        }

        for (int row = 0; row < ctx->current_table.row_count; row++) {
            for (int i = 0; i < ctx->params.left_margin; i++) {
                fprintf(ctx->output, " ");
            }

            for (int col = 0; col < ctx->current_table.cols; col++) {
                int width = ctx->current_table.widths[col];
                char *data = ctx->current_table.data[row][col];

                if (ctx->current_table.aligns[col] == ALIGN_CENTER && width > strlen(data)) {
                    int padding = (width - strlen(data)) / 2;
                    for (int i = 0; i < padding; i++) fprintf(ctx->output, " ");
                    fprintf(ctx->output, "%s", data);
                    int remaining = width - strlen(data) - padding;
                    for (int i = 0; i < remaining && i >= 0; i++) fprintf(ctx->output, " ");
                } else if (ctx->current_table.aligns[col] == ALIGN_RIGHT && width > strlen(data)) {
                    int spaces = width - strlen(data);
                    for (int i = 0; i < spaces && i >= 0; i++) fprintf(ctx->output, " ");
                    fprintf(ctx->output, "%s", data);
                } else {
                    fprintf(ctx->output, "%s", data);
                    for (int i = strlen(data); i < width; i++) fprintf(ctx->output, " ");
                }

                // Espaciado entre columnas (sin marcos verticales)
                if (col < ctx->current_table.cols - 1) {
                    fprintf(ctx->output, "  ");
                }
            }
            fprintf(ctx->output, "\n");

            // Verificar si hay TLINE después de esta fila
            for (int i = 0; i < ctx->current_table.tline_count; i++) {
                if (ctx->current_table.tline_after_row[i] == row) {
                    // Renderizar TLINE
                    for (int j = 0; j < ctx->params.left_margin; j++) {
                        fprintf(ctx->output, " ");
                    }
                    int total_width = 0;
                    for (int j = 0; j < ctx->current_table.cols; j++) {
                        total_width += ctx->current_table.widths[j];
                    }
                    if (ctx->current_table.cols > 1) {
                        total_width += (ctx->current_table.cols - 1) * 2;
                    }
                    for (int j = 0; j < total_width; j++) {
                        fprintf(ctx->output, "-");
                    }
                    fprintf(ctx->output, "\n");
                    break;
                }
            }
        }

        fprintf(ctx->output, "\n");
        ctx->current_table.row_count = 0;
        ctx->current_table.tline_count = 0;
    }
    else if (strcmp(command, "INCLUDE") == 0) {
        char *filename = extract_string_param(line, "INCLUDE");
        if (filename) {
            process_file(ctx, filename);
            free(filename);
        }
    }
}

void process_text(stroff_context_t *ctx, const char *text) {
    if (!ctx->in_document) return;

    if (ctx->in_code_block) {
        for (int i = 0; i < ctx->params.left_margin; i++) {
            fprintf(ctx->output, " ");
        }
        fprintf(ctx->output, "%s\n", text);
    } else {
        output_text(ctx, text, ctx->current_paragraph_align);
    }
}
