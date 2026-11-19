# Statistics

The statistics turn your [History](09-nzb-files.md) into totals and charts: how much you posted, how fast, when, where and with which options. Everything is computed on your computer from the history file; nothing is sent anywhere.

## Where to find it

Open the **History** page and press **Statistics** (the switch next to **List**). If the history is turned off, or empty, the page says so: turn the history on in the Settings and the statistics appear after your first post.

## The period

The list at the top chooses the period: **Today**, **Last 7 days**, **Last 30 days**, **Last 12 months** or **All**. Everything on the page follows it. A line under the list tells since when the data exists and how many posts it covers.

## What it shows

- **Totals**: eight tiles with the number of posts, the data uploaded, the time spent posting, the share of posts without errors, the average and best speed, the largest post and the average per day.
- **Data uploaded**: the size posted by hour (for **Today**), by day (7 and 30 days) or by month (12 months and **All**).
- **Speed**: average and best speed of the posts, on the same scale.
- **Results**: posts without errors and with errors.
- **When you post (day and hour)**: a grid of the week, the darker the square the more posts started at that hour.
- **Most used newsgroups**, **Servers**, **How the posts were started** (Quick Post, Auto Posting with chosen items, folder scan or watched folder, command line) and **Options** (compression, files posted as they are, par2, password, archives kept).
- **Largest posts** and **Busiest days**: the ten largest posts and the busiest days, with their size and speed.

Move the mouse over a bar or a square to read the exact value.

## Export

The **Export** button saves the period shown:

- **PDF document**: an A4 document to print or read, with the totals, the charts and the rankings.
- **Web page (HTML)**: one file with the charts inside, that you can open in any browser or send by email.
- **Spreadsheet (CSV)**: the exact numbers (bytes, bytes per second, number of posts) to use in a spreadsheet; it uses the same separator as the history.
- **Images of the charts (PNG)**: one image for each chart, in the folder you choose.

The exports always use light colors, so they print well even with the dark theme. They contain the names of your largest posts but never the passwords.

## Good to know

- The servers, the options and how a post was started are recorded since version 1.4. Older posts are not counted in those three charts, and a line under each chart says for how many posts the information is known.
- The sizes of the posts made before version 1.4 are rounded.
- A post made with [Guided Post](01b-guided-post.md) is counted as Quick Post or Auto Posting, because those pages do the actual work.
- If you delete the history file, the statistics start again from zero.
