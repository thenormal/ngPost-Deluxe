# Artículos fallidos y republicación

Durante una subida algunos artículos pueden fallar (se corta una conexión, el servidor los rechaza). ngPost intenta cada artículo varias veces. Si algunos siguen fallando, la publicación termina en naranja y el registro indica cuántos artículos fallaron.

## Republicar los artículos que faltan

Cuando algunos artículos han fallado, la pestaña muestra un botón **Reponer artículos que faltan**. Publica de nuevo **solo esos artículos** y corrige el archivo NZB. No hace falta enviar otra vez toda la publicación.

Tras una republicación correcta se actualiza la lista de archivos y la publicación se pone en verde.

## Consejos

- Si fallan muchos artículos, la causa suele ser la conexión. Reduce el número de conexiones o revisa los límites del proveedor.
- Si los mismos artículos fallan una y otra vez, revisa tu cuenta y los grupos de noticias.
- El número de reintentos se puede configurar con la opción `--retry` (5 por defecto) o en el archivo de configuración.

Consulta [Servidores y conexiones](02-servers.md).
