# Primeiros passos

Esta página leva-o desde uma instalação nova até à sua primeira publicação.

## O que precisa

- Uma conta num **fornecedor de Usenet** (nome do host, porta, nome de utilizador e palavra-passe).
- Os ficheiros ou pastas que quer publicar.
- Opcionalmente, as ferramentas para comprimir (**7-Zip** ou **RAR**) e para criar ficheiros par2 (**ParPar** ou **par2**). O instalador do Windows já inclui o 7-Zip e o ParPar.

## A sua primeira publicação

Enquanto não houver um servidor e um newsgroup definidos, uma barra azul no topo de Postagem Rápida e de Postagem Automática diz-lhe o que falta, com um botão que abre a página certa. Desaparece sozinha.

1. Abra **Servidores**, adicione o seu fornecedor e carregue em **Testar ligação**. Veja [Servidores e ligações](02-servers.md).
2. Abra **Definições**, categoria **Publicações**, cartão **Newsgroups**, e adicione pelo menos um newsgroup. Veja [Newsgroups](03-newsgroups.md).
3. Abra **Postagem Rápida**, carregue em **Selecionar ficheiros** (ou arraste ficheiros e pastas para a janela).
4. Escolha as opções que quer (compressão, par2, palavra-passe) e carregue em **Postar Arquivos**.
5. Acompanhe o progresso na parte de baixo da janela e no registo. No fim, o **ficheiro NZB** é guardado e mostrado no registo.

O ficheiro NZB é o «mapa» da sua publicação: sem ele ninguém consegue encontrar os artigos, sobretudo se usar a ofuscação. Guarde-o bem.

## Onde está cada coisa

- **Postagem guiada**: responda a algumas perguntas e a postagem é preparada para você. Veja [Postagem guiada](01b-guided-post.md).
- **Postagem Rápida**: publica já ficheiros e pastas.
- **Postagem Automática**: analisa ou vigia pastas.
- **Servidores**: os seus fornecedores de Usenet.
- **Definições**: opções predefinidas do programa, em cinco categorias (Geral, Publicações, Ficheiros NZB, Notificações, Histórico e fila).
- **Histórico**: tudo o que publicou, com as suas [estatísticas](09b-statistics.md).
- **Ajuda**: esta secção.

## Novas versões

Uma vez por dia, ao iniciar o programa, o ngPost Deluxe pergunta ao GitHub se existe uma versão mais recente (o GitHub só vê o seu endereço IP). Se existir, aparece um botão **Version X available** na parte de baixo da barra da esquerda: clique para abrir a página da versão. Nada é descarregado nem instalado sozinho, e as versões candidatas nunca são anunciadas.

Para desativar, desmarque **Procurar uma nova versão ao iniciar o programa** em **Definições**, categoria **Geral**, cartão **Interface**. A versão de linha de comandos e a imagem Docker nunca fazem esta verificação.

Seguinte: [Servidores e ligações](02-servers.md).
