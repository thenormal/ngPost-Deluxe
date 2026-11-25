# Línea de comandos

El programa también puede funcionar sin ventana, para un servidor, un NAS o un script. La versión de línea de comandos es el mismo programa compilado sin la ventana (o la imagen de Docker).

## Ejemplos

Publicar dos elementos con compresión, nombre y contraseña aleatorios, y par2:

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Publicar cada elemento de una carpeta, una publicación cada uno:

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Vigilar una carpeta y mover los archivos a una carpeta «Posted» una vez publicados:

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` acepta un nombre como `Posted` (la carpeta se crea junto a cada archivo o carpeta) o una ruta completa. Usa `--rm_posted` en su lugar para borrar los archivos una vez publicados. Ambas solo se aplican cuando todo se publicó sin errores.

## Opciones útiles

- `-c` archivo de configuración (si no, `$HOME/.ngPost`)
- `-g` grupos de noticias, separados por comas
- `-o` archivo NZB de salida
- `-S` servidor en una sola cadena
- `--disp_progress` BAR o FILES
- `--resume_queue` publicar de nuevo las publicaciones no terminadas
- `--allow_repost` publicar de nuevo los archivos que ya están en el historial (de forma predeterminada se omiten)
- `--max_speed MB/s` limita la velocidad de subida, para todas las conexiones juntas (0: sin límite); equivale a las claves `limit_speed` y `max_speed` del archivo de configuración
- `--gen_nfo` escribir un archivo NFO junto al nzb; `--nfo_in_post` publicarlo también; `--nfo_password` escribir en él la contraseña del archivo comprimido; `--nfo_text file.txt` añadir un texto al final; `--no_nfo_media` omitir la información multimedia (MediaInfo); `--mediainfo_path ruta` elegir el programa MediaInfo; `--nfo_style nombre` elegir el aspecto (classic, clean, box, scene, elegant, retro); `--nfo_ascii` solo caracteres ASCII
- `--notify_discord URL`, `--notify_telegram_token T` con `--notify_telegram_chat ID`, `--notify_webhook URL` y `--notify_when each|all_done`: notificaciones al final de las publicaciones (consulta [Progreso y registro](06-progress-and-log.md))
- `--notify_slack URL`, `--notify_ntfy URL`, `--notify_gotify_url URL` con `--notify_gotify_token T`, `--notify_pushover_token T` con `--notify_pushover_user K`, `--notify_email_host`, `--notify_email_port`, `--notify_email_security starttls|ssl|none`, `--notify_email_user`, `--notify_email_pass`, `--notify_email_from`, `--notify_email_to`: los otros canales
- `--export_settings archivo` escribe los ajustes, perfiles, notificaciones y servidores (nunca las contraseñas) en un archivo; `--import_settings archivo` añade ese archivo a la configuración (la anterior se conserva como `.before-import`); ambos salen sin publicar
- `--help` la lista completa

Consulta [Publicación Automática](08-auto-posting.md).
