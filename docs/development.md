# 开发与运行

## 环境

- Windows 64 位；当前预编译驱动：Qt 6.11.1、MinGW GCC 13.1。
- Qt 模块：Core、Gui、Widgets、Sql、Charts。
- MySQL 8.0.16 或以上，推荐 MySQL Server 8.x；Python 3.10 或以上。

先按 README 从 Release 获取“运行程序”目录，放在仓库根目录。不同 Qt 或编译器需要重新准备对应运行库，不能混用随包驱动。

## 构建

可直接在 Qt Creator 打开 `.pro` 或 CMake 工程。使用 qmake 时配置变化后先运行 qmake，再构建。

在已经配置 Qt/MinGW 环境的终端中：

```powershell
mkdir build
cd build
qmake ../ScuecMarket.pro CONFIG+=debug CONFIG-=release
mingw32-make -j4
```

默认输出为 `build/debug/CampusMarket.exe`，Release 模式对应 `build/release`。自动部署会补齐 QMYSQL、libmysql、Qt/SSL/VC 依赖和样例图片。首次使用可以双击输出目录内的 `运行演示.cmd`。

CMake 可用：

```powershell
cmake -S . -B build-cmake -G Ninja -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/mingw_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake --parallel 4
```

其中 Qt 路径替换为自己的安装位置。程序读取 exe 同目录的 `database.ini`。

## 连接自己的 MySQL，使用 Qt Creator 调试

从仓库根目录执行：

```powershell
python -m pip install -r requirements.txt
python tools/init_database.py --port 3306 --user root --database campus_market_dev --config-dir build/debug
```

按提示输入数据库管理员密码。工具仅初始化空数据库，创建项目专用账号，并写入程序旁的配置。自有 MySQL 服务保持运行后，可直接在 Qt Creator 点运行或设置断点。

已有数据库请沿用自己的配置；重新构建不覆盖自定义连接信息。缺失配置或仍含“请填写实验数据库密码”的样例配置，可以从本机已初始化的演示实例记录补齐。第一次在新电脑运行时，仍需初始化。

## 常见情况

| 提示 | 检查 |
|---|---|
| Driver not loaded | 保留仓库根目录的运行程序文件夹，重新执行 qmake 并构建，检查套件版本 |
| Access denied | 账号/密码尚未配置或不匹配；运行演示启动器，或初始化自有数据库 |
| 连接被拒绝 | 数据库未启动，或配置中的端口不正确 |
| 演示端口占用 | 启动器只复用凭据和数据目录匹配的本项目实例，不接管其他服务 |

独立演示实例通常在窗口关闭后停止；有其他窗口仍在使用时会保留。直接点 Qt Creator 运行不等于启动独立数据库。

## 数据与测试

`database/schema.sql` 定义 8 张业务表和约束；`seed.sql` 为基础账号和商品，`demo.sql` 为历史成交与举报，`demo_products.sql` 补充六件带网络配图的商品。`reports.sql` 提供五个报表。

集成验证会改变样例数据，请使用全新专用数据库：

```powershell
python tools/init_database.py --port 3306 --user root --database campus_test --basic-test-data --config-dir build-cmake
cmake --build build-cmake --target CampusIntegration --parallel 4
./build-cmake/CampusIntegration.exe
```

`tests/driver_probe.pro` 可单独验证驱动加载；`tests/ui_preview.cpp` 是开发用页面预览入口。实际数据库配置、构建缓存和预编译运行库不提交 Git。
