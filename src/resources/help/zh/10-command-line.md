# 命令行

程序也可以不带窗口运行，用于服务器、NAS 或脚本。命令行版本就是不带窗口编译的同一个程序（或 Docker 镜像）。

## 示例

发布两个项目，使用压缩、随机名称和密码，以及 par2：

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

发布一个文件夹中的每个项目，每个项目一次发布：

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

监视一个文件夹，并在发布后把文件移动到 “Posted” 文件夹：

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --move_posted Posted

`--move_posted` 接受 `Posted` 这样的名称（该文件夹会创建在每个文件或文件夹旁边）或完整路径。若要在发布后删除文件，请改用 `--rm_posted`。两者都只在全部发布无错误时才生效。

## 常用选项

- `-c` 配置文件（否则为 `$HOME/.ngPost`）
- `-g` 新闻组，以逗号分隔
- `-o` NZB 输出文件
- `-S` 以单个字符串表示的服务器
- `--disp_progress` BAR 或 FILES
- `--resume_queue` 重新发布未完成的发布
- `--allow_repost` 重新发布已在历史记录中的文件（默认会跳过它们）
- `--max_speed MB/s` 限制所有连接合计的上传速度（0：不限制）；相当于配置文件中的 `limit_speed` 和 `max_speed` 键
- `--gen_nfo` 在 nzb 旁边写入 NFO 文件；`--nfo_in_post` 同时发布它；`--nfo_password` 在其中写入压缩包密码；`--nfo_text file.txt` 在末尾添加文字；`--no_nfo_media` 省略媒体信息（MediaInfo）；`--mediainfo_path 路径` 选择 MediaInfo 程序；`--nfo_style 名称` 选择外观（classic、clean、box、scene、elegant、retro）；`--nfo_ascii` 仅 ASCII 字符
- `--notify_discord URL`、`--notify_telegram_token T` 配合 `--notify_telegram_chat ID`、`--notify_webhook URL` 和 `--notify_when each|all_done`：发布结束时的通知（参见[进度与日志](06-progress-and-log.md)）
- `--export_settings 文件` 将设置、配置文件、通知和服务器（绝不含密码）写入文件；`--import_settings 文件` 将此类文件添加到配置中（旧配置保存为 `.before-import`）；两者都在不发布的情况下退出
- `--help` 完整列表

参见[自动发布](08-auto-posting.md)。
