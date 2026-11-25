# Roadmap

This is a working plan, not a promise: there are no release dates, and what is listed in the versions not released yet may change, move or never happen. Releases are numbered with two numbers (1.2, 1.3...). What each released version changed is in the [changelog](CHANGELOG.md).

Your opinion counts: if an idea matters to you (or something is missing), say it in the [issues](https://github.com/thenormal/ngPost-Deluxe/issues).

Legend: **Done**, **In progress**, **Not started**.

## Released

| Version | Theme |
|---|---|
| 1.5 | One list and profiles: one list of the posts, complete profiles, NFO in six styles, more notifications, export and import of the settings |
| 1.4 | Guided and informed: Guided Post, statistics, speed limit, more history data |
| 1.3 | Easier to use: Auto Posting redesigned, profiles, checks before posting, notifications, Help in 8 languages |
| 1.2 | Reliability: tests, retry of failed Articles, persistent queue, colored log, Help section |

Details of each version: [CHANGELOG.md](CHANGELOG.md).

## Version 1.6 - Verification and resume (not started)

- **A post is finished when it is verified**: check that the Articles are on the server (button, optional automatic), also **on a different server** from the one used for the upload, show the result in the history, and post again by itself what is missing
- **Availability check** of the old NZB files, by itself and from time to time (warns when a post starts to disappear from the servers), and a **"Check an NZB" tool**: drop any NZB and see the percentage that is still available
- A post that stops **continues from the last Article sent** instead of starting again
- **Backup servers and priority**: if a server fails, the posts continue on another one by themselves (it extends the second round of Articles, which already tries another server)
- **Speed test for each server** and **automatic tuning** of the number of connections and of the Article size, from the measures of the speed and the latency
- **Actions from the history**: check again, post again, write the NFO file again

## Version 1.7 - Web panel and automation (not started)

- **Web panel for Docker and NAS**: queue, progress, history and pause from a browser (access with password, local network only by default), with a **REST API, a health check and metrics** (for Grafana, Uptime Kuma and similar)
- **VPN guard**: pause the posts when the VPN connection drops (the real kill-switch stays in the firewall or in the VPN application)
- **Schedule**: post only at some hours or days, with a speed limit by **time slots** (for example full speed at night, limited during the day) and a limit for each server or each post
- **Rules for each folder** in Auto Posting: another profile or newsgroups for each folder or extension, wait until the file is complete
- **After the post**: send the NZB to a folder, to an address (URL) or to SABnzbd / NZBGet
- **Temporary files in RAM**: easier on Windows, and chosen automatically when there is room

## Version 1.8 - Safety and reach (not started)

- **History with encrypted passwords** (from a main password) and the passwords of the servers in the credential store of the system, instead of clear text in the files
- **More checks before posting**: free disk space, test of the archive, and a "simulate" that shows archive, parts and time
- **Traffic counter** with a warning near the quota of the provider
- **More languages**, with a simple way for the community to translate
- **Digitally signed Windows installer** (Windows would stop warning about an unknown program)

## Later, no version

- Move to Qt 6 and split the biggest source file: a version of its own, because it changes the base of the program (it would also allow a newer OpenSSL on Windows)
- Update from inside the program (the check for a new version exists since 1.3; this would download it, with a checksum, and ask before anything is installed): after the signed installer
- Contextual help and search in the Help section
- The list of the newsgroups of a server (download it and choose in it): slow and different from one provider to another
