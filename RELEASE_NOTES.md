## ngPost Deluxe 1.4

Fifth release of **ngPost Deluxe**, a revisited and redesigned version of the Usenet poster [ngPost](https://github.com/mbruel/ngPost) by Matthieu Bruel (GPL v3). The theme of this version is **guided and informed**: the program asks you step by step what to post, and tells you what was posted.

### What's new
- **Guided Post**: a wizard in the side bar that asks what you want to do (one release, each item, scan a folder, watch a folder), what to post, the options and where, then prepares the post with the same checks as Quick Post and Auto Posting. It remembers your last choices and can start several posts together.
- **Statistics** (History page, switch List / Statistics): totals and charts for the period you choose (today, 7 days, 30 days, 12 months, all): data posted, speed, results, busiest days and hours, newsgroups, servers, how the posts were started and options used, plus rankings. Export as a **PDF** document, a **web page**, a **CSV** file or **PNG** images of the charts.
- **History** records more: the servers used, the options (compression, password, par2), how the post was started and the exact size. Old histories stay valid.
- **Speed limit**: a maximum upload speed in MB/s for all the connections and all the posts together (Settings, Posts, Upload; `--max_speed` on the command line), so the rest of your connection stays usable. It can be changed while posting.
- **Remembers your choices**: Quick Post and Auto Posting keep the last options and the selected profile for the next start; the paths chosen with the buttons, the Auto Posting mode and the NFO text are saved automatically.
- **Post button always visible**, also with a long list of files.
- **Quick Post**: a profile with a random name now replaces the name taken from the first file.
- **Clearer compression problems**: a warning when the temporary folder is synchronized (OneDrive and similar), a message when the source files are slow to read, and a clear error when a file or folder was moved before the compression started (7-Zip no longer searches for it on all the drives). The messages of 7-Zip are read correctly with accents and the log shows at most 20 lines of them.
- **Security and fixes**: possible heap overflow in the yEnc encoding buffer fixed, a space or tab at the end of a yEnc line is now escaped correctly, the generated passwords and names use the cryptographic random generator of the system, and the archive password is no longer visible in the list of processes (7-Zip receives it on its standard input, rar through the `RAR` environment variable).

### Download
| File | For |
|---|---|
| `ngPostDeluxe-v1.4-win64-setup.exe` | **Installer** for Windows 64 bit (Windows 10 / 11), 7-Zip included |
| `ngPostDeluxe-v1.4-win32-setup.exe` | **Installer** for Windows 32 bit (par2cmdline instead of ParPar), 7-Zip included |
| `ngPostDeluxe-v1.4-win64.zip` / `ngPostDeluxe-v1.4-win32.zip` | Portable version, no installation |
| `ghcr.io/thenormal/ngpost-deluxe` | **Docker** image (Linux amd64), see [docker/README.md](https://github.com/thenormal/ngPost-Deluxe/blob/master/docker/README.md) |

**Updating from 1.3, 1.2, 1.1 or 1.0**: run the new installer, it replaces the old version. Your settings and history stay in `%APPDATA%\ngPost Deluxe`.

Windows may show "Windows protected your PC" because the installer is not digitally signed: click **More info**, then **Run anyway**.

**Known limits**: a post resumed after a restart starts again from the beginning, the pause is not remembered after a restart, and the 32 bit version has not been tested on a real computer. See the Help page for more.

Please test it and report what does not work, or suggest improvements, on the [GitHub page](https://github.com/thenormal/ngPost-Deluxe/issues). If you like the program, there is also a small optional [Ko-fi](https://ko-fi.com/thenormal) page.

### Also in versions 1.3, 1.2, 1.1 and 1.0
**1.3**: redesigned Auto Posting, profiles, checks before posting, already-posted warning, NFO file with MediaInfo, notifications (system, Discord, Telegram, webhook), first start help, new version notice, help in 8 languages, feedback entry.

**Before**: Failed Articles recovered with **Repost missing Articles**, queue that survives a restart, progress bar and colored log, newsgroups with "chips", Docker image with a template for Unraid, new interface (side bar, light and dark themes), History page, Servers page with **Test connection**, much faster compression on Windows, faster par2 with ParPar, Windows installer with 7-Zip included, diagnostic report, interface in 8 languages.
