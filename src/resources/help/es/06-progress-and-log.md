# Progreso y registro

## La barra de progreso

La barra de la parte inferior muestra la fase actual:

- **Compresión**: creación del archivo comprimido.
- **par2**: creación de los archivos de recuperación.
- **Subida**: envío de los artículos. Aquí también ves la velocidad, el tamaño, el número de artículos y el **ETA** (tiempo restante estimado).
- **Completado**, **En pausa** o **Detenido**.

El color indica el estado: azul mientras trabaja, gris en pausa, verde al completarse, naranja si algunos artículos fallaron, rojo si la publicación se detuvo con un error. El porcentaje también se muestra en el título de la ventana.

El ETA es una estimación basada en la velocidad reciente: cambia cuando cambia la velocidad.

## El registro

El registro colorea las líneas: las fases, los éxitos, los avisos y los errores se distinguen fácilmente. Al final de una publicación muestra un resumen con el tiempo de cada fase (compresión, par2, subida) y el total.

## Latencia

Al final de una publicación el registro también muestra la **latencia**: el tiempo mediano de un viaje de ida y vuelta con el servidor (el comando POST y su respuesta, sin datos) y de la conexión. Se mide solo a partir de lo que ngPost ya hace, no se envía nada para ello. Un valor alto (más de medio segundo) significa que el retraso viene de la red, de la distancia al servidor o del propio servidor, por ejemplo una VPN, y no de tu ordenador.

## Informe de diagnóstico

Si algo no funciona, el botón **Informe de diagnóstico**, debajo del registro en Publicación Rápida, guarda un informe de lo que ha hecho el programa, con una sección **Latencia** para cada servidor (conexión, SSL, inicio de sesión, viaje de ida y vuelta: mínimo, mediana, percentil 95 y máximo) y una sección **Notificaciones** (qué se notifica, si la notificación del sistema está activada y disponible, qué canales están configurados y el último fallo; las direcciones y los tokens nunca se escriben). Es un archivo de texto que puedes adjuntar cuando pides ayuda. Revísalo antes de compartirlo: no debería contener tu contraseña.

## Notificaciones

ngPost puede avisarte cuando tus publicaciones terminan, para que no tengas que vigilar la ventana. Se configura en **Ajustes**, categoría **Notificaciones**, en dos partes: **qué notificar** (una publicación terminada, una publicación con errores o fallida, un mensaje al final o uno por cada publicación) y **adónde enviar los mensajes**. En el programa con ventana, la notificación del sistema está activada desde el principio; los demás canales necesitan una dirección.

- **Notificación del sistema**: un mensaje desde el icono del área de notificaciones (solo en la versión de escritorio; activada de forma predeterminada, puedes desactivarla). En Windows aparece como cualquier otra notificación y se queda en el centro de notificaciones; si no la ves, comprueba que las notificaciones y la asistencia de concentración («No molestar») de Windows no la oculten.
- **Discord**: pega la dirección de un webhook de Discord.
- **Telegram**: el token de tu bot y el id del chat que recibe los mensajes.
- **Otro webhook**: cualquier dirección que acepte un POST JSON con un `title` y un `text` (herramientas tipo Slack, ntfy, Home Assistant).

Elige qué se notifica (terminadas, errores o fallos) y cómo: **un mensaje cuando todas las publicaciones han terminado** (predeterminado, lo mejor para Publicación Automática) o **un mensaje después de cada publicación**. El botón **Enviar un mensaje de prueba** muestra enseguida si funciona.

El mensaje contiene el nombre de la publicación, el resultado, el tamaño, el tiempo y la velocidad. Nunca contiene la contraseña ni los nombres de los archivos. Las direcciones y el token se guardan en texto claro en el archivo de configuración, como las demás contraseñas: mantenlo privado. Si un mensaje no se puede enviar, una línea del registro lo indica y la publicación no se ve afectada.

En la línea de comandos, o en Docker, usa las opciones `--notify_discord`, `--notify_telegram_token`, `--notify_telegram_chat`, `--notify_webhook` y `--notify_when each|all_done`, o las claves `notify_*` del archivo de configuración.
