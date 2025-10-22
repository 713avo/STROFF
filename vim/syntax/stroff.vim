" Vim syntax file
" Language:     STROFF
" Maintainer:   STROFF Project
" Last Change:  2025-10-22
" Filenames:    *.str *.trf

if exists("b:current_syntax")
  finish
endif

" Commands - Document Setup
syn keyword stroffDocSetup .TITLE .AUTH .DATE .PAGEWIDTH .PAGEHEIGHT .LMARGIN .RMARGIN .TMARGIN .BMARGIN
syn keyword stroffDocSetup .HEADER .FOOTER .JUSTIFY .NOJUSTIFY

" Commands - Document Structure
syn keyword stroffStructure .DOCUMENT .EDOC .CHAP .ECHAP .SUBCHAP .SUBSUBCHAP
syn keyword stroffStructure .P .PAGEBREAK .MAKETOC .MAKETOT

" Commands - Lists and Tables
syn keyword stroffList .LIST .ELIST .ITEM
syn keyword stroffTable .TABLE .ETABLE .ROW .CELL

" Commands - Code Blocks
syn keyword stroffCode .CODE .ECODE

" Commands - File Inclusion
syn keyword stroffInclude .INCLUDE

" List Types
syn keyword stroffListType BULLET NUMBER ROMAN

" Table Alignment
syn keyword stroffTableAlign LEFT CENTER RIGHT

" String literals (for file paths, titles, etc.)
syn region stroffString start=+"+ skip=+\\"+ end=+"+

" Comments (lines starting with .# or # after commands)
syn match stroffComment "^\.#.*$"
syn match stroffComment "#.*$" contains=stroffTodo

" TODO/FIXME/NOTE in comments
syn keyword stroffTodo contained TODO FIXME NOTE XXX

" Variables in headers/footers
syn match stroffVariable "{\w\+}"

" Numbers (for margins, widths, etc.)
syn match stroffNumber "\d\+"

" Chapter/Section References
syn match stroffReference "\d\+\.\d\+\(\.\d\+\)\?"

" Highlighting
hi def link stroffDocSetup     Statement
hi def link stroffStructure    Keyword
hi def link stroffList         Keyword
hi def link stroffTable        Keyword
hi def link stroffCode         PreProc
hi def link stroffInclude      Include
hi def link stroffListType     Type
hi def link stroffTableAlign   Type
hi def link stroffString       String
hi def link stroffComment      Comment
hi def link stroffTodo         Todo
hi def link stroffVariable     Identifier
hi def link stroffNumber       Number
hi def link stroffReference    Special

let b:current_syntax = "stroff"
