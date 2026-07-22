# ClipboardHistory

基于 Qt6 的桌面剪切板历史管理工具，自动记录文本和图片，支持搜索、收藏、预览。

## 功能

- **自动记录** — 监听系统剪切板，文本和图片自动存入 SQLite，MD5 去重
- **搜索过滤** — 顶部搜索栏实时过滤历史记录
- **图片预览** — 选中条目右侧预览完整内容，图片自适应缩放
- **收藏管理** — 右键菜单收藏/取消收藏，收藏条目自动置顶
- **全局快捷键** — `Ctrl+Alt+V` 唤出窗口
- **快捷键导航** — `↑/↓` 或 `Ctrl+P/Ctrl+N` 切换条目，`Enter` 复制，`Esc` 关闭

## 环境要求

- CMake >= 3.16
- C++17 编译器 (g++ >= 11, clang >= 14)
- Qt6 (Widgets, Sql, Gui)

### Ubuntu 24.04 安装依赖

```bash
sudo apt install cmake g++ qt6-base-dev libqt6sql6-sqlite
```

## 构建

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## 运行

```bash
./build/ClipboardHistory
```

## 操作说明

| 操作 | 功能 |
|------|------|
| `Ctrl+Alt+V` | 显示/隐藏窗口 |
| `Ctrl+F` | 聚焦搜索框 |
| `↑/↓` 或 `Ctrl+P/Ctrl+N` | 在列表中切换选择 |
| `Enter` | 复制选中内容到剪切板 |
| `Esc` | 隐藏窗口 |
| 鼠标单击 | 预览内容 |
| 鼠标双击 | 复制并关闭（取决于设置） |
| 鼠标右键 | 弹出菜单：复制/收藏/删除 |

## 设置

托盘右键菜单（需桌面环境支持系统托盘）：
- **复制后自动关闭** — 切换复制后是否自动隐藏窗口

## 项目结构

```
Clipboard/
├── CMakeLists.txt               # CMake 构建配置
├── resources/
│   └── resources.qrc            # Qt 资源文件
├── src/
│   ├── main.cpp                 # 入口
│   ├── core/
│   │   ├── ClipboardItem.h      # 数据模型 (Text/Image)
│   │   ├── DatabaseManager.h/cpp # SQLite 持久化
│   │   └── ClipboardMonitor.h/cpp # 剪切板监听
│   ├── ui/
│   │   ├── MainWindow.h/cpp     # 主窗口
│   │   ├── HistoryListModel.h/cpp # 列表模型
│   │   ├── HistoryListDelegate.h/cpp # 自定义渲染
│   │   └── ImagePreviewWidget.h/cpp # 图片/文本预览
│   └── utils/
│       └── HotkeyManager.h/cpp  # 全局快捷键
└── build/
    ├── ClipboardHistory         # 可执行文件
    └── clipboard_history.db     # 数据库（自动生成）
```

## 数据存储

SQLite 数据库文件 `clipboard_history.db` 生成在可执行文件同目录下。表结构：

| 字段 | 类型 | 说明 |
|------|------|------|
| id | INTEGER | 自增主键 |
| type | INTEGER | 0=文本, 1=图片 |
| content_text | TEXT | 文本内容 |
| content_html | TEXT | HTML 格式文本 |
| image_data | BLOB | 图片 PNG 数据 |
| content_hash | TEXT | MD5 哈希（去重） |
| is_favorite | INTEGER | 是否收藏 |
| created_at | DATETIME | 记录时间 |

## License

MIT
