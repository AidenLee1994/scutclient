# scutclient

**SCUT Dr.com(X) 校园网认证客户端 · SCUT Dr.com(X) campus-network authentication client written in C.**

![OpenWrt](<https://img.shields.io/badge/OpenWrt-21.xx%20~%2025.xx-blue>)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)

> 关键词 / Keywords: scutclient · drcom · SCUT · 华南理工大学 · 校园网 · 宽带认证 · campus network · broadband authentication · 802.1x · EAPOL · OpenWrt · router authentication

## 项目简介 / About

scutclient 是一个用 C 语言实现的华南理工大学校园网 DRCOM 认证客户端，通过标准 802.1X (EAPOL) 加 Dr.com 私有 UDP 心跳协议完成认证与保活。程序体积小、无第三方依赖，特别适合跑在 OpenWrt 路由器等资源受限的嵌入式设备上，实现开机自动拨号、掉线自动重连、按时段上网等无人值守场景。

scutclient is a C implementation of the DRCOM authentication client used on the South China University of Technology (SCUT) campus network. It performs standard 802.1X (EAPOL) authentication plus the Dr.com proprietary UDP heartbeat to keep the session alive. It is small, dependency-free, and well suited to resource-constrained embedded devices such as OpenWrt routers — enabling unattended dial-up on boot, automatic reconnection, and time-window internet access.

配套的 LuCI 图形配置界面见 [luci-app-scutclient-plus](https://github.com/AidenLee1994/luci-app-scutclient-plus)，它在本客户端之上提供网页化的参数填写，需与 scutclient 搭配安装。/ A LuCI web front-end for entering settings, [luci-app-scutclient-plus](https://github.com/AidenLee1994/luci-app-scutclient-plus), runs on top of this client and is installed alongside it.

## 功能列表 / Features

- **802.1X + Dr.com 认证**：完成 EAPOL Start / Identity / MD5-Challenge 全流程，并处理 Dr.com UDP 心跳保活。
- **自动重连**：掉线或心跳超时后按指数退避重新拨号（1s → 2s → … → 256s）。
- **按时段上网**：`--net-time` 指定每日恢复上网的时间点，夜间被强制下线后自动休眠等待。
- **上/下线钩子**：`--online-hook` / `--offline-hook` 可在认证成功或被强制下线时执行自定义命令（如重启 NTP）。
- **可控日志**：分级日志（ERROR/INF/DEBUG/TRACE）写入 `/tmp/scutclient.log`，超过 100KB 自动轮转为 `.backup.log`。
- **优雅退出**：收到 SIGINT/SIGTERM 时自动发送 EAPOL Logoff 下线。

## 兼容性 / Compatibility

| OpenWrt 版本 | 支持状态 |
| --- | --- |
| 21.02.x | ✅ 支持 |
| 22.03.x | ✅ 支持 |
| 23.05.x | ✅ 支持 |
| 24.10.x | ✅ 支持 |
| 25.12.x | ✅ 支持 |

说明：源码仅依赖标准 musl/内核接口（PF_PACKET 原始套接字、`SO_BINDTODEVICE`、EAPOL 等），跨上述版本通用。OpenWrt 21.02 起接口段用 `device` 取代 `ifname`，随附的 procd init 脚本已同时兼容两种写法；OpenWrt 25.12 起包管理器由 opkg (`.ipk`) 切换为 apk (`.apk`)，buildroot 会自动产出对应格式，无需改动源码。

## 编译 / Compiling

> 💡 **[luci-app-scutclient-plus](https://github.com/AidenLee1994/luci-app-scutclient-plus) 是 scutclient 的图形化配置界面**，用于在 LuCI 网页中可视化填写用户名、密码、认证服务器等校园网信息。它本身不含认证功能，**仍需先安装 scutclient 核心客户端**——推荐"先装 scutclient，再装 luci-app-scutclient-plus"，之后即可在网页中完成配置，无需手敲命令行。以下步骤面向开发者或需要手动构建的进阶用户。/ **[luci-app-scutclient-plus](https://github.com/AidenLee1994/luci-app-scutclient-plus) is a graphical configuration front-end for scutclient** that lets you enter the username, password, server and other campus-network settings from the LuCI web UI. It does not perform authentication itself and **still requires the scutclient core client to be installed first** — install scutclient first, then luci-app-scutclient-plus. The steps below are for developers or advanced users who build it themselves.

### 方式一：本地源码编译 / Option 1: Build from source locally

适合在电脑上直接生成可执行文件（需要 `git`、`cmake`、`make` 与 C 编译器）/ Produces an executable on your computer (requires `git`, `cmake`, `make` and a C compiler):

```bash
git clone https://github.com/scutclient/scutclient.git
cd scutclient
mkdir build && cd build
cmake ..
make
```

编译产物为当前目录下的可执行文件 `scutclient`。/ The build produces the `scutclient` executable in the current directory.

### 方式二：在 OpenWrt 源码树中构建软件包 / Option 2: Build a package inside the OpenWrt source tree

适合已经克隆了 OpenWrt 源码、想直接编出可安装到路由器的软件包的场景。以下步骤假设你的 OpenWrt 源码位于 `~/openwrt`，请按实际路径替换。/ For users who already have the OpenWrt source tree and want an installable package for their router. The steps below assume the OpenWrt source is at `~/openwrt`; adjust the path to your setup.

**1. 准备 OpenWrt 源码 / Prepare the OpenWrt source** — 无需额外操作，只要已 clone 并能正常编译即可（首次使用请先执行一次 `./scripts/feeds update -a && ./scripts/feeds install -a`）。/ Nothing special to do — just make sure the tree is cloned and buildable (on first use run `./scripts/feeds update -a && ./scripts/feeds install -a`).

**2. 放入软件包定义 / Add the package definition** — 在 OpenWrt 的 `package` 目录下新建 `scutclient` 目录，并把本仓库的 `openwrt/Makefile` 复制进去（用图形文件管理器操作也可以 :D）：/ Create a `scutclient` directory under OpenWrt's `package` directory and copy this repo's `openwrt/Makefile` into it (a GUI file manager works too :D):

```bash
cd ~/openwrt
mkdir -p package/scutclient
cp /path/to/scutclient/openwrt/Makefile package/scutclient/
```

这样就得到了一个直接拉取主线最新代码编译的软件包（Makefile 会自动从 GitHub 克隆默认分支的最新源码）。/ This gives you a package that builds straight from the latest mainline (the Makefile clones the newest source on the default branch from GitHub automatically).

> 想编译本地改动过的源码，而不是 GitHub 上的稳定版？编辑 `package/scutclient/Makefile`，把 `SRCDIR:=` 一行改成你的源码绝对路径即可，例如 `SRCDIR:=/path/to/scutclient`。/ To build your own modified source instead of the released one, edit `package/scutclient/Makefile` and set `SRCDIR:=` to the absolute path of your source, e.g. `SRCDIR:=/path/to/scutclient`.

**3. 在 menuconfig 中勾选 / Select it in menuconfig**：

```bash
make menuconfig
```

依次进入 `Network` 菜单，找到 `scutclient`，按空格键将其标记为 `M`（编译为独立软件包）或 `*`（直接编入固件），保存退出。/ Go to the `Network` menu, find `scutclient`, press Space to mark it `M` (build as a standalone package) or `*` (build into the firmware image), then save and exit.

**4. 编译 / Compile**：

```bash
make package/scutclient/compile V=s
```

`V=s` 用于打印详细日志，便于排查问题；首次编译若提示缺少工具链，请先 `make defconfig` 或 `make tools/install` 完成基础准备。/ `V=s` prints verbose logs for troubleshooting; on a first build, if the toolchain is missing, run `make defconfig` or `make tools/install` first.

**5. 取出并安装软件包 / Collect and install the package** — 编译产物位于 `bin/packages/<架构>/base/` 下（OpenWrt ≤24.10 为 `scutclient_*.ipk`，25.12+ 为 `scutclient_*.apk`）。把它拷到路由器后安装：/ The result is under `bin/packages/<arch>/base/` (`scutclient_*.ipk` on OpenWrt ≤24.10, `scutclient_*.apk` on 25.12+). Copy it to the router and install:

```bash
# OpenWrt ≤ 24.10
opkg install scutclient_*.ipk
# OpenWrt ≥ 25.12
apk add --allow-untrusted scutclient_*.apk
```

安装后即可通过 `/etc/config/scutclient` 配置并由 `/etc/init.d/scutclient` 启动服务。若想在网页中可视化填写认证信息，可在此基础上再安装 [luci-app-scutclient-plus](https://github.com/AidenLee1994/luci-app-scutclient-plus)。/ After installation, configure via `/etc/config/scutclient` and start it through `/etc/init.d/scutclient`. For a web UI to enter the settings, install [luci-app-scutclient-plus](https://github.com/AidenLee1994/luci-app-scutclient-plus) on top of this.

## 用法 / Usage

```bash
scutclient --username <username> --password <password> [options...]
 -i, --iface <ifname>        执行认证的网卡 / Interface to perform authentication.
 -n, --dns <dns>             上报给 UDP 服务器的 DNS / DNS server address sent to UDP server.
 -H, --hostname <hostname>   上报的主机名 / Hostname reported to the server.
 -s, --udp-server <server>   认证服务器 IP / UDP authentication server IP.
 -c, --cli-version <ver>     Dr.com 客户端版本 / Client version.
 -T, --net-time <time>       允许上网的起始时间，如 6:10 / Time you are allowed online, e.g. 6:10.
 -h, --hash <hash>           DrAuthSvr.dll 哈希值 / DrAuthSvr.dll hash value.
 -E, --online-hook <cmd>     认证成功后执行的命令 / Command run after EAP auth success.
 -Q, --offline-hook <cmd>    夜间被强制下线时执行的命令 / Command run when forced offline at night.
 -D, --debug                 开启调试日志 / Enable debug logging.
 -o, --logoff                下线并退出 / Log off and exit.
```

## 配置说明 / Configuration

| 参数 / Parameter | 说明 / Description |
| --- | --- |
| 用户名 / username | 学号或学校分配的宽带账号（< 32 字符）/ Student ID or assigned broadband account (< 32 chars). |
| 密码 / password | 宽带认证密码（< 100 字符）/ Broadband password (< 100 chars). |
| Drcom 版本 / cli-version | 按所在校区与接入点选择，默认值适用于大多数场景。 |
| 哈希 / hash | 与 Drcom 版本对应的 DrAuthSvr.dll 哈希，默认值适用于大多数场景。 |
| 服务器 IP / udp-server | 认证服务器地址，默认 `202.38.210.131`。 |
| 允许上网时间 / net-time | 断网后恢复上网的起始时间，格式 `H:MM`，如 `6:15`。 |
| 主机名 / hostname | 上报给服务器的设备名，缺省取系统 hostname。 |

在 OpenWrt 上，以上参数通过 `/etc/config/scutclient`（UCI）配置，由 `/etc/init.d/scutclient` 转换为命令行参数启动服务。

## 安全声明 / Security Notice

- 本客户端面向华南理工大学校园网**合法授权用户**，不绕过、不破解校园网认证系统。
- 认证凭据在 OpenWrt 上以明文存储于 `/etc/config/scutclient`，建议限制路由器管理界面的访问来源并定期修改管理密码。
- 命令行方式运行时，密码会出现在进程参数中（`ps` 可见），请在多用户环境下注意。
- 日志文件 `/tmp/scutclient.log` 可能包含用户名、MAC、IP 等信息，反馈问题前请自行脱敏。
- 禁止将本客户端用于任何违反华南理工大学网络使用规定或中华人民共和国相关法律法规的用途。

## 故障排查 / Troubleshooting

认证失败时，优先检查：

1. Drcom 版本与哈希值是否与所在校区一致——不同接入区域可能使用不同版本。
2. 主机名是否符合学校要求（部分接入点会校验主机名格式）。
3. 认证服务器是否可达：`ping 202.38.210.131`。
4. 用 `-D` 开启调试日志，或查看 `/tmp/scutclient.log` 中的实时输出。

## 许可证 / License

[GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.html)

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)

We believe that you know what you are doing. You should get this software for free.

## 致谢 / Acknowledgements

- 核心 DRCOM 认证客户端 / Core DRCOM client — scutclient
- OpenWrt 与 LuCI 框架 / The OpenWrt and LuCI frameworks
- 华南理工大学计算机科学与工程学院

技术支持 / Technical support: **Dr. Lee**
