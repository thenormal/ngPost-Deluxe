# Postagem Rápida

O Quick Post prepara uma publicação: os ficheiros e pastas da lista formam uma release. Ao premir **Postar Arquivos**, a publicação passa para a página **Publicações** e o Quick Post fica de novo vazio, pronto para a release seguinte.

## Passos

1. Adicione ficheiros e pastas com **Selecionar ficheiros** ou arrastando-os.
2. Escolha as opções (abaixo).
3. Carregue em **Postar Arquivos**.

## Opções

- **Comprimir**: coloca os ficheiros num arquivo RAR ou 7z antes de publicar. Pode dividi-lo em volumes.
- **Palavra-passe**: o arquivo pode ter uma palavra-passe, fixa ou gerada ao acaso. A palavra-passe é guardada com a publicação (por exemplo, nos metadados do NZB e no Histórico).
- **Nome aleatório**: dá ao arquivo um nome aleatório.
- **par2**: cria ficheiros de recuperação. Defina a redundância em percentagem: mais redundância permite ao descarregador reparar mais artigos em falta, mas envia mais dados.
- **Ofuscação**: esconde os assuntos dos artigos. Sem o ficheiro NZB ninguém consegue encontrar a sua publicação, nem sequer você.

## Verificações e resumo

Por baixo dos cartões, uma linha diz-lhe o que vai ser publicado: o número de elementos, o seu tamanho e, com compressão ou par2, aproximadamente quanto é enviado. O tempo de envio aparece depois da primeira publicação da sessão, a partir da velocidade real desse envio (antes, o ngPost não adivinha).

Quando carrega em **Postar Arquivos**, o ngPost verifica primeiro a publicação. Se algo estiver mal, uma janela explica-o em palavras simples, com um botão que o corrige:

- **Erros** (a publicação não pode começar): nenhum servidor ativado, nenhum newsgroup válido, pastas sem compressão, programa rar / 7z ou par2 não encontrado, uma palavra-passe ativada mas vazia, uma pasta NZB sem permissão de escrita.
- **Avisos** (pode escolher **Postar mesmo assim**): a pasta temporária não existe ou pode ser demasiado pequena, o MediaInfo não foi encontrado para a informação multimédia do NFO.

Depois de uma correção, as verificações voltam a ser feitas, por isso um problema novo aparece logo em vez de surgir a meio da publicação. A Postagem Automática ainda não tem esta janela.

## Nome, newsgroups e servidores

O cartão **NZB** tem mais três linhas. Vazia, cada uma usa o que está nas Definições.

- **Nome**: um modelo para o nome da publicação (o ficheiro NZB e, sem nome aleatório, o arquivo): `{name}` é o nome do ficheiro ou pasta, `{date}` a data (2026-11-23), `{time}` a hora (15-04) e `{n}` um número que sobe um em cada publicação. Exemplo: `{name} [{date}]`.
- **Newsgroups**: os grupos desta publicação, com os favoritos (ver [Newsgroups](03-newsgroups.md)).
- **Servidores**: todos os servidores ativos (por omissão) ou só os que marcar no menu. Uma publicação mantém os seus servidores na fila, mesmo depois de reiniciar.

Estas linhas são lembradas ao fechar o programa.

## Perfis

O menu **Perfil** por cima dos cartões define todas as opções de uma só vez.

- **Recomendado**: compressão, nome e palavra-passe aleatórios, par2 com 10 % de redundância. É a escolha de uma instalação nova.
- **Personalizado**: aparece quando muda uma opção à mão.
- **Os seus próprios perfis**: defina as opções, carregue em **Salvar como perfil...** e dê-lhe um nome. **Excluir** remove o perfil escolhido. Os perfis são guardados no ficheiro de configuração (chave `profile`) e são os mesmos na Postagem Rápida e na Postagem Automática.

O cartão **Arquivo NFO** (veja [Ficheiros NZB](09-nzb-files.md)) também é partilhado: a Postagem Rápida e a Postagem Automática mostram as mesmas definições.

Ao guardar um perfil também pode guardar o modelo de nome, os newsgroups, os servidores e a pasta NZB (marque o que quiser na janela). Uma parte que não é guardada não é alterada quando escolhe o perfil. Passe o rato sobre um perfil do menu para ver o que contém.

## Depois de Postar Arquivos

A publicação entra na fila e o programa mostra a página **Publicações**, onde acompanha o progresso e o registo. O Quick Post mantém as suas opções e cria um novo nome e uma nova palavra-passe aleatórios, para preparar logo a release seguinte. Com **Adicionar em pausa** marcado, a publicação espera na fila até a retomar.

Veja também [Progresso e registo](06-progress-and-log.md).
