# First steps

This page takes you from a fresh installation to your first post.

## What you need

- An account on a **Usenet provider** (host name, port, user name and password).
- The files or folders you want to post.
- Optionally the tools to compress (**7-Zip** or **RAR**) and to create par2 files (**ParPar** or **par2**). The Windows installer already includes 7-Zip and ParPar.

## Your first post

Until a server and a newsgroup are set, a blue bar at the top of Quick Post and Auto Posting tells you what is missing, with a button that opens the right page. It disappears by itself.

1. Open **Servers**, add your provider and press **Test connection**. See [Servers and connections](02-servers.md).
2. Open **Settings**, category **Posts**, card **Newsgroups**, and add at least one newsgroup. See [Newsgroups](03-newsgroups.md).
3. Open **Quick Post**, press **Select Files** (or drag files and folders into the window).
4. Choose the options you want (compression, par2, password) and press **Post Files**.
5. Follow the progress at the bottom of the window and in the log. At the end the **NZB file** is saved and shown in the log.

The NZB file is the "map" of your post: without it nobody can find the articles, especially if you use obfuscation. Keep it safe.

## Where things are

- **Guided Post**: answer a few questions and the post is prepared for you. See [Guided Post](01b-guided-post.md).
- **Quick Post**: post files and folders now.
- **Auto Posting**: scan or monitor folders.
- **Servers**: your Usenet providers.
- **Settings**: default options of the program, in five categories (General, Posts, NZB files, Notifications, History and queue).
- **History**: everything you posted, with its [statistics](09b-statistics.md).
- **Help**: this section.

## New versions

Once a day, when the program starts, ngPost Deluxe asks GitHub whether a newer release exists (GitHub only sees your IP address). If there is one, a **Version X available** button appears at the bottom of the left bar: click it to open the release page. Nothing is downloaded or installed by itself, and release candidates are never announced.

To turn it off, clear **Look for a new version when the program starts** in **Settings**, category **General**, card **Interface**. The command line version and the Docker image never check.

## Change computer or share a configuration

In **Settings**, category **History and queue**, card **Export and import the settings**, **Export the settings...** writes a text file with the settings of the program, the profiles, the favorite newsgroups, the notifications chosen and, if you want, the servers. **Import the settings...** reads such a file on another computer and shows what it contains before changing anything.

The file never has a password, a token, the address of a notification, the fixed archive password or a folder of the computer. The servers imported are switched off until you type their password in **Servers**. Servers you already have are not touched, and a profile with the same name is kept unless you ask to replace it. The old configuration is kept next to the new one (`.before-import`), and ngPost restarts to use the new settings.

From the command line: `--export_settings file` and `--import_settings file` (then the program exits).

Next: [Servers and connections](02-servers.md).
