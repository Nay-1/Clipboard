## ClipboardHistory 项目规范

### 项目概述
Qt6 C++17 桌面剪切板历史管理工具，自动记录文本/图片，支持搜索、收藏、预览。

### 构建命令
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 运行
```bash
./build/ClipboardHistory
```

### 代码风格
- 使用 `QStringLiteral` / `QLatin1String` 而非裸字符串
- 头文件使用 `#pragma once`
- 成员变量前缀 `m_`，静态变量前缀 `s_`
- Qt 信号/槽使用新式语法 `connect(sender, &Class::signal, this, &Class::slot)`
- 禁止在代码中添加注释

### 文件结构
- `src/core/` — 核心逻辑（数据模型、数据库、剪切板监听）
- `src/ui/` — 界面组件（主窗口、列表模型、委托渲染、预览控件）
- `src/utils/` — 工具类（全局快捷键）
- `resources/` — Qt 资源文件

### 数据层约定
- 数据库使用 SQLite，通过 Qt SQL 模块访问
- `ClipboardItem` 为纯数据结构（POD），定义在 `ClipboardItem.h`
- `DatabaseManager` 封装所有 CRUD 操作，包括 `cleanupOldItems()` 清理过期记录
- `ClipboardMonitor` 通过信号 `newItem` 通知上层
- 自动清理规则：非收藏记录超过 24 小时删除，取消收藏时 `created_at` 重置为当前时间重新计时
- 每小时通过 `QTimer` 执行一次 `cleanupOldItems()`

### 命名规范
- 源文件: `PascalCase`（如 `MainWindow.cpp`）
- 类名: `PascalCase`
- 函数名: `camelCase`
- 枚举值: `PascalCase`（如 `ClipboardItem::Text`）
