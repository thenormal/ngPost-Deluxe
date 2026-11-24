# Linha de comandos

O programa também pode funcionar sem janela, para um servidor, um NAS ou um script. A versão de linha de comandos é o mesmo programa compilado sem a janela (ou a imagem Docker).

## Exemplos

Publicar dois elementos com compressão, nome e palavra-passe aleatórios, e par2:

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Publicar cada elemento de uma pasta, uma publicação cada um:

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Vigiar uma pasta e mover os ficheiros para uma pasta «Posted» depois de publicados:

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` aceita um nome como `Posted` (a pasta é criada junto a cada ficheiro ou pasta) ou um caminho completo. Use `--rm_posted` em vez disso para apagar os ficheiros depois de publicados. Ambos só se aplicam quando tudo foi publicado sem erros.

## Opções úteis

- `-c` ficheiro de configuração (caso contrário `$HOME/.ngPost`)
- `-g` newsgroups, separados por vírgulas
- `-o` ficheiro NZB de saída
- `-S` servidor numa só cadeia
- `--disp_progress` BAR ou FILES
- `--resume_queue` publicar de novo as publicações não terminadas
- `--allow_repost` publicar de novo os ficheiros que já estão no histórico (por predefinição são ignorados)
- `--max_speed MB/s` limita a velocidade de envio, para todas as ligações em conjunto (0: sem limite); equivale às chaves `limit_speed` e `max_speed` do ficheiro de configuração
- `--gen_nfo` escrever um ficheiro NFO junto ao nzb; `--nfo_in_post` publicá-lo também; `--nfo_password` escrever nele a palavra-passe do arquivo; `--nfo_text file.txt` acrescentar um texto no fim; `--no_nfo_media` omitir a informação multimédia (MediaInfo); `--mediainfo_path caminho` escolher o programa MediaInfo; `--nfo_style nome` escolher o aspeto (classic, clean, box, scene, elegant, retro); `--nfo_ascii` só caracteres ASCII
- `--notify_discord URL`, `--notify_telegram_token T` com `--notify_telegram_chat ID`, `--notify_webhook URL` e `--notify_when each|all_done`: notificações no fim das publicações (veja [Progresso e registo](06-progress-and-log.md))
- `--help` a lista completa

Veja [Postagem Automática](08-auto-posting.md).
