# Publicación Automática

Publicación Automática publica muchos archivos y carpetas sin preparar cada uno a mano. Elige uno de los tres botones de la parte superior de la página; solo se muestran los ajustes que necesitas.

## Publicar lo que elijo

Añade los archivos y carpetas que quieras (arrástralos a la lista, o usa **Seleccionar archivos** / **Seleccione una carpeta**) y pulsa **Publicar la lista**: cada archivo o carpeta se convierte en una publicación independiente. Con **Añadir en pausa** marcado (también para **Escanear una carpeta una vez**), las publicaciones esperan en la página **Publicaciones** hasta que las reanudes.

## Analizar una carpeta una vez

Elige una carpeta y pulsa **Analizar**: se lista lo que contiene. Las carpetas necesitan **compresión**. Usa **primero los archivos más recientes** para ordenar por fecha.

## Vigilar una carpeta y publicar lo que llega

Elige la carpeta y pulsa **Empezar a vigilar**: cada archivo o carpeta nuevo se publica cuando llega. Pulsa **Dejar de vigilar** para parar. Detener la vigilancia con publicaciones aún en espera pide confirmación.

Mientras vigila, una barra bajo las tres opciones muestra siempre la carpeta vigilada, cuántas publicaciones han terminado hoy (y cuántas con errores) y cuántas están en curso o en espera. **Pausar la vigilancia** hace que los archivos nuevos esperen: se publican cuando pulsas **Reanudar la vigilancia**; las publicaciones ya iniciadas continúan. Detener la vigilancia olvida los archivos que estaban en espera.

## La lista

Cada elemento tiene una casilla y su tamaño. Desmarca un elemento para dejarlo fuera; solo se publican los elementos marcados. Para quitar elementos de la lista, selecciónalos y pulsa SUPR. La línea bajo la lista muestra cuántos elementos están seleccionados, su tamaño, el tamaño que se va a subir y, tras una primera publicación, el tiempo de subida.

## Elementos ya publicados

Si algunos elementos marcados ya están en el historial (mismo nombre y tamaño), una ventana los lista con la fecha de la publicación anterior. Marca **Publicar de nuevo** para los que quieras publicar una vez más; los demás no se publican y se quedan en la lista, sin marcar y en gris. Si ya existe un archivo NZB con el mismo nombre, el nuevo se llama `nombre_1.nzb` y así sucesivamente. Al vigilar una carpeta, los elementos que ya están en el historial se omiten con una línea en el registro. Puedes desactivar el aviso en **Ajustes** (Historial).

## Antes de empezar

Cuando pulsas **Publicar la lista** o **Empezar a vigilar**, el programa comprueba los ajustes una sola vez para todo el grupo (un servidor y un grupo de noticias, compresión para las carpetas, los programas rar / 7z y par2, la carpeta del NZB, la carpeta temporal). Los problemas se muestran en palabras sencillas con un botón que los arregla; los avisos se pueden ignorar con **Publicar igualmente**. Es la misma ventana que en [Publicación Rápida](04-quick-post.md).

## Consejos

- Usa compresión, nombres aleatorios, contraseñas aleatorias y par2: es la configuración habitual para la publicación automatizada. El perfil **Recomendado** (menú **Perfil**) la ajusta con un clic.
- **Después de publicar** (la fila sobre el botón Publicar): no hacer nada (predeterminado), mover los archivos a una **carpeta «Posted»**, o borrarlos. El nombre predeterminado `Posted` crea esa carpeta junto a cada archivo o carpeta; también puedes elegir otra carpeta. No se mueve ni se borra nada si la publicación no terminó sin errores. Un nombre ya usado en la carpeta nunca se sobrescribe: el archivo pasa a ser `nombre (2)`. El análisis y la vigilancia de carpetas ignoran la carpeta «Posted». Borrar es definitivo: úsalo solo cuando estés seguro.
- En la línea de comandos, la misma opción es `--move_posted` o `--rm_posted` (consulta [Línea de comandos](10-command-line.md)).
- El programa recuerda el último botón que usaste.
- Para un NAS o un servidor sin pantalla, la línea de comandos y la imagen de Docker hacen el mismo trabajo.

Consulta [Línea de comandos](10-command-line.md).
