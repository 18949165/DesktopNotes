# StickyNotes

一个轻量级的 Windows 桌面便签应用，基于 Qt 6 和 ElaWidgetTools。

## 功能

- **便签管理** — 创建、编辑、删除便签，支持独立标题和正文
- **废纸篓** — 软删除，可恢复或永久删除
- **重要标记** — 标记便签为重要以便优先展示
- **全局热键** — 可自定义热键，快速显示/隐藏主窗口
- **系统托盘** — 最小化到托盘，支持快捷操作
- **贴窗口** — 切换为紧凑浮动便签视图
- **设置** — 偏好设置持久化（热键等）

## 构建

### 前置依赖

- CMake >= 3.21
- Qt 6.7.3（Core、Gui、Widgets）
- MSVC 2022（或兼容 C++17 编译器）

### 快速开始

```bash
cmake -B build -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH=C:/Qt/6.7.3/msvc2022_64
cmake --build build --config Release
```

可执行文件位于 `build/Release/StickyNotes.exe`。

### 运行测试

```bash
cmake --build build --config Release --target runtests
# 或直接:
ctest --test-dir build -C Release
```

### 打包

```bash
cmake --build build --config Release --target package
```

在 `build/` 下生成 NSIS 安装包。

## 架构

```
src/
├── platform/   — 操作系统抽象层（文件系统、时钟、热键、托盘）+ Win32 实现
├── core/       — 领域逻辑（便签模型、设置、数据存储）
├── ui/         — Qt 界面组件（主窗口、编辑器、设置对话框、贴窗口）
├── app/        — 组合根（AppContext、Runtime 组装）
tests/          — GoogleTest 单元测试
```

## 许可

MIT
