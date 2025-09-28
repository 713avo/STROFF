#include "stroff.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo.str> <archivo.txt>\n", argv[0]);
        return 1;
    }

    stroff_context_t ctx;
    init_context(&ctx);

    ctx.output = fopen("/dev/null", "w");
    if (!ctx.output) {
        fprintf(stderr, "Error: No se puede abrir /dev/null\n");
        return 1;
    }

    process_file(&ctx, argv[1]);
    fclose(ctx.output);

    // Guardar el total de páginas de la primera pasada
    ctx.total_pages = ctx.current_page;

    ctx.output = fopen(argv[2], "w");
    if (!ctx.output) {
        fprintf(stderr, "Error: No se puede abrir el archivo de salida '%s'\n", argv[2]);
        return 1;
    }

    // Reinicializar todo el contexto para la segunda pasada
    ctx.in_document = 0;
    ctx.in_code_block = 0;
    ctx.in_chapters = 0;
    ctx.generate_toc = 0;
    ctx.generate_tot = 0;
    ctx.current_list.type = LIST_NONE;
    ctx.current_list.item_count = 0;
    ctx.current_table.row_count = 0;
    ctx.current_paragraph_align = ALIGN_LEFT;
    ctx.first_line_of_paragraph = 0;
    ctx.current_page = 1;
    ctx.current_line = 0;
    strcpy(ctx.current_chapter, "");
    strcpy(ctx.current_subchap, "");
    strcpy(ctx.current_subsubchap, "");
    ctx.include_depth = 0;
    for (int i = 0; i < MAX_INCLUDE_DEPTH; i++) {
        ctx.include_stack[i][0] = '\0';
    }
    // NOTA: NO reinicializar ctx.total_pages - mantiene el valor de la primera pasada

    process_file(&ctx, argv[1]);

    fclose(ctx.output);
    return 0;
}
