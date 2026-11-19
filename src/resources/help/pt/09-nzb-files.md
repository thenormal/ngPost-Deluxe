# Ficheiros NZB

Um ficheiro NZB lista os artigos de uma publicação. Qualquer pessoa que o tenha pode descarregar a publicação da Usenet.

## Onde é guardado

A pasta define-se em **Definições**, categoria **Ficheiros NZB**. O ficheiro toma o nome da publicação (o primeiro ficheiro ou pasta).

## Como usá-lo

- Partilhe-o juntamente com a palavra-passe do arquivo, se definiu uma.
- Um programa de descarga (um leitor de notícias) abre-o e descarrega os ficheiros.
- Com a ofuscação, o NZB é a **única** forma de encontrar a sua publicação: guarde uma cópia.

## Verificar um NZB

Na linha de comandos, `--check` verifica se os artigos de um NZB ainda estão disponíveis no servidor.

## Histórico

A página **Histórico** lista os seus envios passados (data, nome, tamanho, velocidade, arquivo, palavra-passe, grupos). Pode desativar o histórico nas Definições. A palavra-passe é guardada em texto simples.

A vista **Estatísticas** desta página (o seletor ao lado de **Lista**) transforma o histórico em totais e gráficos: veja [Estatísticas](09b-statistics.md).

### Publicações já presentes no histórico

Antes de uma publicação começar, o ngPost procura no histórico uma publicação bem-sucedida com o mesmo nome de NZB e o mesmo tamanho total dos ficheiros. Na **Postagem Rápida** uma janela pergunta se a quer publicar de novo (**Postar mesmo assim** ou **Cancelar**). Na **Postagem Automática** e na linha de comandos a publicação é ignorada e uma linha no registo di-lo. Para a publicar mesmo assim, use a opção `--allow_repost` ou desative o aviso em **Definições**, categoria **Histórico e fila** (cartão Histórico dos envios). A verificação precisa de o histórico estar ativado e só olha para o nome e o tamanho: uma pasta renomeada não é reconhecida.

## Ficheiro NFO

Um NFO é um pequeno ficheiro de texto que descreve a publicação: o seu nome, a data, os ficheiros com os respetivos tamanhos, como foi preparada (arquivo e par2) e um texto seu. Ative-o no cartão **Arquivo NFO** da Postagem Rápida ou da Postagem Automática (é o mesmo cartão: as definições são partilhadas).

- **Escrever um ficheiro NFO junto ao nzb**: `nome.nfo` é guardado na mesma pasta que `nome.nzb`.
- **Publicar também o ficheiro NFO com os ficheiros**: é enviado com a publicação e aparece no NZB.
- **Incluir informação multimédia (MediaInfo)**: ativado por predefinição. Para os ficheiros de vídeo e áudio, o NFO lista a duração, o vídeo (formato, tamanho, taxa de fotogramas, HDR), as faixas de áudio (idioma, formato, canais, taxa de bits) e as legendas. No máximo são descritos 20 ficheiros (os outros são contados) e a leitura pára ao fim de 20 segundos. Usa o programa gratuito MediaInfo: vem com o instalador do Windows e a imagem Docker; em Linux e macOS instale o pacote `mediainfo`, ou escolha o programa nas Definições. Sem o MediaInfo, o NFO simplesmente não tem secção multimédia.
- **Escrever a palavra-passe do arquivo**: desativado por predefinição. Num NFO publicado, a palavra-passe pode ser lida por quem descarregar a publicação.
- **Texto acrescentado no fim**: notas, ligações, o que quiser dizer. O mesmo texto é usado em todas as publicações.

Quando os nomes dos ficheiros estão escondidos (ofuscação dos artigos ou dos nomes dos ficheiros), o NFO que é publicado não lista nenhum nome de ficheiro nem informação multimédia, e o próprio ficheiro recebe um nome aleatório. Na linha de comandos use `--gen_nfo`, `--nfo_in_post`, `--nfo_password` e `--nfo_text file.txt`, mais `--no_nfo_media` e `--mediainfo_path caminho`.
