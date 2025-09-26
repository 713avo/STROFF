#include "stroff.h"

void output_text(stroff_context_t *ctx, const char *text, align_t align) {
    int content_width = ctx->params.page_width - ctx->params.left_margin - ctx->params.right_margin;
    char words[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int word_count = 0;

    // Dividir texto en palabras
    char *text_copy = malloc(strlen(text) + 1);
    strcpy(text_copy, text);
    char *word = strtok(text_copy, " \t");
    while (word && word_count < MAX_LINE_LENGTH) {
        strcpy(words[word_count], word);
        word_count++;
        word = strtok(NULL, " \t");
    }

    if (word_count == 0) {
        free(text_copy);
        return;
    }

    // Procesar palabras línea por línea
    int current_word = 0;
    while (current_word < word_count) {
        char line_words[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
        int line_word_count = 0;
        int line_length = 0;

        // Construir línea respetando ancho de contenido
        // Ajustar ancho disponible si es primera línea con indentación
        int available_width = content_width;
        int first_line_of_paragraph = (current_word == 0 && ctx->first_line_of_paragraph);
        if (first_line_of_paragraph) {
            available_width -= ctx->params.indent;
        }

        while (current_word < word_count) {
            int word_len = strlen(words[current_word]);
            int needed = word_len + (line_word_count > 0 ? 1 : 0); // +1 para espacio

            if (line_length + needed > available_width && line_word_count > 0) {
                break; // No cabe más en esta línea
            }

            strcpy(line_words[line_word_count], words[current_word]);
            line_length += word_len + (line_word_count > 0 ? 1 : 0);
            line_word_count++;
            current_word++;
        }

        // Imprimir margen izquierdo
        for (int i = 0; i < ctx->params.left_margin; i++) {
            fprintf(ctx->output, " ");
        }

        // Aplicar indentación solo en la primera línea del párrafo
        if (first_line_of_paragraph) {
            for (int i = 0; i < ctx->params.indent; i++) {
                fprintf(ctx->output, " ");
            }
            ctx->first_line_of_paragraph = 0; // Solo primera línea
        }

        // Imprimir línea con alineación
        if (align == ALIGN_CENTER) {
            int total_text_len = 0;
            for (int i = 0; i < line_word_count; i++) {
                total_text_len += strlen(line_words[i]);
                if (i < line_word_count - 1) total_text_len++;
            }
            int padding = (available_width - total_text_len) / 2;
            for (int i = 0; i < padding; i++) {
                fprintf(ctx->output, " ");
            }
            for (int i = 0; i < line_word_count; i++) {
                fprintf(ctx->output, "%s", line_words[i]);
                if (i < line_word_count - 1) fprintf(ctx->output, " ");
            }
        } else if (align == ALIGN_RIGHT) {
            int total_text_len = 0;
            for (int i = 0; i < line_word_count; i++) {
                total_text_len += strlen(line_words[i]);
                if (i < line_word_count - 1) total_text_len++;
            }
            int padding = available_width - total_text_len;
            for (int i = 0; i < padding; i++) {
                fprintf(ctx->output, " ");
            }
            for (int i = 0; i < line_word_count; i++) {
                fprintf(ctx->output, "%s", line_words[i]);
                if (i < line_word_count - 1) fprintf(ctx->output, " ");
            }
        } else if (align == ALIGN_FULL && line_word_count > 1 && current_word < word_count) {
            // Justificación completa solo si no es la última línea
            int total_word_len = 0;
            for (int i = 0; i < line_word_count; i++) {
                total_word_len += strlen(line_words[i]);
            }
            int total_spaces = available_width - total_word_len;
            int gaps = line_word_count - 1;

            if (gaps > 0) {
                int spaces_per_gap = total_spaces / gaps;
                int extra_spaces = total_spaces % gaps;

                for (int i = 0; i < line_word_count; i++) {
                    fprintf(ctx->output, "%s", line_words[i]);
                    if (i < line_word_count - 1) {
                        for (int j = 0; j < spaces_per_gap; j++) {
                            fprintf(ctx->output, " ");
                        }
                        if (i < extra_spaces) {
                            fprintf(ctx->output, " ");
                        }
                    }
                }
            } else {
                fprintf(ctx->output, "%s", line_words[0]);
            }
        } else {
            // Alineación izquierda o última línea de justificada
            for (int i = 0; i < line_word_count; i++) {
                fprintf(ctx->output, "%s", line_words[i]);
                if (i < line_word_count - 1) fprintf(ctx->output, " ");
            }
        }

        check_page_break(ctx, ctx->params.line_space);
        fprintf(ctx->output, "\n");
        ctx->current_line++;

        for (int i = 1; i < ctx->params.line_space; i++) {
            check_page_break(ctx, 1);
            fprintf(ctx->output, "\n");
            ctx->current_line++;
        }
    }

    free(text_copy);
}

void output_list_item(stroff_context_t *ctx, const char *prefix, const char *text) {
    int content_width = ctx->params.page_width - ctx->params.left_margin - ctx->params.right_margin;
    int list_base_margin = ctx->params.left_margin + ctx->current_list.indent;
    int prefix_len = strlen(prefix);

    char words[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int word_count = 0;

    // Dividir texto en palabras
    char *text_copy = malloc(strlen(text) + 1);
    strcpy(text_copy, text);
    char *word = strtok(text_copy, " \t");
    while (word && word_count < MAX_LINE_LENGTH) {
        strcpy(words[word_count], word);
        word_count++;
        word = strtok(NULL, " \t");
    }

    if (word_count == 0) {
        free(text_copy);
        return;
    }

    // Procesar palabras línea por línea
    int current_word = 0;
    int is_first_line = 1;

    while (current_word < word_count) {
        char line_words[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
        int line_word_count = 0;
        int line_length = 0;

        // Calcular ancho disponible (primera línea incluye prefijo)
        int available_width = content_width - ctx->current_list.indent;
        if (is_first_line) {
            available_width -= prefix_len;
        }

        // Construir línea respetando ancho disponible
        while (current_word < word_count) {
            int word_len = strlen(words[current_word]);
            int needed = word_len + (line_word_count > 0 ? 1 : 0);

            if (line_length + needed > available_width && line_word_count > 0) {
                break;
            }

            strcpy(line_words[line_word_count], words[current_word]);
            line_length += word_len + (line_word_count > 0 ? 1 : 0);
            line_word_count++;
            current_word++;
        }

        // Imprimir margen izquierdo
        for (int i = 0; i < list_base_margin; i++) {
            fprintf(ctx->output, " ");
        }

        // Imprimir prefijo solo en primera línea
        if (is_first_line) {
            fprintf(ctx->output, "%s", prefix);
            is_first_line = 0;
        } else {
            // En líneas siguientes, alinear con el texto (después del prefijo)
            for (int i = 0; i < prefix_len; i++) {
                fprintf(ctx->output, " ");
            }
        }

        // Imprimir palabras de la línea
        for (int i = 0; i < line_word_count; i++) {
            fprintf(ctx->output, "%s", line_words[i]);
            if (i < line_word_count - 1) fprintf(ctx->output, " ");
        }

        check_page_break(ctx, 1);
        fprintf(ctx->output, "\n");
        ctx->current_line++;
    }

    free(text_copy);
}

void output_header(stroff_context_t *ctx) {
    if (strlen(ctx->params.header) == 0) return;

    char header_text[MAX_TITLE_LENGTH];
    strcpy(header_text, ctx->params.header);
    substitute_variables(ctx, header_text);

    // Output header without page break checking to avoid recursion
    int content_width = ctx->params.page_width - ctx->params.left_margin - ctx->params.right_margin;
    int text_len = strlen(header_text);

    for (int i = 0; i < ctx->params.left_margin; i++) {
        fprintf(ctx->output, " ");
    }

    if (ctx->params.head_align == ALIGN_CENTER) {
        int padding = (content_width - text_len) / 2;
        for (int i = 0; i < padding; i++) {
            fprintf(ctx->output, " ");
        }
        fprintf(ctx->output, "%s", header_text);
    } else if (ctx->params.head_align == ALIGN_RIGHT) {
        int padding = content_width - text_len;
        for (int i = 0; i < padding; i++) {
            fprintf(ctx->output, " ");
        }
        fprintf(ctx->output, "%s", header_text);
    } else {
        fprintf(ctx->output, "%s", header_text);
    }

    fprintf(ctx->output, "\n");
}

void output_footer(stroff_context_t *ctx) {
    if (strlen(ctx->params.footer) == 0) return;

    char footer_text[MAX_TITLE_LENGTH];
    strcpy(footer_text, ctx->params.footer);
    substitute_variables(ctx, footer_text);

    // Output footer without page break checking to avoid recursion
    int content_width = ctx->params.page_width - ctx->params.left_margin - ctx->params.right_margin;
    int text_len = strlen(footer_text);

    fprintf(ctx->output, "\n");

    for (int i = 0; i < ctx->params.left_margin; i++) {
        fprintf(ctx->output, " ");
    }

    if (ctx->params.foot_align == ALIGN_CENTER) {
        int padding = (content_width - text_len) / 2;
        for (int i = 0; i < padding; i++) {
            fprintf(ctx->output, " ");
        }
        fprintf(ctx->output, "%s", footer_text);
    } else if (ctx->params.foot_align == ALIGN_RIGHT) {
        int padding = content_width - text_len;
        for (int i = 0; i < padding; i++) {
            fprintf(ctx->output, " ");
        }
        fprintf(ctx->output, "%s", footer_text);
    } else {
        fprintf(ctx->output, "%s", footer_text);
    }

    fprintf(ctx->output, "\n");
}

void output_toc(stroff_context_t *ctx) {
    check_page_break(ctx, 3 + ctx->chapter_count + 2);
    fprintf(ctx->output, "\nTABLA DE CONTENIDOS\n");
    ctx->current_line++;
    fprintf(ctx->output, "==================\n\n");
    ctx->current_line += 2;

    for (int i = 0; i < ctx->chapter_count; i++) {
        check_page_break(ctx, 1);

        for (int j = 0; j < ctx->params.left_margin; j++) {
            fprintf(ctx->output, " ");
        }

        for (int j = 1; j < ctx->chapters[i].level; j++) {
            fprintf(ctx->output, "  ");
        }

        fprintf(ctx->output, "%s", ctx->chapters[i].title);

        // Estrategia de posición fija: números siempre en la misma columna
        int content_width = ctx->params.page_width - ctx->params.left_margin - ctx->params.right_margin;
        int title_width = utf8_display_width(ctx->chapters[i].title) + (ctx->chapters[i].level - 1) * 2;

        // Posición fija para números: 4 caracteres desde el final (espacio para números hasta 999)
        int number_field_width = 4;  // "  99" o " 123"
        int dots_start_pos = title_width;
        int dots_end_pos = content_width - number_field_width;

        // Calcular puntos necesarios
        int dots_needed = dots_end_pos - dots_start_pos;
        if (dots_needed < 1) dots_needed = 1;

        // Imprimir puntos
        for (int j = 0; j < dots_needed; j++) {
            fprintf(ctx->output, ".");
        }

        // Imprimir número con padding a la derecha
        char page_str[16];
        snprintf(page_str, sizeof(page_str), "%d", ctx->chapters[i].page);
        int page_len = strlen(page_str);
        int padding = number_field_width - page_len;

        for (int j = 0; j < padding; j++) {
            fprintf(ctx->output, " ");
        }

        fprintf(ctx->output, "%s\n", page_str);
        ctx->current_line++;
    }

    check_page_break(ctx, 1);
    fprintf(ctx->output, "\n");
    ctx->current_line++;
}

void output_tot(stroff_context_t *ctx) {
    check_page_break(ctx, 3 + ctx->table_ref_count + 2);
    fprintf(ctx->output, "\nINDICE DE TABLAS\n");
    ctx->current_line++;
    fprintf(ctx->output, "================\n\n");
    ctx->current_line += 2;

    for (int i = 0; i < ctx->table_ref_count; i++) {
        check_page_break(ctx, 1);

        for (int j = 0; j < ctx->params.left_margin; j++) {
            fprintf(ctx->output, " ");
        }

        fprintf(ctx->output, "%s", ctx->table_refs[i].name);

        // Estrategia de posición fija: números siempre en la misma columna
        int content_width = ctx->params.page_width - ctx->params.left_margin - ctx->params.right_margin;
        int name_width = strlen(ctx->table_refs[i].name);

        // Posición fija para números: 4 caracteres desde el final
        int number_field_width = 4;
        int dots_start_pos = name_width;
        int dots_end_pos = content_width - number_field_width;

        // Calcular puntos necesarios
        int dots_needed = dots_end_pos - dots_start_pos;
        if (dots_needed < 1) dots_needed = 1;

        // Imprimir puntos
        for (int j = 0; j < dots_needed; j++) {
            fprintf(ctx->output, ".");
        }

        // Imprimir número con padding a la derecha
        char page_str[16];
        snprintf(page_str, sizeof(page_str), "%d", ctx->table_refs[i].page);
        int page_len = strlen(page_str);
        int padding = number_field_width - page_len;

        for (int j = 0; j < padding; j++) {
            fprintf(ctx->output, " ");
        }

        fprintf(ctx->output, "%s\n", page_str);
        ctx->current_line++;
    }

    check_page_break(ctx, 1);
    fprintf(ctx->output, "\n");
    ctx->current_line++;
}

void new_page(stroff_context_t *ctx) {
    // Agregar footer de la página actual si está configurado
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

    // Salto de página sin caracteres especiales
    fprintf(ctx->output, "\n");

    // Cambiar a nueva página
    ctx->current_page++;
    ctx->current_line = 0;

    // Agregar header de la nueva página (solo en capítulos)
    if (strlen(ctx->params.header) > 0 && ctx->in_chapters) {
        output_header(ctx);
        fprintf(ctx->output, "\n");
        ctx->current_line++;
    }
}

void check_page_break(stroff_context_t *ctx, int lines_needed) {
    if (ctx->params.page_height > 0) {
        // Reservar espacio para header y footer
        int header_lines = (strlen(ctx->params.header) > 0) ? 2 : 0;
        int footer_lines = (strlen(ctx->params.footer) > 0) ? 3 : 0;
        int available_lines = ctx->params.page_height - header_lines - footer_lines;

        if (ctx->current_line + lines_needed > available_lines) {
            new_page(ctx);
        }
    }
}

void output_line(stroff_context_t *ctx, const char *text) {
    check_page_break(ctx, 1);
    fprintf(ctx->output, "%s\n", text);
    ctx->current_line++;
}

void substitute_variables(stroff_context_t *ctx, char *text) {
    char result[MAX_TITLE_LENGTH];
    char *pos;

    // Usar un buffer temporal para evitar corrupción de memoria
    strcpy(result, text);

    pos = strstr(result, "{TITLE}");
    if (pos) {
        char before[MAX_TITLE_LENGTH];
        strncpy(before, result, pos - result);
        before[pos - result] = '\0';
        snprintf(text, MAX_TITLE_LENGTH, "%s%s%s", before, ctx->params.title, pos + 7);
        strcpy(result, text);
    }

    pos = strstr(result, "{CHAPTITLE}");
    if (pos) {
        char before[MAX_TITLE_LENGTH];
        strncpy(before, result, pos - result);
        before[pos - result] = '\0';
        snprintf(text, MAX_TITLE_LENGTH, "%s%s%s", before, ctx->current_chapter, pos + 11);
        strcpy(result, text);
    }

    pos = strstr(result, "{SUBCHAP}");
    if (pos) {
        char before[MAX_TITLE_LENGTH];
        strncpy(before, result, pos - result);
        before[pos - result] = '\0';
        snprintf(text, MAX_TITLE_LENGTH, "%s%s%s", before, ctx->current_subchap, pos + 9);
        strcpy(result, text);
    }

    pos = strstr(result, "{SUBSUBCHAP}");
    if (pos) {
        char before[MAX_TITLE_LENGTH];
        strncpy(before, result, pos - result);
        before[pos - result] = '\0';
        snprintf(text, MAX_TITLE_LENGTH, "%s%s%s", before, ctx->current_subsubchap, pos + 12);
        strcpy(result, text);
    }

    pos = strstr(result, "{PAGE}");
    if (pos) {
        char before[MAX_TITLE_LENGTH];
        strncpy(before, result, pos - result);
        before[pos - result] = '\0';
        snprintf(text, MAX_TITLE_LENGTH, "%s%d%s", before, ctx->current_page, pos + 6);
        strcpy(result, text);
    }

    pos = strstr(result, "{PAGES}");
    if (pos) {
        char before[MAX_TITLE_LENGTH];
        strncpy(before, result, pos - result);
        before[pos - result] = '\0';
        snprintf(text, MAX_TITLE_LENGTH, "%s%d%s", before, ctx->total_pages, pos + 7);
        strcpy(result, text);
    }
}