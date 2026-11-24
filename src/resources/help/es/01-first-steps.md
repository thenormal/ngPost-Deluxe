# Primeros pasos

Esta página te lleva desde una instalación nueva hasta tu primera publicación.

## Qué necesitas

- Una cuenta con un **proveedor de Usenet** (nombre del host, puerto, nombre de usuario y contraseña).
- Los archivos o carpetas que quieres publicar.
- Opcionalmente, las herramientas para comprimir (**7-Zip** o **RAR**) y para crear archivos par2 (**ParPar** o **par2**). El instalador de Windows ya incluye 7-Zip y ParPar.

## Tu primera publicación

Mientras no haya un servidor y un grupo de noticias configurados, una barra azul en la parte superior de Publicación Rápida y de Publicación Automática te dice qué falta, con un botón que abre la página adecuada. Desaparece sola.

1. Abre **Servidores**, añade tu proveedor y pulsa **Probar conexión**. Consulta [Servidores y conexiones](02-servers.md).
2. Abre **Ajustes**, categoría **Publicaciones**, tarjeta **Grupos de noticias**, y añade al menos un grupo de noticias. Consulta [Grupos de noticias](03-newsgroups.md).
3. Abre **Publicación Rápida**, pulsa **Seleccionar archivos** (o arrastra archivos y carpetas a la ventana).
4. Elige las opciones que quieras (compresión, par2, contraseña) y pulsa **Publicar Ficheros**.
5. Sigue el progreso en la parte inferior de la ventana y en el registro. Al final el **archivo NZB** se guarda y se muestra en el registro.

El archivo NZB es el «mapa» de tu publicación: sin él nadie puede encontrar los artículos, sobre todo si usas la ofuscación. Guárdalo bien.

## Dónde está cada cosa

- **Publicación guiada**: responde unas preguntas y la publicación se prepara por ti. Consulta [Publicación guiada](01b-guided-post.md).
- **Publicación Rápida**: publica archivos y carpetas ahora.
- **Publicación Automática**: analiza o vigila carpetas.
- **Servidores**: tus proveedores de Usenet.
- **Ajustes**: opciones predeterminadas del programa, en cinco categorías (General, Publicaciones, Archivos NZB, Notificaciones, Historial y cola).
- **Historial**: todo lo que has publicado, con sus [estadísticas](09b-statistics.md).
- **Ayuda**: esta sección.

## Nuevas versiones

Una vez al día, al iniciar el programa, ngPost Deluxe pregunta a GitHub si existe una versión más reciente (GitHub solo ve tu dirección IP). Si la hay, aparece un botón **Version X available** en la parte inferior de la barra izquierda: haz clic para abrir la página de la versión. No se descarga ni se instala nada por sí solo, y las versiones candidatas nunca se anuncian.

Para desactivarlo, desmarca **Buscar una nueva versión al iniciar el programa** en **Ajustes**, categoría **General**, tarjeta **Interfaz**. La versión de línea de comandos y la imagen de Docker nunca lo comprueban.

## Cambiar de ordenador o compartir una configuración

En **Ajustes**, categoría **Historial y cola**, tarjeta **Exportar e importar los ajustes**, **Exportar los ajustes...** escribe un archivo de texto con los ajustes del programa, los perfiles, los grupos de noticias favoritos, las notificaciones elegidas y, si quieres, los servidores. **Importar los ajustes...** lee ese archivo en otro ordenador y muestra lo que contiene antes de cambiar nada.

El archivo nunca contiene una contraseña, un token, la dirección de una notificación, la contraseña fija de los archivos ni una carpeta del ordenador. Los servidores importados quedan desactivados hasta que escribas su contraseña en **Servidores**. Los servidores que ya tienes no se tocan, y un perfil con el mismo nombre se conserva salvo que pidas reemplazarlo. La configuración anterior se guarda junto a la nueva (`.before-import`) y ngPost se reinicia para usar los nuevos ajustes.

Desde la línea de comandos: `--export_settings archivo` y `--import_settings archivo` (después el programa sale).

Siguiente: [Servidores y conexiones](02-servers.md).
