# STROFF - Document Formatting Language

STROFF is a powerful document formatting system inspired by classic RUNOFF and ROFF systems. It processes plain text markup files and generates professionally formatted documents with automatic pagination, table of contents, and advanced typography features.

## Features

- **📄 Professional Document Formatting**: Complete control over page layout, margins, and typography
- **📚 Automatic Indexing**: Generate table of contents and table indexes automatically
- **🔤 Advanced Typography**: Full text justification with intelligent word wrapping
- **📑 Smart Pagination**: Automatic page breaks with customizable headers and footers
- **📋 Rich Content Elements**: Support for tables, lists, code blocks, and hierarchical chapters
- **🔧 Variable Substitution**: Dynamic headers and footers with document variables
- **📖 Two-Pass Processing**: First pass collects references, second pass generates final output

## Quick Start

### Building

```bash
gcc src/*.c -o bin/stroff
```

### Usage

```bash
./bin/stroff input.trf output.txt
```

### Example Document

Create a file `example.trf`:

```
.TITLE "My Document"
.AUTH "Your Name"
.DATE "2025-09-25"
.PAGEWIDTH 80
.PAGEHEIGHT 24
.JUSTIFY FULL

.DOCUMENT
.MAKETOC

.CHAP "Introduction"
.P
This is a paragraph that will be automatically wrapped and justified
according to the page width settings. STROFF handles all the formatting
details for you.

.CHAP "Features"
.LIST TYPE=BULLET
.ITEM "Automatic text wrapping"
.ITEM "Professional pagination"
.ITEM "Table of contents generation"
.ELIST

.EDOC
```

Then process it:

```bash
./bin/stroff example.trf example.txt
```

## Document Structure

### Configuration Section
Set document parameters before `.DOCUMENT`:

```
.TITLE "Document Title"
.AUTH "Author Name"
.DATE "Creation Date"
.PAGEWIDTH 80          # Page width in characters
.PAGEHEIGHT 24         # Page height in lines
.LMARGIN 4             # Left margin
.RMARGIN 4             # Right margin
.JUSTIFY FULL          # Text justification
.HEADER "Header text"   # Page header
.FOOTER "Footer text"   # Page footer
```

### Content Structure
```
.DOCUMENT              # Start document
.MAKETOC              # Generate table of contents

.CHAP "Chapter Title"  # Main chapter
.SUBCHAP "Section"     # Subsection
.SUBSUBCHAP "Sub"      # Sub-subsection

.P                     # Paragraph
Your paragraph text here.

.EDOC                  # End document
```

## Content Elements

### Paragraphs
```
.P                     # New paragraph (global justification)
.P LEFT               # Left-aligned paragraph
.P CENTER             # Centered paragraph
.P FULL               # Fully justified paragraph
```

### Lists
```
# Bulleted list
.LIST TYPE=BULLET CHAR=*
.ITEM "First item"
.ITEM "Second item"
.ELIST

# Numbered list
.LIST TYPE=NUMBER
.ITEM "First numbered item"
.ITEM "Second numbered item"
.ELIST

# Roman numeral list
.LIST TYPE=RNUMBER
.ITEM "First roman item"
.ITEM "Second roman item"
.ELIST
```

### Tables
```
.TABLE COLS=3 WIDTHS=20,15,25 ALIGNS=L,C,R NAME="Sample Table"
.TH "Column 1" "Column 2" "Column 3"
.TR "Data 1" "Data 2" "Data 3"
.TR "More data" "Center" "Right"
.ETABLE
```

### Code Blocks
```
.CODE
function example() {
    return "preserved formatting";
}
.ECODE
```

## Variables

Use these variables in headers and footers:

- `{TITLE}` - Document title
- `{CHAPTITLE}` - Current chapter title
- `{SUBCHAP}` - Current subsection title
- `{SUBSUBCHAP}` - Current sub-subsection title
- `{PAGE}` - Current page number
- `{PAGES}` - Total pages (available in second pass)

Example:
```
.HEADER "{TITLE} — {CHAPTITLE}"
.FOOTER "Page {PAGE} of {PAGES}"
```

## Architecture

### Project Structure
```
├── src/
│   ├── main.c         # Entry point and two-pass processing
│   ├── parser.c       # Command parsing and processing
│   ├── formatter.c    # Text formatting and output
│   ├── utils.c        # Utility functions
│   └── stroff.h       # Header definitions
├── bin/               # Compiled binaries and object files
├── MANUAL.TRF         # Complete manual in STROFF format
└── STROFF.md          # Language specification
```

### Two-Pass Processing

1. **First Pass**: Collects chapter references, table names, and calculates total pages
2. **Second Pass**: Generates final formatted output with complete cross-references

## File Extensions

- `.trf` - STROFF markup source files
- `.txt` - Generated formatted text output

## Limits

- Maximum 100 chapters
- Maximum 50 tables
- Maximum 1024 characters per line
- Maximum 100 list items per list
- Maximum 20 columns per table

## Examples

The repository includes comprehensive examples:

- `MANUAL.TRF` - Complete user manual demonstrating all features
- `STROFF.md` - Language specification and reference

## Contributing

This project implements a complete document formatting language. When contributing:

1. Follow the existing C code style
2. Test with the included `MANUAL.TRF` example
3. Update documentation for new features
4. Ensure backward compatibility

## License

This project is provided as-is for educational and personal use.