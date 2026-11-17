## ngPost Deluxe 1.3

Fourth release of **ngPost Deluxe**, a revisited and redesigned version of the Usenet poster [ngPost](https://github.com/mbruel/ngPost) by Matthieu Bruel (GPL v3). The theme of this version is **easier to use**: the program should be simple for someone who opens it for the first time, especially the Auto Posting page, and it should tell you what is happening.

### What's new
- **Auto Posting, redesigned**: three clear choices (post what I choose, scan a folder once, watch a folder and post what arrives), a preview of what will be posted with the sizes, the state of the folder watching always visible (folder, posts of the day, pause button). After posting you choose what happens to the files: nothing (default), **move them to a "Posted" folder**, or delete them.
- **Profiles**: a ready **Recommended** profile (compression, random name and password, par2 10 %) and your own profiles, in Quick Post and Auto Posting.
- **Checks before posting**: a window in plain words with a one-click fix (no server or group, folders without compression, rar / 7z or par2 not found, empty password, NZB folder not writable...).
- **Already posted?** The items that are already in the history are listed in one window, with a "Post again" box (off by default). An existing NZB file gets a number instead of a question.
- **NFO file** written next to the NZB, optionally posted with the files, with the **media information** from MediaInfo (duration, video, audio, subtitles). MediaInfo is included in the Windows installer and in the Docker image.
- **Notifications** when the posts are done or fail: system notification, **Discord**, **Telegram** or a generic **webhook**. One message at the end (default) or one for each post. The messages never contain passwords or file names.
- **First start help**: until a server and a newsgroup are set, a bar says what is missing, with a button to the right page. The **Settings** are now in five categories.
- **New version notice**: once a day the program asks GitHub for the latest release and shows a "Version X available" button in the side bar. Nothing is downloaded, and it can be turned off in the Settings (the command line and Docker never check).
- **Help in 8 languages** (Chinese, Dutch, English, French, German, Italian, Portuguese, Spanish).
- **Feedback** entry in the About window: opens a new GitHub issue already filled in, nothing is sent without you seeing it.
- **Quick Post**: summary line (items, size, size to upload, upload time), a note that the list is posted as one release, and a warning when a post is already in the history.
- **Small windows**: Quick Post and Auto Posting scroll instead of squeezing the cards on top of each other.
- **Installer**: the uninstaller asks whether to also delete your settings and history (default: keep them).
- **Fixes**: the translations compiled in the program were out of date, so some new texts were not shown translated; the diagnostic report now includes the notifications (what is set, messages sent and failed, never the addresses or tokens).

### Download
| File | For |
|---|---|
| `ngPostDeluxe-v1.3-win64-setup.exe` | **Installer** for Windows 64 bit (Windows 10 / 11), 7-Zip included |
| `ngPostDeluxe-v1.3-win32-setup.exe` | **Installer** for Windows 32 bit (par2cmdline instead of ParPar), 7-Zip included |
| `ngPostDeluxe-v1.3-win64.zip` / `ngPostDeluxe-v1.3-win32.zip` | Portable version, no installation |
| `ghcr.io/thenormal/ngpost-deluxe` | **Docker** image (Linux amd64), see [docker/README.md](https://github.com/thenormal/ngPost-Deluxe/blob/master/docker/README.md) |

**Updating from 1.2, 1.1 or 1.0**: run the new installer, it replaces the old version. Your settings and history stay in `%APPDATA%\ngPost Deluxe`.

Windows may show "Windows protected your PC" because the installer is not digitally signed: click **More info**, then **Run anyway**.

If you already use ngPost: copy your `ngPost.conf` into `%APPDATA%\ngPost Deluxe` (the folder is created at the first start) to keep your servers and settings with the installed version. The portable version, started from the folder that contains `ngPost.conf`, keeps using it.

**Known limits**: a post resumed after a restart starts again from the beginning, the pause is not remembered after a restart, and the 32 bit version has not been tested on a real computer. See the Help page for more.

Please test it and report what does not work, or suggest improvements, on the [GitHub page](https://github.com/thenormal/ngPost-Deluxe/issues). If you like the program, there is also a small optional [Ko-fi](https://ko-fi.com/thenormal) page.

### Also in versions 1.2, 1.1 and 1.0
Failed Articles recovered with **Repost missing Articles**, queue that survives a restart, progress bar and colored log, newsgroups with "chips", Docker image with a template for Unraid, new interface (side bar, light and dark themes), History page, Servers page with **Test connection**, much faster compression on Windows, faster par2 with ParPar, Windows installer with 7-Zip included, diagnostic report, interface in 8 languages.
