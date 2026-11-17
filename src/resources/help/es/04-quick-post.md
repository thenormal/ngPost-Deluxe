# Publicación Rápida

Publicación Rápida envía los archivos y carpetas que eliges. Cada pestaña es una publicación, así que puedes preparar varias y ejecutarlas en cola.

## Pasos

1. Añade archivos y carpetas con **Seleccionar archivos** o arrastrándolos.
2. Elige las opciones (abajo).
3. Pulsa **Publicar Ficheros**.

## Opciones

- **Comprimir**: mete los archivos en un archivo comprimido RAR o 7z antes de publicar. Puedes dividirlo en volúmenes.
- **Contraseña**: el archivo comprimido puede tener una contraseña, fija o generada al azar. La contraseña se guarda con la publicación (por ejemplo, en los metadatos del NZB y en el Historial).
- **Nombre aleatorio**: da al archivo comprimido un nombre aleatorio.
- **par2**: crea archivos de recuperación. Ajusta la redundancia en porcentaje: más redundancia permite al descargador reparar más artículos que faltan, pero sube más datos.
- **Ofuscación**: oculta los asuntos de los artículos. Sin el archivo NZB nadie puede encontrar tu publicación, ni siquiera tú.

## Comprobaciones y resumen

Debajo de las tarjetas, una línea te dice qué se va a publicar: el número de elementos, su tamaño y, con compresión o par2, aproximadamente cuánto se sube. El tiempo de subida aparece después de la primera publicación de la sesión, a partir de la velocidad real de esa subida (antes, ngPost no adivina).

Cuando pulsas **Publicar Ficheros**, ngPost comprueba primero la publicación. Si algo va mal, una ventana lo explica en palabras sencillas, con un botón que lo arregla:

- **Errores** (la publicación no puede empezar): ningún servidor activado, ningún grupo de noticias válido, carpetas sin compresión, programa rar / 7z o par2 no encontrado, una contraseña activada pero vacía, una carpeta NZB sin permiso de escritura.
- **Avisos** (puedes elegir **Publicar igualmente**): falta la carpeta temporal o puede ser demasiado pequeña, no se encuentra MediaInfo para la información multimedia del NFO.

Tras un arreglo se vuelven a hacer las comprobaciones, así que un problema nuevo aparece enseguida en lugar de a mitad de la publicación. Publicación Automática todavía no tiene esta ventana.

## Perfiles

El menú **Perfil** sobre las tarjetas ajusta todas las opciones de una vez.

- **Recomendado**: compresión, nombre y contraseña aleatorios, par2 con un 10 % de redundancia. Es la opción de una instalación nueva.
- **Personalizado**: se muestra cuando cambias una opción a mano.
- **Tus propios perfiles**: ajusta las opciones, pulsa **Guardar como perfil...** y ponle un nombre. **Eliminar** borra el perfil elegido. Los perfiles se guardan en el archivo de configuración (clave `profile`) y son los mismos en Publicación Rápida y en Publicación Automática.

La tarjeta **Archivo NFO** (consulta [Archivos NZB](09-nzb-files.md)) también es compartida: Publicación Rápida y Publicación Automática muestran los mismos ajustes.

## Pestañas

La pestaña **+** abre una nueva publicación. Una pestaña muestra su estado con un icono: pendiente, subiendo, terminada o con error.

Consulta también [Progreso y registro](06-progress-and-log.md).
