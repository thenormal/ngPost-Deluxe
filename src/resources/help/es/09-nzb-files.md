# Archivos NZB

Un archivo NZB lista los artículos de una publicación. Cualquiera que lo tenga puede descargar la publicación de Usenet.

## Dónde se guarda

La carpeta se configura en **Ajustes**, categoría **Archivos NZB**. El archivo toma el nombre de la publicación (el primer archivo o carpeta).

## Cómo usarlo

- Compártelo junto con la contraseña del archivo comprimido, si has puesto una.
- Un programa de descarga (un lector de noticias) lo abre y descarga los archivos.
- Con la ofuscación, el NZB es la **única** forma de encontrar tu publicación: guarda una copia.

## Comprobar un NZB

En la línea de comandos, `--check` verifica que los artículos de un NZB siguen disponibles en el servidor.

## Historial

La página **Historial** lista tus subidas pasadas (fecha, nombre, tamaño, velocidad, archivo comprimido, contraseña, grupos). Puedes desactivar el historial en Ajustes. La contraseña se guarda en texto claro.

### Publicaciones que ya están en el historial

Antes de que empiece una publicación, ngPost busca en el historial una publicación correcta con el mismo nombre de NZB y el mismo tamaño total de los archivos. En **Publicación Rápida** una ventana pregunta si publicarla de nuevo (**Publicar igualmente** o **Cancelar**). En **Publicación Automática** y en la línea de comandos la publicación se omite y una línea del registro lo indica. Para publicarla de todos modos, usa la opción `--allow_repost` o desactiva el aviso en **Ajustes**, categoría **Historial y cola** (tarjeta Historial de las subidas). La comprobación necesita que el historial esté activado y solo mira el nombre y el tamaño: una carpeta renombrada no se reconoce.

## Archivo NFO

Un NFO es un pequeño archivo de texto que describe la publicación: su nombre, la fecha, los archivos con sus tamaños, cómo se preparó (archivo comprimido y par2) y un texto tuyo. Actívalo en la tarjeta **Archivo NFO** de Publicación Rápida o de Publicación Automática (es la misma tarjeta: los ajustes son compartidos).

- **Escribir un archivo NFO junto al nzb**: `nombre.nfo` se guarda en la misma carpeta que `nombre.nzb`.
- **Publicar también el archivo NFO con los archivos**: se sube con la publicación y aparece en el NZB.
- **Incluir información multimedia (MediaInfo)**: activado de forma predeterminada. Para los archivos de vídeo y audio, el NFO lista la duración, el vídeo (formato, tamaño, fotogramas por segundo, HDR), las pistas de audio (idioma, formato, canales, tasa de bits) y los subtítulos. Se describen como máximo 20 archivos (los demás se cuentan) y la lectura se detiene tras 20 segundos. Usa el programa gratuito MediaInfo: viene con el instalador de Windows y la imagen de Docker; en Linux y macOS instala el paquete `mediainfo`, o elige el programa en Ajustes. Sin MediaInfo, el NFO simplemente no tiene sección multimedia.
- **Escribir la contraseña del archivo comprimido**: desactivado de forma predeterminada. En un NFO publicado, la contraseña la puede leer cualquiera que descargue la publicación.
- **Texto añadido al final**: notas, enlaces, lo que quieras decir. El mismo texto se usa para todas las publicaciones.

Cuando los nombres de los archivos están ocultos (ofuscación de los artículos o de los nombres de archivo), el NFO que se publica no lista ningún nombre de archivo ni información multimedia, y el propio archivo recibe un nombre aleatorio. En la línea de comandos usa `--gen_nfo`, `--nfo_in_post`, `--nfo_password` y `--nfo_text file.txt`, además de `--no_nfo_media` y `--mediainfo_path ruta`.
