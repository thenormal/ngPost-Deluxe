## ngPost Deluxe 1.5

Sixth release of **ngPost Deluxe**, a revisited and redesigned version of the Usenet poster [ngPost](https://github.com/mbruel/ngPost) by Matthieu Bruel (GPL v3). The theme of this version is **one list and profiles**: all your posts in one place, profiles that keep everything, and settings you can carry to another computer.

### What's new
- **One list of the posts**: the new page **Posts** shows all the posts together (state, progress bar, graph of the speed, size, remaining time, NZB, password) instead of one tab for each post, with a **log for each post** (time of each line, filters, search, copy). Each post has a **priority**, can be **paused** and **reordered** (buttons, right click, drag and drop), and the queue is kept after a restart. Quick Post is now a form that is empty again after each post.
- **Complete profiles**: a profile also keeps the **name template** (`{name}`, `{date}`, `{time}`, `{n}`), the **newsgroups**, the **servers** and the **NZB folder**; old profiles work as before. Newsgroups and servers can be chosen **for each post** in Quick Post and Auto Posting (also for the watched folders), and your most used newsgroups can be marked as **favorites**.
- **NFO file in six styles** (Scene, Clean, Box, Elegant, Retro and the old Classic) with a banner of the program, a **preview** of an example post to choose the style, and an option for only ASCII characters (`--nfo_style`, `--nfo_ascii`). Scene is the new default.
- **More notifications**: **email** (SMTP with STARTTLS, SSL/TLS or no encryption), **ntfy**, **Gotify**, **Pushover** and **Slack**, next to the system notification, Discord, Telegram and the webhook. One message at the end or one for each post, never the passwords.
- **Export and import of the settings**: Settings, History and queue, or `--export_settings` and `--import_settings`. The file has the settings, profiles, favorite newsgroups, notifications chosen and, if you want, the servers. It never has passwords, tokens, notification addresses or folders of the computer; imported servers stay off until you type their password, and your old configuration is kept as `.before-import`.
- **Watched folders are remembered**: if the program is closed while Auto Posting watches folders, the next start offers to watch them again.
- The Auto Posting page in Watch mode now fits a 1280 px window.

### Download
| File | For |
|---|---|
| `ngPostDeluxe-v1.5-win64-setup.exe` | **Installer** for Windows 64 bit (Windows 10 / 11), 7-Zip included |
| `ngPostDeluxe-v1.5-win32-setup.exe` | **Installer** for Windows 32 bit (par2cmdline instead of ParPar), 7-Zip included |
| `ngPostDeluxe-v1.5-win64.zip` / `ngPostDeluxe-v1.5-win32.zip` | Portable version, no installation |
| `ghcr.io/thenormal/ngpost-deluxe` | **Docker** image (Linux amd64), see [docker/README.md](https://github.com/thenormal/ngPost-Deluxe/blob/master/docker/README.md) |

**Updating from 1.4, 1.3, 1.2, 1.1 or 1.0**: run the new installer, it replaces the old version. Your settings and history stay in `%APPDATA%\ngPost Deluxe`.

Windows may show "Windows protected your PC" because the installer is not digitally signed: click **More info**, then **Run anyway**.

**Known limits**: a post resumed after a restart starts again from the beginning, the email notifications were tested with a test server but not yet with a real mail provider, and the 32 bit version has not been tested on a real computer. See the Help page for more.

Please test it and report what does not work, or suggest improvements, on the [GitHub page](https://github.com/thenormal/ngPost-Deluxe/issues). If you like the program, there is also a small optional [Ko-fi](https://ko-fi.com/thenormal) page.

### Also in versions 1.3, 1.2, 1.1 and 1.0
**1.3**: redesigned Auto Posting, profiles, checks before posting, already-posted warning, NFO file with MediaInfo, notifications (system, Discord, Telegram, webhook), first start help, new version notice, help in 8 languages, feedback entry.

**Before**: Failed Articles recovered with **Repost missing Articles**, queue that survives a restart, progress bar and colored log, newsgroups with "chips", Docker image with a template for Unraid, new interface (side bar, light and dark themes), History page, Servers page with **Test connection**, much faster compression on Windows, faster par2 with ParPar, Windows installer with 7-Zip included, diagnostic report, interface in 8 languages.
