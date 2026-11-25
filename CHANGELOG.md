# Changelog

What changed in each version of ngPost Deluxe, in more detail than the [release notes](https://github.com/thenormal/ngPost-Deluxe/releases). What is planned is in the [roadmap](ROADMAP.md).

Newest first.

## Version 1.5 - One list and profiles (not released yet)

Goal: one place for all the posts, profiles that keep everything, and a program that is easy to move to another computer.

- **One list of the posts**, a page Posts in the side bar (state, progress, size, speed, remaining time, NZB, password, details) instead of one tab for each post; Quick Post is a form that is empty again after each post
- **Priority, reordering and pause for each post** (buttons, right click and drag and drop in the page Posts; "Add paused" in Quick Post and Auto Posting; kept in the saved queue after a restart)
- **Complete profiles**: a profile can also keep the name template ({name}, {date}, {time}, {n}), the newsgroups, the servers and the NZB folder (each part is optional; old profiles work as before). **Newsgroups and servers for each post** in Quick Post and Auto Posting (also for the watched folders), **favorite newsgroups** (star)
- **NFO file in six styles** (Scene, Clean, Box, Elegant, Retro and the old Classic) with a banner of the program, a preview of an example post in the program to choose the style, and an option for only ASCII characters (`--nfo_style`, `--nfo_ascii`)
- **Watched folders are remembered**: if the program is closed while Auto Posting watches folders, the next start offers to watch them again (Watch again / Forget / Decide later)
- **A progress bar and a graph of the speed for each post** (page Posts; the graph is kept until the program is closed)
- **Log for each post** with the time of each line, filters by category (Debug hidden at first), search and copy
- **More notifications**: email (SMTP with STARTTLS, SSL/TLS or no encryption), ntfy, Gotify, Pushover and Slack (next to system, Discord, Telegram and webhook), with the same rules: one message at the end or one for each post, never the passwords; none of the addresses, tokens or passwords is exported with the settings
- **Export and import of the settings and the profiles** (Settings, History and queue; `--export_settings`, `--import_settings`): settings, profiles, favorite newsgroups, notifications chosen and, if wanted, servers; never passwords, tokens, notification addresses or folders of the computer; imported servers stay off until the password is typed; the old config is kept as `.before-import`
- Auto Posting, Watch mode: the page fits a 1280 px window (narrower number boxes in the Archive card)

## Version 1.4 - Guided and informed

Goal: help the user to post step by step, and show what was posted.

- **Guided Post**: a wizard in the side bar (what to do, what to post, options, where) with the same checks as Quick Post and Auto Posting; it remembers the last choices and can start several posts together
- **Statistics** in the History page (List / Statistics): totals and charts for a period, rankings, export as PDF, web page, CSV or PNG images
- **History records more**: servers used, options, how the post was started, exact size (old histories stay valid)
- **Speed limit**: one limit in MB/s for all the connections and all the posts together (Settings, Posts, Upload; `--max_speed`, keys `limit_speed` and `max_speed`), it can be changed while posting; shown in the diagnostic report.
- Quick Post: a profile with a random name replaces the name taken from the first file
- **Remembers the last choices**: options and selected profile of Quick Post and Auto Posting, paths, Auto Posting mode and NFO text are saved automatically; with automatic compression (`PACK`) the matching profile is shown
- The Post button is always visible (fixed bar under the scrolling page)
- Compression: warning for a synchronized temporary folder, text of 7-Zip read in the console code page, message when the source files are slow to read, a missing source stops the compression with a clear error, no `-r` for 7-Zip (it searched missing paths on all the drives), at most 20 lines of messages from the compressor in the log

## Version 1.3 - Easier to use

Goal: the program should be simple for someone who opens it for the first time, especially the Auto Posting page, and it should tell you what happens.

#### Auto Posting, redesigned (the biggest part of this version)

- Three clear choices at the top, in plain words: post what I choose, scan a folder once, watch a folder and post what arrives
- A ready profile, "Recommended for automatic posting" (compression, random name and password, par2 10 %), with the single settings under "Advanced options"
- Checks in plain words before starting (for example: folders need compression), fixed with one click instead of an error message
- Preview of what will be posted (sizes), with a box to leave an item out
- After posting, the user chooses: do nothing (default), **move to a "Posted" folder** (never overwrites, ignored by the scan and the watching; `--move_posted` and `move_posted` in the configuration file), or delete the files
- Items already in the history found when posting a list (scan or choice): one window lists them (name, size, date of the previous post) with a "Post again" box, off by default, instead of one question for each post; the ones not posted again stay in the list, unchecked and grey. An existing NZB file now gets a number (`name_1.nzb`) instead of a question. The folder watching keeps skipping them with a line in the log
- State of the folder watching always visible (folder, how many posts today), with a pause button

#### Everything else

- **Profiles** of options (menu in Quick Post and Auto Posting: Recommended, Custom and the profiles saved by the user), one **NFO file** card shared by Quick Post and Auto Posting (moved from Settings), par2 starts at 10 % and the password is in the Archive card of Quick Post
- **Quick Post**: checks before posting (a window in plain words with a one-click fix: no server or group, folders without compression, rar / 7z or par2 not found, empty password, NZB folder not writable; warnings with "Post anyway": temporary folder, MediaInfo) and a summary line (items, size, size to upload, upload time from the real speed of the last upload). Auto Posting has the same checks, once for the whole group and also before watching a folder
- Fix: the "One release" note of Quick Post must not appear during or after the upload (the list then shows the volumes of the archive)
- Fix: in a Quick Post tab whose post is finished, adding new files should show the "One release" note again (the finished state is not reset when the user adds files)
- Uninstaller: ask whether to also delete the settings and the history kept in `%APPDATA%\ngPost Deluxe` (default: keep them)
- **Feedback** entry in the About window: report a bug or suggest an idea. It opens the new-issue page of GitHub already filled in (no access key in the program, nothing is sent without the user seeing it), with the diagnostic report to attach and issue forms in the repository
- Warning when a post is already in the history (same name and size), before it is posted again (Quick Post asks, auto posting and command line skip it; `--allow_repost` and a Settings option turn it off)
- NFO file generator for a post: written next to the nzb, optionally posted with the files, optional password and free text (Settings, card NFO file; `--gen_nfo`). the options are in the shared options card of Quick Post and Auto Posting
- Media information in the NFO file with MediaInfo (duration, video, audio, subtitles of up to 20 files; on by default, `--no_nfo_media`, `--mediainfo_path`). MediaInfo is shipped in the Windows installer and the Docker image
- **Notifications** when the posts are done or fail: system notification (GUI), Discord, Telegram and a generic webhook; one message when all the posts are done (default) or after each post; the addresses and tokens are treated like passwords
- **First start help**: until a server and a newsgroup are set, a bar on Quick Post and Auto Posting says what is missing, with a button to the right page (the "Open Settings" fix of the checks now opens the right category)
- **New version notice**: once a day the GUI asks GitHub for the latest release and shows a "Version X available" button in the sidebar that opens the release page (nothing is downloaded; release candidates are ignored; Settings, General, Interface turns it off; command line and Docker never check; key `check_updates`)
- **Help in 8 languages**: the 13 pages of the Help section are translated (Chinese, Dutch, English, French, German, Italian, Portuguese, Spanish); a page that is missing falls back to English. A test checks that every translated page has the same headings, links and command examples as the English one
- **Settings in five categories** (General, Posts, NZB files, Notifications, History and queue), saved automatically
- **Small windows**: Quick Post and Auto Posting scroll instead of squeezing the cards on top of each other; the files list shrinks first
- Fix: the translations compiled in the program (`.qm`) were out of date, so some new texts were never shown translated; a test now checks them
- **Notifications in the diagnostic report**: what is notified, system notification on/available and shown, which channels are set (yes/no only, never the addresses or tokens), messages sent and failed this session, last failure

## Version 1.2 - Reliability

Goal: a post should not be lost because of a bad connection, a crash or a restart, and it should be easy to see what is going on.

- Automated tests on every change (command line build and a fake Usenet server)
- Articles that fail are tried again by another connection or another server, with a second round at the end of the post
- **Post the missing Articles again** with one click (the nzb file is fixed, nothing is posted twice)
- **Persistent queue**: the posts that are not finished come back after a restart (resume, discard or decide later)
- Colored log and duration of each phase (compression, par2, upload)
- Progress bar with the phase, the remaining time and colors
- Newsgroups box with "chips" (suggestions and checks) and a menu for where the posts go when there are several groups
- Help section inside the program (offline, in English; other languages can be added later)
- Guide for Docker / Unraid behind a VPN container (gluetun)
- Latency measures in the diagnostic report
