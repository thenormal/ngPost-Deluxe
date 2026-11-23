# Cola y reanudación

## La cola

Cuando publicas varias releases, esperan su turno en la página **Publicaciones** y se suben una tras otra. El botón **pausa** al pie de la ventana detiene la subida en curso y la reanuda.

## Orden, prioridad y pausa

En la página **Publicaciones** las publicaciones en espera aparecen en el orden en que empezarán, con su puesto: *En espera (1)* es la siguiente. Selecciona una para cambiarla con los botones de arriba o con el botón derecho:

- **Prioridad**: **Alta**, **Normal** (predeterminada) o **Baja**. Una publicación con mayor prioridad pasa delante de las de menor prioridad; no interrumpe la que se está subiendo.
- **Flechas**: mueven una publicación en espera arriba, abajo, al principio o al final de la cola. También puedes arrastrarla con el ratón. Una publicación que mueves se queda donde la pones, sea cual sea su prioridad.
- **Pausa** / **Reanudar**: una publicación en espera en pausa se queda en la cola y las siguientes pasan antes. La que se está subiendo deja de enviar y conserva su puesto: las demás esperan hasta que la reanudes, porque las publicaciones se suben de una en una. Si todavía se está comprimiendo, se pausa cuando empiece su subida.
- **Añadir en pausa**, junto a **Publicar Ficheros** en Quick Post y junto a **Publicar la lista** en Publicación Automática: las publicaciones entran en la cola sin empezar. Reanúdalas cuando quieras.

Mientras se sube una publicación, la siguiente puede comprimirse ya (*Preparando (1)*, luego *Lista (1)*). Esa publicación se queda la primera: no se puede pausar ni mover, y nada puede ir delante. Aun así puedes cancelarla.

## Cola guardada

De forma predeterminada se guardan las publicaciones que no han terminado. Si cierras ngPost, o el ordenador se apaga, en el siguiente inicio una ventana muestra cuántas publicaciones no habían terminado:

- **Reanudar**: se inician de nuevo. Una publicación que se estaba enviando empieza otra vez **desde el principio**.
- **Descartar**: se eliminan.
- **Decidir más tarde**: siguen guardadas y se ofrecerán de nuevo en el siguiente inicio.

Las publicaciones reanudadas aparecen en la página **Publicaciones**.

## Conviene saber

- El orden, la prioridad y la pausa de cada publicación se guardan con la cola: tras un reinicio vuelven tal como las dejaste. En la línea de comandos también se publican las que están en pausa.
- La cola guardada también contiene la contraseña de cada archivo comprimido, en texto claro, como el Historial. Puedes desactivarla en **Ajustes**, categoría **Historial y cola**, tarjeta **Cola de envíos**: esto también borra la cola guardada.
- En la línea de comandos usa `--resume_queue`.
