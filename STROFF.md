# Especificación del Lenguaje STROFF

## Versión 1.0 - Especificación Actualizada

STROFF es un lenguaje de marcas para la creación de documentos de texto formateado, inspirado en los clásicos sistemas RUNOFF y ROFF. Esta especificación documenta la implementación actual y todas las características soportadas.

## Características Principales

- **Formato de página configurable**: Control preciso sobre márgenes, ancho y alto
- **Justificación completa**: Text wrapping automático con distribución uniforme de espacios
- **Paginación automática**: Saltos de página automáticos con headers y footers opcionales
- **Estructura jerárquica**: Capítulos, subcapítulos y sub-subcapítulos
- **Índices automáticos**: Tabla de contenidos e índice de tablas
- **Elementos avanzados**: Listas, tablas, bloques de código
- **Variables dinámicas**: Sustitución automática en headers y footers

## Estructura de un Documento

### 1. Parámetros Globales

Los parámetros deben definirse antes del comando `.DOCUMENT`:

#### Información del Documento
```
.TITLE "Título del Documento"
.AUTH "Nombre del Autor"
.DATE "Fecha de Creación"
```

#### Configuración de Página
```
.PAGEWIDTH 60        # Ancho total en caracteres (default: 80)
.PAGEHEIGHT 24       # Alto en líneas (default: 40, 0=sin paginación)
.LMARGIN 10          # Margen izquierdo en espacios (default: 0)
.RMARGIN 10          # Margen derecho en espacios (default: 0)
```

#### Formato de Texto
```
.INDENT 4            # Indentación de párrafos (default: 0)
.TABSIZE 4           # Tamaño de tabulación (default: 4)
.JUSTIFY FULL        # Justificación: LEFT, RIGHT, CENTER, FULL (default: LEFT)
.LINESPACE 1         # Interlineado: 1=simple, 2=doble (default: 1)
```

#### Headers y Footers
```
.HEADER "Texto de cabecera con variables"
.HEADALIGN CENTER    # Alineación: LEFT, RIGHT, CENTER (default: LEFT)
.FOOTER "Texto de pie con variables"
.FOOTALIGN RIGHT     # Alineación: LEFT, RIGHT, CENTER (default: LEFT)
```

**Variables disponibles en headers/footers:**
- `{TITLE}`: Título del documento
- `{CHAPTITLE}`: Título del capítulo actual
- `{SUBCHAP}`: Título del subcapítulo actual
- `{SUBSUBCHAP}`: Título del sub-subcapítulo actual
- `{PAGE}`: Número de página actual
- `{PAGES}`: Total de páginas (calculado en segunda pasada)

### 2. Estructura del Documento

#### Inicio y Fin
```
.DOCUMENT            # Inicia el documento (muestra título, autor, fecha)
  ... contenido ...
.EDOC                # Finaliza el documento
```

#### Capítulos y Secciones
```
.CHAP "Título del Capítulo"        # Capítulo (nivel 1)
.SUBCHAP "Título del Subcapítulo"  # Subcapítulo (nivel 2)
.SUBSUBCHAP "Título"               # Sub-subcapítulo (nivel 3)

.ECHAP                             # Cierra capítulo actual
.ESCHAP                            # Cierra subcapítulo actual
.ESSCHAP                           # Cierra sub-subcapítulo actual
```

#### Índices Automáticos
```
.MAKETOC             # Genera tabla de contenidos
.MAKETOT             # Genera índice de tablas
```

*Nota: Los índices se generan automáticamente usando un sistema de dos pasadas.*

### 3. Control de Paginación

```
.PAGEBREAK           # Fuerza un salto de página
```

El sistema maneja automáticamente:
- **Saltos de página**: Cuando se alcanza `PAGEHEIGHT`
- **Headers**: Solo aparecen en páginas de capítulos (no en título/índices)
- **Footers**: Aparecen en todas las páginas si están configurados
- **Relleno automático**: Páginas se llenan con líneas vacías hasta `PAGEHEIGHT`

## Contenido del Documento

### Párrafos y Texto

#### Creación de Párrafos
```
.P                   # Nuevo párrafo con justificación global
.P LEFT             # Párrafo con alineación específica
.P RIGHT            # Párrafo alineado a la derecha
.P CENTER           # Párrafo centrado
.P FULL             # Párrafo justificado completo
```

**Comportamiento de indentación:**
- Solo la **primera línea** de cada párrafo se indenta según `.INDENT`
- Las líneas siguientes mantienen solo el margen izquierdo

#### Text Wrapping y Justificación
- **Automático**: El texto se divide automáticamente en líneas
- **Ancho efectivo**: `PAGEWIDTH - LMARGIN - RMARGIN - INDENT` (primera línea)
- **JUSTIFY FULL**: Distribuye espacios uniformemente entre palabras
- **Última línea**: En párrafos justificados, la última línea queda alineada a la izquierda

#### Control de Líneas
```
.BREAK               # Salto de línea dentro del párrafo
```

### Listas

#### Sintaxis Básica
```
.LIST TYPE=BULLET INDENT=4 CHAR=*
.ITEM "Primer elemento"
.ITEM "Segundo elemento"
.ELIST
```

#### Tipos de Listas

**Lista con viñetas:**
```
.LIST TYPE=BULLET CHAR=*
.ITEM "Elemento con asterisco"
.ITEM "Otro elemento"
.ELIST
```

**Lista numerada:**
```
.LIST TYPE=NUMBER
.ITEM "Primer elemento numerado"
.ITEM "Segundo elemento numerado"
.ELIST
```

**Lista con números romanos:**
```
.LIST TYPE=RNUMBER
.ITEM "Primer elemento romano"
.ITEM "Segundo elemento romano"
.ELIST
```

### Tablas

#### Sintaxis Básica
```
.TABLE COLS=3 WIDTHS=20,15,25 ALIGNS=L,C,R NAME="Mi Tabla"
.TH "Columna 1" "Columna 2" "Columna 3"
.TLINE
.TR "Datos 1" "Datos 2" "Datos 3"
.TR "Más datos" "Centro" "Derecha"
.TLINE
.TR "Total" "Resumen" "Final"
.ETABLE
```

#### Parámetros de Tabla
- `COLS=n`: Número de columnas (requerido)
- `WIDTHS=n1,n2,n3`: Anchos de columnas en caracteres
- `ALIGNS=L,C,R`: Alineaciones (L=Left, C=Center, R=Right)
- `NAME="texto"`: Nombre para índice de tablas

#### Elementos de Tabla
- `.TH`: Fila de headers (encabezados)
- `.TR`: Fila de datos regulares
- `.TLINE`: Línea separadora horizontal opcional
- `.ETABLE`: Cierra la tabla

**Formato de salida:**
- Formato limpio sin marcos externos
- Espaciado uniforme entre columnas (2 espacios)
- Alineación automática según `ALIGNS`
- Líneas separadoras opcionales con `.TLINE`
- Ajuste de contenido según `WIDTHS`

### Bloques de Código

```
.CODE
  función ejemplo() {
      return "código preservado";
  }
.ECODE
```

**Características:**
- Preserva espaciado y formato original
- No aplica justificación ni text wrapping
- Respeta márgenes configurados

## Comentarios

```
# Esto es un comentario - línea ignorada completamente
```

## Procesamiento del Documento

### Sistema de Dos Pasadas

El procesador STROFF utiliza un sistema de dos pasadas:

1. **Primera pasada**: Recolecta información de capítulos y referencias
2. **Segunda pasada**: Genera la salida final con índices correctos

### Formato de Salida

- **Text wrapping**: Automático respetando `PAGEWIDTH`
- **Paginación**: Headers/footers solo donde corresponde
- **Índices**: Dot leaders con números alineados en columna fija
- **Sin caracteres especiales**: Salida en texto plano sin form feeds

## Ejemplo Completo

```
# Configuración del documento
.TITLE "Mi Documento"
.AUTH "Autor"
.DATE "2025-09-25"
.PAGEWIDTH 60
.PAGEHEIGHT 24
.LMARGIN 10
.RMARGIN 10
.INDENT 4
.JUSTIFY FULL
.HEADER "Mi Documento — {CHAPTITLE}"
.FOOTER "Página {PAGE}"

# Inicio del documento
.DOCUMENT
.MAKETOC

# Contenido
.CHAP "Introducción"
.P
Este es un párrafo justificado que se dividirá automáticamente
en múltiples líneas respetando el ancho de página configurado.

.CHAP "Datos"
.TABLE COLS=2 WIDTHS=20,20 ALIGNS=L,R NAME="Tabla de Ejemplo"
.TH "Concepto" "Valor"
.TLINE
.TR "Elemento A" "123"
.TR "Elemento B" "456"
.ETABLE

.EDOC
```

## Notas de Implementación

- **Compilación**: `gcc src/*.c -o stroff`
- **Uso**: `./stroff archivo.trf archivo.txt`
- **Extensiones**: `.trf` para archivos STROFF, `.txt` para salida
- **Codificación**: UTF-8 soportado para texto unicode
- **Límites**: Máximo 100 capítulos, 50 tablas, líneas de 1024 caracteres

## Diferencias con ROFF Original

- **Simplificado**: Sintaxis más simple y legible
- **Text wrapping**: Automático con justificación completa
- **Paginación moderna**: Sin form feeds, headers contextuales
- **Índices automáticos**: Generación automática con dot leaders
- **Variables dinámicas**: Sustitución automática en headers/footers