# Command line

The program can also run without a window, for a server, a NAS or a script. The command line version is the same program built without the window (or the Docker image).

## Examples

Post two items with compression, random name and password, and par2:

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Post every item of a folder, one post each:

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Watch a folder and move the files to a "Posted" folder once posted:

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` takes a name like `Posted` (the folder is created next to each file or folder) or a full path. Use `--rm_posted` instead to delete the files once posted. Both only apply when everything was posted without error.

## Useful options

- `-c` configuration file (otherwise `$HOME/.ngPost`)
- `-g` newsgroups, separated by commas
- `-o` NZB output file
- `-S` server in one string
- `--disp_progress` BAR or FILES
- `--resume_queue` post again the posts not finished
- `--allow_repost` post again the files that are already in the history (by default they are skipped)
- `--max_speed MB/s` limit the upload speed, for all the connections together (0: no limit); the same as the keys `limit_speed` and `max_speed` of the configuration file
- `--gen_nfo` write an NFO file next to the nzb; `--nfo_in_post` also post it; `--nfo_password` write the archive password in it; `--nfo_text file.txt` add a text at the end; `--no_nfo_media` leave out the media information (MediaInfo); `--mediainfo_path path` choose the MediaInfo program; `--nfo_style name` choose the look (classic, clean, box, scene, elegant, retro); `--nfo_ascii` only ASCII characters
- `--notify_discord URL`, `--notify_telegram_token T` with `--notify_telegram_chat ID`, `--notify_webhook URL` and `--notify_when each|all_done`: notifications at the end of the posts (see [Progress and log](06-progress-and-log.md))
- `--notify_slack URL`, `--notify_ntfy URL`, `--notify_gotify_url URL` with `--notify_gotify_token T`, `--notify_pushover_token T` with `--notify_pushover_user K`, `--notify_email_host`, `--notify_email_port`, `--notify_email_security starttls|ssl|none`, `--notify_email_user`, `--notify_email_pass`, `--notify_email_from`, `--notify_email_to`: the other channels
- `--export_settings file` write the settings, profiles, notifications and servers (never the passwords) in a file; `--import_settings file` add such a file to the configuration (the old one is kept as `.before-import`); both exit without posting
- `--help` the full list

See [Auto Posting](08-auto-posting.md).
