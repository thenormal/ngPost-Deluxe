# Fila e retoma

## A fila

Quando publica várias releases, esperam a sua vez na página **Publicações** e são enviadas uma após a outra. O botão **pausa** no fundo da janela para o envio em curso e retoma-o.

## Ordem, prioridade e pausa

Na página **Publicações** as publicações em espera aparecem pela ordem em que vão começar, com o seu lugar: *Em espera (1)* é a próxima. Selecione uma para a alterar com os botões de cima ou com o botão direito:

- **Prioridade**: **Alta**, **Normal** (predefinida) ou **Baixa**. Uma publicação com prioridade mais alta passa à frente das de prioridade mais baixa; não interrompe a que está a ser enviada.
- **Setas**: movem uma publicação em espera para cima, para baixo, para o início ou para o fim da fila. Também a pode arrastar com o rato. Uma publicação que move fica onde a pôs, seja qual for a sua prioridade.
- **Pausa** / **Retomar**: uma publicação em espera posta em pausa fica na fila e as seguintes passam à frente. A que está a ser enviada deixa de enviar e mantém o seu lugar: as outras esperam até a retomar, porque as publicações são enviadas uma de cada vez. Se ainda estiver a ser comprimida, fica em pausa quando o envio começar.
- **Adicionar em pausa**, ao lado de **Postar Arquivos** no Quick Post e de **Publicar a lista** na Postagem Automática: as publicações entram na fila sem começar. Retome-as quando quiser.

Enquanto uma publicação é enviada, a seguinte já pode ser comprimida (*A preparar (1)*, depois *Pronta (1)*). Essa publicação fica em primeiro: não pode ser posta em pausa nem movida, e nada pode passar-lhe à frente. Pode na mesma cancelá-la.

## Fila guardada

Por predefinição, as publicações que não terminaram são guardadas. Se fechar o ngPost, ou o computador desligar, no início seguinte uma janela mostra quantas publicações não tinham terminado:

- **Retomar**: são iniciadas de novo. Uma publicação que estava a ser enviada recomeça **do princípio**.
- **Descartar**: são removidas.
- **Decidir depois**: ficam guardadas e serão oferecidas outra vez no início seguinte.

As publicações retomadas aparecem na página **Publicações**.

## Bom saber

- A ordem, a prioridade e a pausa de cada publicação são guardadas com a fila: depois de um reinício as publicações voltam como as deixou. Na linha de comandos as publicações em pausa também são publicadas.
- A fila guardada também contém a palavra-passe de cada arquivo, em texto simples, tal como o Histórico. Pode desativá-la em **Definições**, categoria **Histórico e fila**, cartão **Fila dos posts**: isto também apaga a fila guardada.
- Na linha de comandos use `--resume_queue`.
