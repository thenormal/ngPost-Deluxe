# Queue and resume

## The queue

When you post several releases, they wait for their turn in the page **Posts** and are uploaded one after the other. The **pause** button at the bottom of the window stops the upload in progress and starts it again.

## Order, priority and pause

In the page **Posts** the waiting posts are shown in the order they will start, with their place: *Waiting (1)* is the next one. Select a post to change it with the buttons at the top or with a right click:

- **Priority**: **High**, **Normal** (default) or **Low**. A post with a higher priority goes before the posts with a lower one; it doesn't stop the post being uploaded.
- **Arrows**: move a waiting post up, down, to the top or to the bottom of the queue. You can also drag it with the mouse. A post you move stays where you put it, whatever its priority.
- **Pause** / **Resume**: a waiting post that is paused stays in the queue and the next posts go before it. The post being uploaded stops sending and keeps its place: the other posts wait until you resume it, because the posts are uploaded one at a time. If it is still being compressed, it pauses when its upload starts.
- **Add paused**, next to **Post Files** in Quick Post and next to **Post the list** in Auto Posting: the posts go in the queue without starting. Resume them when you want.

While a post is uploaded, the next one can already be compressed (*Preparing (1)*, then *Ready (1)*). That post stays first: it can't be paused or moved, and nothing can go before it. You can still cancel it.

## Saved queue

By default the posts that are not finished are saved. If you close ngPost, or the computer stops, at the next start a window shows how many posts were not finished:

- **Resume**: they are started again. A post that was being sent starts again **from the beginning**.
- **Discard**: they are removed.
- **Decide later**: they stay saved and will be offered again at the next start.

The resumed posts appear in the page **Posts**.

## Good to know

- The order, the priority and the pause of each post are saved with the queue: after a restart the posts come back as you left them. On the command line the paused posts are posted too.
- The saved queue also contains the password of each archive, in clear text, like the History. You can turn it off in **Settings**, category **History and queue**, card **Queue of the posts**: this also deletes the saved queue.
- On the command line use `--resume_queue`.
