# Postagem Automática

A Postagem Automática publica muitos ficheiros e pastas sem preparar cada um à mão. Escolha um dos três botões no topo da página; só são mostradas as definições de que precisa.

## Postar o que eu escolher

Adicione os ficheiros e pastas que quer (arraste-os para a lista, ou use **Selecionar ficheiros** / **Selecione uma Pasta**) e carregue em **Publicar a lista**: cada ficheiro ou pasta torna-se uma publicação separada.

## Analisar uma pasta uma vez

Escolha uma pasta e carregue em **Analisar**: o que ela contém é listado. As pastas precisam de **compressão**. Use **primeiro os ficheiros mais recentes** para ordenar por data.

## Vigiar uma pasta e publicar o que chega

Escolha a pasta e carregue em **Começar a vigiar**: cada novo ficheiro ou pasta é publicado quando chega. Carregue em **Parar de vigiar** para parar. Parar com publicações ainda em espera pede confirmação.

Enquanto vigia, uma barra por baixo das três escolhas mostra sempre a pasta vigiada, quantas publicações terminaram hoje (e quantas com erros) e quantas estão em curso ou em espera. **Pausar a vigilância** faz os ficheiros novos esperarem: são publicados quando carrega em **Retomar a vigilância**; as publicações já iniciadas continuam. Parar a vigilância esquece os ficheiros que estavam em espera.

## A lista

Cada elemento tem uma caixa e o seu tamanho. Desmarque um elemento para o deixar de fora; só os elementos marcados são publicados. Para remover elementos da lista, selecione-os e carregue em DEL. A linha por baixo da lista mostra quantos elementos estão selecionados, o seu tamanho, o tamanho a enviar e, depois de uma primeira publicação, o tempo de envio.

## Elementos já publicados

Se alguns elementos marcados já estão no histórico (mesmo nome e tamanho), uma janela lista-os com a data da publicação anterior. Marque **Publicar de novo** para os que quer publicar mais uma vez; os outros não são publicados e ficam na lista, desmarcados e a cinzento. Se já existir um ficheiro NZB com o mesmo nome, o novo chama-se `nome_1.nzb` e assim por diante. Ao vigiar uma pasta, os elementos que já estão no histórico são ignorados, com uma linha no registo. Pode desativar o aviso em **Definições** (Histórico).

## Antes de começar

Quando carrega em **Publicar a lista** ou **Começar a vigiar**, o programa verifica as definições uma só vez para todo o grupo (um servidor e um newsgroup, compressão para as pastas, os programas rar / 7z e par2, a pasta do NZB, a pasta temporária). Os problemas são mostrados em palavras simples com um botão que os corrige; os avisos podem ser ignorados com **Postar mesmo assim**. É a mesma janela da [Postagem Rápida](04-quick-post.md).

## Dicas

- Use compressão, nomes aleatórios, palavras-passe aleatórias e par2: é a configuração habitual para a publicação automatizada. O perfil **Recomendado** (menu **Perfil**) define-a com um clique.
- **Depois de publicar** (a linha por cima do botão Publicar): não fazer nada (predefinido), mover os ficheiros para uma **pasta «Posted»**, ou apagá-los. O nome predefinido `Posted` cria essa pasta junto a cada ficheiro ou pasta; também pode escolher outra pasta. Nada é movido ou apagado se a publicação não terminou sem erros. Um nome já usado na pasta nunca é substituído: o ficheiro passa a `nome (2)`. A análise e a vigilância de pastas ignoram a pasta «Posted». Apagar é para sempre: use-o só quando tiver a certeza.
- Na linha de comandos, a mesma escolha é `--move_posted` ou `--rm_posted` (veja [Linha de comandos](10-command-line.md)).
- O programa lembra-se do último botão que usou.
- Para um NAS ou um servidor sem ecrã, a linha de comandos e a imagem Docker fazem o mesmo trabalho.

Veja [Linha de comandos](10-command-line.md).
