# Postagem Rápida

A Postagem Rápida envia os ficheiros e pastas que escolher. Cada separador é uma publicação, por isso pode preparar várias e executá-las em fila.

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

## Perfis

O menu **Perfil** por cima dos cartões define todas as opções de uma só vez.

- **Recomendado**: compressão, nome e palavra-passe aleatórios, par2 com 10 % de redundância. É a escolha de uma instalação nova.
- **Personalizado**: aparece quando muda uma opção à mão.
- **Os seus próprios perfis**: defina as opções, carregue em **Salvar como perfil...** e dê-lhe um nome. **Excluir** remove o perfil escolhido. Os perfis são guardados no ficheiro de configuração (chave `profile`) e são os mesmos na Postagem Rápida e na Postagem Automática.

O cartão **Arquivo NFO** (veja [Ficheiros NZB](09-nzb-files.md)) também é partilhado: a Postagem Rápida e a Postagem Automática mostram as mesmas definições.

## Separadores

O separador **+** abre uma nova publicação. Um separador mostra o seu estado com um ícone: pendente, a enviar, concluído ou erro.

Veja também [Progresso e registo](06-progress-and-log.md).
