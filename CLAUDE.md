# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

STROFF is a document formatting language and processor inspired by classic RUNOFF and ROFF systems. It processes markup files (`.str`, legacy `.trf`) and generates formatted plain text documents (`.txt`) using a two-pass system for automatic table of contents and cross-references.

## Architecture

### Core Components

- **`src/main.c`**: Entry point that implements the two-pass processing system (first pass collects references, second pass generates output)
- **`src/parser.c`**: Command parser that processes STROFF markup directives (`.TITLE`, `.CHAP`, `.P`, etc.)
- **`src/formatter.c`**: Text formatting engine handling pagination, justification, tables, lists, and headers/footers
- **`src/utils.c`**: Utility functions for string manipulation and parameter parsing
- **`src/stroff.h`**: Main header with data structures and function declarations

### Key Data Structures

- **`stroff_context_t`**: Main processing context containing document parameters, chapter/table references, and current state
- **`document_params_t`**: Document configuration (margins, page size, justification, headers/footers)
- **`chapter_t`**: Chapter reference for table of contents generation
- **`table_t`**: Table structure with columns, alignments, and data
- **`list_t`**: List structure supporting bullets, numbers, and roman numerals

## Development Commands

### Building
```bash
gcc src/*.c -o bin/stroff
```

### Usage
```bash
./bin/stroff input.str output.txt
```

### Testing with Sample Document
```bash
./bin/stroff MANUAL.STR MANUAL.TXT
```

## Document Format

STROFF uses markup directives starting with `.` (dot commands):

- **Document setup**: `.TITLE`, `.AUTH`, `.DATE`, `.PAGEWIDTH`, `.LMARGIN`, etc.
- **Structure**: `.DOCUMENT`/`.EDOC`, `.CHAP`/`.ECHAP`, `.SUBCHAP`, `.SUBSUBCHAP`
- **File inclusion**: `.INCLUDE "path"` (relative to the file where it appears)
- **Content**: `.P` (paragraphs), `.LIST`/`.ELIST`, `.TABLE`/`.ETABLE`, `.CODE`/`.ECODE`
- **Layout**: `.PAGEBREAK`, `.MAKETOC` (table of contents), `.MAKETOT` (table of tables)

## Key Features

- **Two-pass processing**: First pass collects metadata, second pass generates formatted output
- **Automatic pagination**: Headers/footers with variable substitution (`{PAGE}`, `{TITLE}`, `{CHAPTITLE}`)
- **Text justification**: Full justification with automatic word wrapping
- **Dynamic indexing**: Automatic table of contents and table index generation
- **Complex tables**: Multi-column tables with configurable widths and alignments
- **Hierarchical structure**: Three levels of chapters with automatic numbering
- **Modular authoring**: Include reusable sections with `.INCLUDE` and nested relative paths

## File Extensions

- `.str` (legacy `.trf`): STROFF markup source files
- `.txt`: Generated formatted text output
- `.c/.h`: C source and header files
- `.o`: Compiled object files (in `bin/`)
