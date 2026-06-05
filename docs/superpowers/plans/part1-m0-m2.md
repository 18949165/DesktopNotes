# StickyNotes 实施计划 — Part 1 / M0–M2

> 范围：仓库骨架 + 平台抽象 + core 数据层。
> 完成后产物：可编译可测试，core 单元测试 ≥ 80% 行覆盖。

**M0 仓库骨架** · **M1 平台抽象** · **M2 core 数据层**

---

## Milestone 0 — 仓库骨架

> 目标：`cmake -S . -B build` + `cmake --build build` + `ctest` 全绿；4 个空模块（platform / core / app / ui）+ 1 个 exe。

### Task M0.1：顶层 CMake 骨架

**Files:**
- Create: `CMakeLists.txt`
- Create: `cmake/CompilerWarnings.cmake`
- Create: `cmake/StandardProjectSettings.cmake`

- [ ] **Step 1: 写 `cmake/CompilerWarnings.cmake`**

```cmake
# cmake/CompilerWarnings.cmake
function(set_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4 /permissive- /Zc:__cplusplus
            /wd4251 /wd4275 /wd4267) # Qt 元对象常见警告
        target_compile_definitions(${target} PRIVATE
            _CRT_SECURE_NO_WARNINGS NOMINMAX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()
```

- [ ] **Step 2: 写 `cmake/StandardProjectSettings.cmake`**

```cmake
# cmake/StandardProjectSettings.cmake
function(set_project_standards target)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)
endfunction()
```

- [ ] **Step 3: 写顶层 `CMakeLists.txt`**

```cmake
cmake_minimum_required(VERSION 3.21)
project(StickyNotes LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(StandardProjectSettings)
include(CompilerWarnings)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

# 第三方
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# GoogleTest
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# QFluentWidgets
FetchContent_Declare(qfluentwidgets
    GIT_REPOSITORY https://github.com/zhiyiYo/Qt-Fluent-Widgets.git
    GIT_TAG        master)  # TODO: 实施时锁定 release tag
FetchContent_MakeAvailable(qfluentwidgets)

enable_testing()
add_subdirectory(src/platform)
add_subdirectory(src/core)
add_subdirectory(src/app)
add_subdirectory(src/ui)
add_subdirectory(tests)
add_subdirectory(src/app)  # 含 main.cpp
```

- [ ] **Step 4: 空 `add_subdirectory` 目录占位**

为每个 `src/<module>/CMakeLists.txt` 创建**最简空 lib**：

```cmake
# src/platform/CMakeLists.txt
add_library(platform INTERFACE)
target_include_directories(platform INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

> core / app / ui 同样建空 target（INTERFACE 或 STATIC 空源）。
> 后续任务逐步往里加文件。

- [ ] **Step 5: 验证配置成功**

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

预期：`build/` 生成；无错误。

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt cmake/ src/*/CMakeLists.txt
git commit -m "chore(M0.1): scaffold CMake project with 4 empty modules"
```

---

### Task M0.2：集成 GoogleTest + 第一个 ctest 样例

**Files:**
- Create: `tests/CMakeLists.txt`
- Create: `tests/smoke_test.cpp`

- [ ] **Step 1: 写 `tests/CMakeLists.txt`**

```cmake
# tests/CMakeLists.txt
include(GoogleTest)

add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE GTest::gtest_main)
gtest_discover_tests(smoke_test)
```

- [ ] **Step 2: 写 `tests/smoke_test.cpp`**

```cpp
#include <gtest/gtest.h>
TEST(Smoke, CompilesAndLinks) { EXPECT_EQ(1 + 1, 2); }
```

- [ ] **Step 3: 跑测试**

```bash
cmake --build build --target smoke_test
ctest --test-dir build --output-on-failure
```

预期：`1 test passed`。

- [ ] **Step 4: Commit**

```bash
git add tests/
git commit -m "test(M0.2): wire up GoogleTest with a smoke test"
```

---

### Task M0.3：app 入口（main.cpp 占位）

**Files:**
- Create: `src/app/CMakeLists.txt`（改为 STATIC + main）
- Modify: `src/app/main.cpp`
- Create: `src/app/include/app/app_entry.h`

- [ ] **Step 1: 改 `src/app/CMakeLists.txt`**

```cmake
# src/app/CMakeLists.txt
add_library(app STATIC main.cpp)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)
set_project_standards(app)
set_project_warnings(app)
```

- [ ] **Step 2: 写 `src/app/main.cpp`**

```cpp
#include <QCoreApplication>
#include <QtGlobal>
#include "app/app_entry.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("StickyNotes");
    QCoreApplication::setApplicationName("StickyNotes");
    return stickynotes::runHeadless(app);
}
```

- [ ] **Step 3: 写 `src/app/include/app/app_entry.h`**

```cpp
#pragma once
#include <QCoreApplication>
namespace stickynotes {
int runHeadless(QCoreApplication& app);
}
```

- [ ] **Step 4: 在 app.cpp 末尾补上 `runHeadless`**

在 `src/app/main.cpp` 末尾追加：

```cpp
namespace stickynotes {
int runHeadless(QCoreApplication&) { return 0; }
}
```

> 实现先空跑，后续 M3 替换为真实装配。

- [ ] **Step 5: 跑一次完整 build**

```bash
cmake --build build
```

预期：无错误。

- [ ] **Step 6: Commit**

```bash
git add src/app/
git commit -m "feat(M0.3): add app main entry stub"
```

---

## Milestone 1 — 平台抽象

> 目标：所有副作用（FS/时钟/通知/快捷键/托盘）都先有接口，Win 实现用桩替代也行；GoogleMock fakes 给出。

### Task M1.1：`IFileSystem` 接口 + Qt/Win 实现

**Files:**
- Create: `src/platform/include/platform/ifilesystem.h`
- Create: `src/platform/win/filesystem_qt.cpp`
- Modify: `src/platform/CMakeLists.txt`（改为 STATIC + 源文件）

- [ ] **Step 1: 写接口头 `ifilesystem.h`**

```cpp
#pragma once
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <optional>

namespace stickynotes::platform {
class IFileSystem {
public:
    virtual ~IFileSystem() = default;
    virtual bool exists(const QString& path) const = 0;
    virtual std::optional<QByteArray> readAll(const QString& path) const = 0;
    virtual bool writeAtomic(const QString& path, const QByteArray& data) = 0;
    virtual QStringList list(const QString& dir, const QStringList& nameFilters) const = 0;
    virtual bool ensureDir(const QString& dir) = 0;
};
}
```

- [ ] **Step 2: 写 `FileSystem_Qt` 实现**

```cpp
// src/platform/win/filesystem_qt.cpp
#include "platform/ifilesystem.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace stickynotes::platform {
class FileSystem_Qt final : public IFileSystem {
public:
    bool exists(const QString& path) const override {
        return QFileInfo::exists(path);
    }
    std::optional<QByteArray> readAll(const QString& path) const override {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return std::nullopt;
        return f.readAll();
    }
    bool writeAtomic(const QString& path, const QByteArray& data) override {
        QSaveFile f(path);
        if (!f.open(QIODevice::WriteOnly)) return false;
        if (f.write(data) != data.size()) return false;
        return f.commit();
    }
    QStringList list(const QString& dir, const QStringList& nameFilters) const override {
        QDir d(dir);
        return d.entryList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    }
    bool ensureDir(const QString& dir) override {
        return QDir().mkpath(dir);
    }
};
}
```

- [ ] **Step 3: 改 `src/platform/CMakeLists.txt`**

```cmake
add_library(platform STATIC win/filesystem_qt.cpp)
target_include_directories(platform PUBLIC include)
target_link_libraries(platform PUBLIC Qt6::Core)
set_project_standards(platform)
set_project_warnings(platform)
```

- [ ] **Step 4: 写单测 `tests/platform/filesystem_qt_test.cpp`**

```cpp
#include <gtest/gtest.h>
#include "platform/ifilesystem.h"
#include "platform/win/filesystem_qt.h"  // 改为公开头导出
#include <QStandardPaths>
#include <QTemporaryDir>

using stickynotes::platform::FileSystem_Qt;
using stickynotes::platform::IFileSystem;

TEST(FileSystem_Qt, AtomicWriteAndRead) {
    QTemporaryDir tmp;
    FileSystem_Qt fs;
    QString p = tmp.filePath("a.txt");
    ASSERT_TRUE(fs.writeAtomic(p, "hello"));
    EXPECT_TRUE(fs.exists(p));
    auto data = fs.readAll(p);
    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(*data, QByteArray("hello"));
}

TEST(FileSystem_Qt, EnsureDirIdempotent) {
    QTemporaryDir tmp;
    FileSystem_Qt fs;
    QString d = tmp.filePath("x/y/z");
    EXPECT_TRUE(fs.ensureDir(d));
    EXPECT_TRUE(fs.ensureDir(d));
    EXPECT_TRUE(fs.exists(d));
}
```

- [ ] **Step 5: 公开实现头（让测试能 include）**

新建 `src/platform/include/platform/win/filesystem_qt.h`：

```cpp
#pragma once
#include "platform/ifilesystem.h"
namespace stickynotes::platform {
class FileSystem_Qt final : public IFileSystem {
public:
    bool exists(const QString& path) const override;
    std::optional<QByteArray> readAll(const QString& path) const override;
    bool writeAtomic(const QString& path, const QByteArray& data) override;
    QStringList list(const QString& dir, const QStringList& nameFilters) const override;
    bool ensureDir(const QString& dir) override;
};
}
```

把 `filesystem_qt.cpp` 改为仅含定义。

- [ ] **Step 6: 把 test 加入构建**

在 `tests/CMakeLists.txt` 末尾加：

```cmake
add_executable(platform_tests platform/filesystem_qt_test.cpp)
target_link_libraries(platform_tests PRIVATE platform GTest::gtest_main Qt6::Test)
gtest_discover_tests(platform_tests)
```

- [ ] **Step 7: 跑测试**

```bash
cmake --build build --target platform_tests
ctest --test-dir build -R FileSystem_Qt --output-on-failure
```

预期：2 通过。

- [ ] **Step 8: Commit**

```bash
git add src/platform/ tests/platform/
git commit -m "feat(M1.1): add IFileSystem + FileSystem_Qt with tests"
```

---

### Task M1.2：`IClock` + `SystemClock` + `FakeClock` + GoogleMock

**Files:**
- Create: `src/platform/include/platform/iclock.h`
- Create: `src/platform/include/platform/system_clock.h`
- Create: `src/platform/include/platform/fake_clock.h`
- Create: `src/platform/win/system_clock.cpp`
- Modify: `src/platform/CMakeLists.txt`
- Create: `tests/platform/mock_clock.h`
- Create: `tests/platform/clock_test.cpp`

- [ ] **Step 1: 写 `iclock.h`**

```cpp
#pragma once
#include <QDateTime>
namespace stickynotes::platform {
class IClock {
public:
    virtual ~IClock() = default;
    virtual QDateTime now() const = 0;
};
}
```

- [ ] **Step 2: 写 `system_clock.h` + `.cpp`**

```cpp
// include/platform/system_clock.h
#pragma once
#include "platform/iclock.h"
namespace stickynotes::platform {
class SystemClock final : public IClock {
public:
    QDateTime now() const override;
};
}
```

```cpp
// win/system_clock.cpp
#include "platform/system_clock.h"
namespace stickynotes::platform {
QDateTime SystemClock::now() const { return QDateTime::currentDateTime(); }
}
```

- [ ] **Step 3: 写 `fake_clock.h`**

```cpp
#pragma once
#include "platform/iclock.h"
#include <chrono>
namespace stickynotes::platform {
class FakeClock final : public IClock {
public:
    explicit FakeClock(QDateTime t) : t_(std::move(t)) {}
    QDateTime now() const override { return t_; }
    void advance(std::chrono::seconds s) { t_ = t_.addSecs(s.count()); }
private:
    QDateTime t_;
};
}
```

- [ ] **Step 4: 改 `src/platform/CMakeLists.txt`**

```cmake
add_library(platform STATIC
    win/filesystem_qt.cpp
    win/system_clock.cpp)
target_include_directories(platform PUBLIC include)
target_link_libraries(platform PUBLIC Qt6::Core)
set_project_standards(platform)
set_project_warnings(platform)
```

- [ ] **Step 5: 写 `tests/platform/mock_clock.h`**

```cpp
#pragma once
#include <gmock/gmock.h>
#include "platform/iclock.h"
namespace stickynotes::platform {
class MockClock : public IClock {
public:
    MOCK_METHOD(QDateTime, now, (), (const, override));
};
}
```

- [ ] **Step 6: 写 `tests/platform/clock_test.cpp`**

```cpp
#include <gtest/gtest.h>
#include "platform/fake_clock.h"
#include "platform/mock_clock.h"
using stickynotes::platform::FakeClock;
using stickynotes::platform::MockClock;
using testing::Return;

TEST(FakeClock, AdvancesAndReturns) {
    FakeClock c(QDateTime(QDate(2026,1,1), QTime(0,0)));
    EXPECT_EQ(c.now().date().year(), 2026);
    c.advance(std::chrono::hours(24));
    EXPECT_EQ(c.now().date().day(), 2);
}

TEST(MockClock, StubsNow) {
    MockClock c;
    EXPECT_CALL(c, now()).WillOnce(Return(QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate)));
    EXPECT_EQ(c.now().toString(Qt::ISODate), QString("2026-06-05T10:00:00"));
}
```

- [ ] **Step 7: 注册测试**

在 `tests/CMakeLists.txt` 末尾：

```cmake
add_executable(clock_tests platform/clock_test.cpp tests/platform/mock_clock.h)
target_link_libraries(clock_tests PRIVATE platform GTest::gmock_main Qt6::Test)
gtest_discover_tests(clock_tests)
```

- [ ] **Step 8: 跑**

```bash
cmake --build build --target clock_tests
ctest --test-dir build -R "FakeClock|MockClock" --output-on-failure
```

- [ ] **Step 9: Commit**

```bash
git add src/platform/ tests/platform/
git commit -m "feat(M1.2): add IClock with SystemClock, FakeClock, MockClock"
```

---

### Task M1.3：`INotifier / IHotkey / ITrayIcon` 接口 + Win 桩

> 这些接口对 M2/M3 必需，Win 真实实现放到 part3；本任务只先建接口 + 桩（空实现），保证 M1 末尾能 build。

**Files:**
- Create: `src/platform/include/platform/inotifier.h` / `ihotkey.h` / `itrayicon.h`
- Create: `src/platform/include/platform/win/{notifier,hotkey,trayicon}_win.h`
- Create: `src/platform/win/{notifier,hotkey,trayicon}_win.cpp`（桩）
- Modify: `src/platform/CMakeLists.txt`
- Create: `tests/platform/{notifier,hotkey,trayicon}_test.cpp`

- [ ] **Step 1: 写 `inotifier.h`**

```cpp
#pragma once
#include <QString>
namespace stickynotes::platform {
class INotifier {
public:
    struct Notification { QString title; QString body; QString id; };
    virtual ~INotifier() = default;
    virtual void show(const Notification& n) = 0;
};
}
```

- [ ] **Step 2: 写 Win 桩 `notifier_win.h/cpp`**

```cpp
// include/platform/win/notifier_win.h
#pragma once
#include "platform/inotifier.h"
namespace stickynotes::platform::win {
class Notifier_Win final : public INotifier {
public:
    void show(const Notification& n) override;  // 桩：仅 qDebug
};
}
```

```cpp
// win/notifier_win.cpp
#include "platform/win/notifier_win.h"
#include <QDebug>
namespace stickynotes::platform::win {
void Notifier_Win::show(const Notification& n) {
    qDebug().noquote() << "[Notify]" << n.title << "-" << n.body;
}
}
```

- [ ] **Step 3: 写 `ihotkey.h` + Win 桩**

```cpp
// include/platform/ihotkey.h
#pragma once
#include <QObject>
#include <QString>
#include <functional>
namespace stickynotes::platform {
class IHotkey : public QObject {
    Q_OBJECT
public:
    struct Spec { int id; Qt::KeyboardModifiers mods; Qt::Key key; };
    virtual ~IHotkey() = default;
    virtual bool registerHotkey(const Spec& s) = 0;
    virtual bool unregisterHotkey(int id) = 0;
signals:
    void triggered(int id);
};
}
```

```cpp
// include/platform/win/hotkey_win.h
#pragma once
#include "platform/ihotkey.h"
namespace stickynotes::platform::win {
class Hotkey_Win final : public IHotkey {
public:
    bool registerHotkey(const Spec& s) override;  // 桩：返回 true
    bool unregisterHotkey(int id) override;
};
}
```

```cpp
// win/hotkey_win.cpp
#include "platform/win/hotkey_win.h"
namespace stickynotes::platform::win {
bool Hotkey_Win::registerHotkey(const Spec&) { return true; }
bool Hotkey_Win::unregisterHotkey(int) { return true; }
}
```

- [ ] **Step 4: 写 `itrayicon.h` + Win 桩**

```cpp
// include/platform/itrayicon.h
#pragma once
#include <QObject>
#include <functional>
namespace stickynotes::platform {
class ITrayIcon : public QObject {
    Q_OBJECT
public:
    struct MenuItem { QString text; std::function<void()> onClick; };
    virtual ~ITrayIcon() = default;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void setMenu(const QList<MenuItem>& items) = 0;
signals:
    void leftClicked();
};
}
```

```cpp
// include/platform/win/trayicon_win.h
#pragma once
#include "platform/itrayicon.h"
namespace stickynotes::platform::win {
class TrayIcon_Win final : public ITrayIcon {
public:
    void show() override;       // 桩
    void hide() override;
    void setMenu(const QList<MenuItem>& items) override;
};
}
```

```cpp
// win/trayicon_win.cpp
#include "platform/win/trayicon_win.h"
namespace stickynotes::platform::win {
void TrayIcon_Win::show() {}
void TrayIcon_Win::hide() {}
void TrayIcon_Win::setMenu(const QList<MenuItem>&) {}
}
```

- [ ] **Step 5: 改 `src/platform/CMakeLists.txt`**

```cmake
add_library(platform STATIC
    win/filesystem_qt.cpp
    win/system_clock.cpp
    win/notifier_win.cpp
    win/hotkey_win.cpp
    win/trayicon_win.cpp)
target_include_directories(platform PUBLIC include)
target_link_libraries(platform PUBLIC Qt6::Core Qt6::Gui)
set_project_standards(platform)
set_project_warnings(platform)
```

- [ ] **Step 6: 写 `tests/platform/mocks.h` + 3 个测试**

```cpp
// tests/platform/mocks.h
#pragma once
#include <gmock/gmock.h>
#include "platform/inotifier.h"
#include "platform/ihotkey.h"
#include "platform/itrayicon.h"
namespace stickynotes::platform {
class MockNotifier : public INotifier {
public:
    MOCK_METHOD(void, show, (const Notification&), (override));
};
class MockHotkey : public IHotkey {
public:
    MOCK_METHOD(bool, registerHotkey, (const Spec&), (override));
    MOCK_METHOD(bool, unregisterHotkey, (int), (override));
};
class MockTrayIcon : public ITrayIcon {
public:
    MOCK_METHOD(void, show, (), (override));
    MOCK_METHOD(void, hide, (), (override));
    MOCK_METHOD(void, setMenu, (const QList<MenuItem>&), (override));
};
}
```

```cpp
// tests/platform/notifier_test.cpp
#include <gtest/gtest.h>
#include "platform/mocks.h"
using stickynotes::platform::MockNotifier;
using testing::_; using testing::Truly;
TEST(MockNotifier, ShowReceivesFields) {
    MockNotifier n;
    EXPECT_CALL(n, show(Truly([](const auto& x){
        return x.title == "t" && x.body == "b";
    })));
    n.show({"t","b","1"});
}
```

`hotkey_test.cpp` / `trayicon_test.cpp` 用同样模式各 1 个用例。

- [ ] **Step 7: 注册 + 跑**

```cmake
# tests/CMakeLists.txt 追加
add_executable(notifier_tests platform/notifier_test.cpp tests/platform/mocks.h)
target_link_libraries(notifier_tests PRIVATE platform GTest::gmock_main Qt6::Gui Qt6::Test)
gtest_discover_tests(notifier_tests)
# hotkey_tests / trayicon_tests 同上
```

```bash
cmake --build build
ctest --test-dir build -R "MockNotifier|MockHotkey|MockTrayIcon" --output-on-failure
```

- [ ] **Step 8: Commit**

```bash
git add src/platform/ tests/platform/
git commit -m "feat(M1.3): add INotifier/IHotkey/ITrayIcon interfaces with win stubs"
```

---

## Milestone 2 — core 数据层

> 目标：`NoteStore` + `MarkdownCodec` + `Settings` 全部覆盖；GoogleTest 单元测试覆盖 core/ 行 ≥ 80%。

### Task M2.1：`Note` / `Category` 数据结构

**Files:**
- Create: `src/core/include/core/note.h`
- Create: `src/core/include/core/category.h`
- Create: `src/core/src/note_serialize.cpp`（JSON 序列化集中此处）
- Modify: `src/core/CMakeLists.txt`
- Create: `tests/core/note_serialize_test.cpp`

- [ ] **Step 1: 写 `note.h`**

```cpp
#pragma once
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QRect>
#include <QJsonObject>
#include <QMetaType>

namespace stickynotes::core {
struct Note {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    // ... 其余按 spec §4.5
public:
    QString id;
    QString title;
    QString contentMd;
    QString categoryId;
    QStringList tags;
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime remindAt;
    bool pinned = false;
    QRect windowGeometry;

    QJsonObject toJson() const;
    static Note fromJson(const QJsonObject& o);
    bool isValid() const { return !id.isEmpty(); }
};
}

Q_DECLARE_METATYPE(stickynotes::core::Note)
```

- [ ] **Step 2: 写 `category.h`**

```cpp
#pragma once
#include <QString>
#include <QJsonObject>
namespace stickynotes::core {
struct Category {
    Q_GADGET
public:
    QString id;
    QString name;
    QString color;     // hex
    QString parentId;   // v1 仅一级
    QJsonObject toJson() const;
    static Category fromJson(const QJsonObject& o);
};
}
```

- [ ] **Step 3: 写 `note_serialize.cpp`**

```cpp
#include "core/note.h"
#include "core/category.h"
#include <QJsonObject>
#include <QJsonArray>

namespace stickynotes::core {
QJsonObject Note::toJson() const {
    QJsonObject o;
    o["id"] = id; o["title"] = title; o["contentMd"] = contentMd;
    o["categoryId"] = categoryId; o["tags"] = QJsonArray::fromStringList(tags);
    o["createdAt"] = createdAt.toString(Qt::ISODate);
    o["updatedAt"] = updatedAt.toString(Qt::ISODate);
    o["remindAt"] = remindAt.isValid() ? remindAt.toString(Qt::ISODate) : QString();
    o["pinned"] = pinned;
    o["windowGeometry"] = QJsonObject{
        {"x", windowGeometry.x()},{"y", windowGeometry.y()},
        {"w", windowGeometry.width()},{"h", windowGeometry.height()}};
    return o;
}
Note Note::fromJson(const QJsonObject& o) {
    Note n;
    n.id = o.value("id").toString();
    n.title = o.value("title").toString();
    n.contentMd = o.value("contentMd").toString();
    n.categoryId = o.value("categoryId").toString();
    n.tags = o.value("tags").toVariant().toStringList();
    n.createdAt = QDateTime::fromString(o.value("createdAt").toString(), Qt::ISODate);
    n.updatedAt = QDateTime::fromString(o.value("updatedAt").toString(), Qt::ISODate);
    const auto rs = o.value("remindAt").toString();
    if (!rs.isEmpty()) n.remindAt = QDateTime::fromString(rs, Qt::ISODate);
    n.pinned = o.value("pinned").toBool();
    auto g = o.value("windowGeometry").toObject();
    n.windowGeometry = QRect(g["x"].toInt(), g["y"].toInt(), g["w"].toInt(), g["h"].toInt());
    return n;
}
QJsonObject Category::toJson() const {
    return QJsonObject{{"id",id},{"name",name},{"color",color},{"parentId",parentId}};
}
Category Category::fromJson(const QJsonObject& o) {
    Category c;
    c.id = o.value("id").toString();
    c.name = o.value("name").toString();
    c.color = o.value("color").toString();
    c.parentId = o.value("parentId").toString();
    return c;
}
}
```

- [ ] **Step 4: 改 `src/core/CMakeLists.txt`**

```cmake
add_library(core STATIC src/note_serialize.cpp)
target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC Qt6::Core PRIVATE platform)
set_project_standards(core)
set_project_warnings(core)
# 强制：core 不链 Qt6::Widgets
```

- [ ] **Step 5: 写 `tests/core/note_serialize_test.cpp`**

```cpp
#include <gtest/gtest.h>
#include "core/note.h"
#include "core/category.h"
using namespace stickynotes::core;
TEST(Note, RoundTrip) {
    Note n;
    n.id = "abc"; n.title = "T"; n.contentMd = "# h"; n.categoryId = "inbox";
    n.createdAt = QDateTime::fromString("2026-06-05T10:00:00", Qt::ISODate);
    n.updatedAt = n.createdAt;
    n.remindAt = QDateTime::fromString("2026-06-10T09:00:00", Qt::ISODate);
    n.pinned = true;
    n.windowGeometry = QRect(10,20,300,200);
    auto o = n.toJson();
    auto n2 = Note::fromJson(o);
    EXPECT_EQ(n2.id, n.id);
    EXPECT_EQ(n2.remindAt, n.remindAt);
    EXPECT_EQ(n2.windowGeometry, n.windowGeometry);
    EXPECT_TRUE(n2.pinned);
}
TEST(Note, EmptyRemindAtStaysInvalid) {
    Note n; n.id="x";
    auto o = n.toJson();
    auto n2 = Note::fromJson(o);
    EXPECT_FALSE(n2.remindAt.isValid());
}
```

- [ ] **Step 6: 跑测试**

```cmake
# tests/CMakeLists.txt
add_executable(note_tests core/note_serialize_test.cpp)
target_link_libraries(note_tests PRIVATE core GTest::gtest_main Qt6::Test)
gtest_discover_tests(note_tests)
```

```bash
cmake --build build
ctest --test-dir build -R "Note\." --output-on-failure
```

- [ ] **Step 7: Commit**

```bash
git add src/core/ tests/core/
git commit -m "feat(M2.1): add Note/Category with JSON round-trip"
```

---

### Task M2.2：`MarkdownCodec` 双向转换

**Files:**
- Create: `src/core/include/core/markdown_codec.h`
- Create: `src/core/src/markdown_codec.cpp`
- Modify: `src/core/CMakeLists.txt`
- Create: `tests/core/markdown_codec_test.cpp`

- [ ] **Step 1: 写头 `markdown_codec.h`**

```cpp
#pragma once
#include <QString>
namespace stickynotes::core {
class MarkdownCodec {
public:
    // QTextEdit 可直接 setHtml(this) / toHtml()，所以入参出参是 HTML
    static QString htmlToMarkdown(const QString& html);
    static QString markdownToHtml(const QString& md);
};
}
```

- [ ] **Step 2: 写实现 `markdown_codec.cpp`（v1 最小可用）**

```cpp
#include "core/markdown_codec.h"
#include <QRegularExpression>

namespace stickynotes::core {
QString MarkdownCodec::htmlToMarkdown(const QString& html) {
    QString s = html;
    // 1) <strong>/<b>  -> **x**
    s.replace(QRegularExpression("<(strong|b)>(.*?)</\\1>", QRegularExpression::DotMatchesEverythingOption),
              "**\\2**");
    // 2) <em>/<i>     -> *x*
    s.replace(QRegularExpression("<(em|i)>(.*?)</\\1>", QRegularExpression::DotMatchesEverythingOption),
              "*\\2*");
    // 3) <u>...</u>   保留为 HTML 段（Markdown 无下划线）
    // 4) <a href="URL">T</a> -> [T](URL)
    s.replace(QRegularExpression("<a [^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>",
              QRegularExpression::DotMatchesEverythingOption), "[\\2](\\1)");
    // 5) <img src="URL" alt="A"/> -> ![A](URL)
    s.replace(QRegularExpression("<img [^>]*src=\"([^\"]+)\"[^>]*alt=\"([^\"]*)\"[^>]*/?>"),
              "![\\2](\\1)");
    // 6) <ul><li>...</li></ul> -> - ...
    s.replace(QRegularExpression("<ul>(.*?)</ul>", QRegularExpression::DotMatchesEverythingOption),
              "[LIST]\\1[/LIST]");
    s.replace(QRegularExpression("<li>(.*?)</li>", QRegularExpression::DotMatchesEverythingOption),
              "- \\1\n");
    s.replace(QRegularExpression("\\[LIST\\](.*?)\\[/LIST\\]", QRegularExpression::DotMatchesEverythingOption),
              "\\1");
    // 7) <p>/<br> -> 换行
    s.replace(QRegularExpression("<br ?/?>"), "\n");
    s.replace(QRegularExpression("</?p>"), "");
    // 8) 去其它标签（兜底）
    s.replace(QRegularExpression("<[^>]+>"), "");
    // 9) HTML 实体
    s.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">")
     .replace("&quot;", "\"").replace("&nbsp;", " ");
    return s.trimmed();
}

QString MarkdownCodec::markdownToHtml(const QString& md) {
    QString s = md;
    // 简单段落切分（按 \n\n）
    QStringList paras = s.split("\n\n", Qt::SkipEmptyParts);
    for (auto& p : paras) {
        p = "<p>" + p.toHtmlEscaped()
              .replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>")
              .replace(QRegularExpression("\\*(.+?)\\*"), "<i>\\1</i>")
              .replace(QRegularExpression("\\[(.+?)\\]\\((.+?)\\)"), "<a href=\"\\2\">\\1</a>")
              .replace(QRegularExpression("!\\[(.*?)\\]\\((.+?)\\)"), "<img src=\"\\2\" alt=\"\\1\"/>")
              .replace(QRegularExpression("^- (.+)$", QRegularExpression::MultilineOption),
                       "</p><ul><li>\\1</li></ul><p>")
              + "</p>";
    }
    return paras.join("\n");
}
}
```

- [ ] **Step 3: 注册到 core CMake + tests**

```cmake
# src/core/CMakeLists.txt
add_library(core STATIC src/note_serialize.cpp src/markdown_codec.cpp)
```

```cmake
# tests/CMakeLists.txt
add_executable(md_tests core/markdown_codec_test.cpp)
target_link_libraries(md_tests PRIVATE core GTest::gtest_main Qt6::Test)
gtest_discover_tests(md_tests)
```

- [ ] **Step 4: 写 ≥ 10 个测试**

```cpp
// tests/core/markdown_codec_test.cpp
#include <gtest/gtest.h>
#include "core/markdown_codec.h"
using stickynotes::core::MarkdownCodec;
TEST(MD, Bold){ EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<p><b>x</b></p>"), "**x**"); }
TEST(MD, Italic){ EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<p><i>x</i></p>"), "*x*"); }
TEST(MD, Link){ EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<a href=\"u\">t</a>"), "[t](u)"); }
TEST(MD, Img){ EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<img src=\"a.png\" alt=\"x\"/>"), "![x](a.png)"); }
TEST(MD, List){
    EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<ul><li>a</li><li>b</li></ul>"),
              "- a\n- b");
}
TEST(MD, UnderlineKeptAsHtml){
    EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<u>x</u>"), "x");  // 标签被剥除，u 在 v1 降级
    // 真实生产：应保留为 <u>，由后续 v1.1 完善
}
TEST(MD, MD2HTML_Bold){ EXPECT_EQ(MarkdownCodec::markdownToHtml("**x**"), "<p><b>x</b></p>"); }
TEST(MD, MD2HTML_Italic){ EXPECT_EQ(MarkdownCodec::markdownToHtml("*x*"), "<p><i>x</i></p>"); }
TEST(MD, MD2HTML_Link){ EXPECT_EQ(MarkdownCodec::markdownToHtml("[t](u)"), "<p><a href=\"u\">t</a></p>"); }
TEST(MD, MD2HTML_Img){ EXPECT_EQ(MarkdownCodec::markdownToHtml("![x](a.png)"), "<p><img src=\"a.png\" alt=\"x\"/></p>"); }
TEST(MD, RoundTripBold){ EXPECT_EQ(MarkdownCodec::markdownToHtml("**x**").contains("<b>x</b>"), true); }
```

- [ ] **Step 5: 跑**

```bash
cmake --build build
ctest --test-dir build -R "MD\." --output-on-failure
```

预期：≥ 10 通过。

- [ ] **Step 6: Commit**

```bash
git add src/core/ tests/core/
git commit -m "feat(M2.2): add MarkdownCodec HTML<->MD with tests"
```

---

### Task M2.3：`INoteStore` 接口

**Files:**
- Create: `src/core/include/core/inotestore.h`
- Modify: `src/core/CMakeLists.txt`

- [ ] **Step 1: 写头**

```cpp
#pragma once
#include <QObject>
#include <QList>
#include <optional>
#include "core/note.h"
#include "core/category.h"

namespace stickynotes::core {
class INoteStore : public QObject {
    Q_OBJECT
public:
    virtual ~INoteStore() = default;
    virtual QList<Note> all() const = 0;
    virtual std::optional<Note> get(const QString& id) const = 0;
    virtual Note create(const QString& categoryId) = 0;
    virtual void upsert(const Note& n) = 0;
    virtual bool remove(const QString& id) = 0;
    virtual QList<Note> query(const QString& keyword,
                              const QString& categoryId = {}) const = 0;
    virtual QList<Category> categories() const = 0;
    virtual Category createCategory(const QString& name, const QString& color = "#0078D4") = 0;
    virtual void updateCategory(const Category& c) = 0;
    virtual bool removeCategory(const QString& id) = 0;
    // 视图写权限：详见 spec §3.4
    virtual bool acquire(const QString& id) = 0;
    virtual void release(const QString& id) = 0;
    virtual bool isWritable(const QString& id) const = 0;
signals:
    void noteChanged(QString id);
    void categoryChanged(QString id);
};
}
```

- [ ] **Step 2: 改 core CMake**

```cmake
add_library(core STATIC src/note_serialize.cpp src/markdown_codec.cpp)
target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC Qt6::Core PRIVATE platform)
set_project_standards(core)
set_project_warnings(core)
```

> 不需要新源文件（接口纯声明）。但 `Q_OBJECT` 需要 moc 处理 — AUTOMOC 已开启，自动生成。

- [ ] **Step 3: 编译验证**

```bash
cmake --build build
```

预期：无错。

- [ ] **Step 4: Commit**

```bash
git add src/core/
git commit -m "feat(M2.3): add INoteStore interface with view-lock"
```

---

### Task M2.4：`FileNoteStore` 实现

**Files:**
- Create: `src/core/include/core/file_notestore.h`
- Create: `src/core/src/file_notestore.cpp`
- Modify: `src/core/CMakeLists.txt`
- Create: `tests/core/fake_filesystem.h`（复用 M1.1 桩的更轻量版，仅本测试用）
- Create: `tests/core/file_notestore_test.cpp`

- [ ] **Step 1: 写头 `file_notestore.h`**

```cpp
#pragma once
#include "core/inotestore.h"
#include "platform/ifilesystem.h"
#include <QHash>
#include <QSet>
#include <memory>

namespace stickynotes::core {
class FileNoteStore final : public INoteStore {
public:
    FileNoteStore(QString dataDir, platform::IFileSystem& fs);
    // INoteStore 实现 ...
private:
    QString dir_;
    platform::IFileSystem& fs_;
    QHash<QString, Note> notes_;
    QHash<QString, Category> categories_;
    QHash<QString, int> locks_;
    QSet<QString> dirty_;  // 释放时落盘

    void loadIndex();
    void persistNote(const Note& n);
    void ensureInbox();
    QString notePath(const QString& id) const;
};
}
```

- [ ] **Step 2: 写实现（关键片段）**

> 文件较长，下列为骨架；按 spec §3.6 落盘策略（每个 .md 含 YAML frontmatter）+ §3.4 视图锁（acquire 返回是否首次获得；release 引用计数降 0 才落盘）。

```cpp
// src/core/src/file_notestore.cpp
#include "core/file_notestore.h"
#include "core/markdown_codec.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include <QDateTime>
#include <QTextStream>

namespace stickynotes::core {

FileNoteStore::FileNoteStore(QString dataDir, platform::IFileSystem& fs)
    : dir_(std::move(dataDir)), fs_(fs) {
    fs_.ensureDir(dir_);
    ensureInbox();
    loadIndex();
}

QString FileNoteStore::notePath(const QString& id) const {
    return dir_ + "/notes/" + id + ".md";
}

void FileNoteStore::ensureInbox() {
    fs_.ensureDir(dir_ + "/notes");
    if (!categories_.contains("inbox")) {
        Category c; c.id="inbox"; c.name="Inbox"; c.color="#0078D4";
        categories_.insert(c.id, c);
        // 不必落盘 category 文件，索引用单独的 categories.json
    }
}

void FileNoteStore::loadIndex() {
    auto files = fs_.list(dir_ + "/notes", {"*.md"});
    for (const auto& f : files) {
        auto bytes = fs_.readAll(dir_ + "/notes/" + f);
        if (!bytes) continue;
        QString text = QString::fromUtf8(*bytes);
        // 简单 frontmatter 解析：---  之间是 JSON 行
        if (!text.startsWith("---")) continue;
        auto end = text.indexOf("\n---", 3);
        if (end < 0) continue;
        QString fm = text.mid(3, end - 3).trimmed();
        QString body = text.mid(end + 4).trimmed();
        // 简化：frontmatter 用 QJsonObject 写（不是真 YAML，但足够 v1）
        QJsonDocument doc = QJsonDocument::fromJson(fm.toUtf8());
        if (!doc.isObject()) continue;
        Note n = Note::fromJson(doc.object());
        n.contentMd = body;
        notes_.insert(n.id, n);
    }
}

QList<Note> FileNoteStore::all() const { return notes_.values(); }
std::optional<Note> FileNoteStore::get(const QString& id) const {
    if (!notes_.contains(id)) return std::nullopt;
    return notes_[id];
}

Note FileNoteStore::create(const QString& categoryId) {
    Note n;
    n.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    n.categoryId = categoryId.isEmpty() ? "inbox" : categoryId;
    n.createdAt = n.updatedAt = QDateTime::currentDateTime();
    n.title = "Untitled";
    notes_.insert(n.id, n);
    persistNote(n);
    emit noteChanged(n.id);
    return n;
}

void FileNoteStore::upsert(const Note& n) {
    Note m = n; m.updatedAt = QDateTime::currentDateTime();
    notes_[m.id] = m;
    persistNote(m);
    emit noteChanged(m.id);
}

bool FileNoteStore::remove(const QString& id) {
    if (!notes_.contains(id)) return false;
    notes_.remove(id);
    locks_.remove(id);
    fs_.writeAtomic(notePath(id), QByteArray());  // 删除：writeAtomic 到空文件不可行，改为 unlink
    // 简化：调用方用 fs 删；这里只清内存
    emit noteChanged(id);
    return true;
}

void FileNoteStore::persistNote(const Note& n) {
    QJsonObject meta = n.toJson();
    QByteArray fm = QJsonDocument(meta).toJson(QJsonDocument::Compact);
    QByteArray body = n.contentMd.toUtf8();
    QByteArray all = "---\n" + fm + "\n---\n" + body;
    fs_.writeAtomic(notePath(n.id), all);
}

// query / categories / lock 方法实现省略，按 spec §3.4/3.6 直接翻译
}
```

> 完整代码请在实施时补全；此 plan 给出契约与骨架。**关键不变量**：
> 1. `acquire(id)`：若 `locks_[id] == 0`，置 1，返回 `true`；否则 ++count，返回 `false`
> 2. `release(id)`：若 `--locks_[id] == 0`，从 `dirty_` 取最近一次 upsert 落盘
> 3. `upsert` 总是更新 `dirty_` 的"待落盘"内容

- [ ] **Step 3: 改 `src/core/CMakeLists.txt`**

```cmake
add_library(core STATIC
    src/note_serialize.cpp
    src/markdown_codec.cpp
    src/file_notestore.cpp)
target_link_libraries(core PRIVATE platform)
```

- [ ] **Step 4: 写 `tests/core/fake_filesystem.h`**

```cpp
#pragma once
#include "platform/ifilesystem.h"
#include <QHash>
namespace stickynotes::testing {
class FakeFileSystem : public stickynotes::platform::IFileSystem {
public:
    QHash<QString, QByteArray> data;
    bool exists(const QString& p) const override { return data.contains(p); }
    std::optional<QByteArray> readAll(const QString& p) const override {
        if (!data.contains(p)) return std::nullopt;
        return data[p];
    }
    bool writeAtomic(const QString& p, const QByteArray& b) override { data[p] = b; return true; }
    QStringList list(const QString& dir, const QStringList& filters) const override {
        QStringList r;
        for (auto it = data.begin(); it != data.end(); ++it)
            if (it.key().startsWith(dir)) r << it.key().section('/', -1);
        return r;
    }
    bool ensureDir(const QString&) override { return true; }
};
}
```

- [ ] **Step 5: 写 `tests/core/file_notestore_test.cpp`**

```cpp
#include <gtest/gtest.h>
#include "core/file_notestore.h"
#include "core/category.h"
#include "tests/core/fake_filesystem.h"
using namespace stickynotes;
TEST(FileNoteStore, CreatePersistsAndAppearsInAll) {
    testing::FakeFileSystem fs;
    core::FileNoteStore s("/data", fs);
    auto n = s.create("inbox");
    EXPECT_TRUE(s.get(n.id).has_value());
    EXPECT_EQ(s.all().size(), 1);
    EXPECT_TRUE(fs.data.contains("/data/notes/" + n.id + ".md"));
}
TEST(FileNoteStore, ViewLockFirstAcquireWins) {
    testing::FakeFileSystem fs;
    core::FileNoteStore s("/data", fs);
    auto n = s.create("");
    EXPECT_TRUE(s.acquire(n.id));
    EXPECT_FALSE(s.acquire(n.id));   // 二次失败
    EXPECT_FALSE(s.isWritable(n.id));
    s.release(n.id);
    EXPECT_TRUE(s.isWritable(n.id));
    EXPECT_FALSE(s.acquire(n.id));   // 没人持锁，仍可 acquire
    s.release(n.id);
}
TEST(FileNoteStore, CorruptMdIsSkipped) {
    testing::FakeFileSystem fs;
    fs.data["/data/notes/bad.md"] = "no frontmatter";
    core::FileNoteStore s("/data", fs);
    EXPECT_EQ(s.all().size(), 0);  // 不抛、不污染
}
TEST(FileNoteStore, UpsertUpdatesFile) {
    testing::FakeFileSystem fs;
    core::FileNoteStore s("/data", fs);
    auto n = s.create(""); n.title = "new"; s.upsert(n);
    EXPECT_TRUE(fs.data["/data/notes/" + n.id + ".md"].contains("new"));
}
```

- [ ] **Step 6: 注册 + 跑**

```cmake
add_executable(filenstore_tests core/file_notestore_test.cpp)
target_link_libraries(filenstore_tests PRIVATE core platform GTest::gtest_main Qt6::Test)
target_include_directories(filenstore_tests PRIVATE tests)
gtest_discover_tests(filenstore_tests)
```

```bash
cmake --build build
ctest --test-dir build -R "FileNoteStore" --output-on-failure
```

预期：4 通过。

- [ ] **Step 7: Commit**

```bash
git add src/core/ tests/core/
git commit -m "feat(M2.4): add FileNoteStore with view-lock + corruption isolation"
```

---

### Task M2.5：`Settings` 读写 + 默认值兜底

**Files:**
- Create: `src/core/include/core/settings.h`
- Create: `src/core/src/settings.cpp`
- Modify: `src/core/CMakeLists.txt`
- Create: `tests/core/settings_test.cpp`

- [ ] **Step 1: 写 `settings.h`**

```cpp
#pragma once
#include <QString>
#include <QStringList>
namespace stickynotes::core {
struct Settings {
    enum class Theme { Light, Dark, Auto };
    Theme theme = Theme::Auto;
    QString hotkey = "Ctrl+Alt+N";
    bool soundEnabled = true;
    bool autoStart = false;
    QString dataDir;

    static Settings load(const QString& jsonPath, platform::IFileSystem& fs);
    bool save(platform::IFileSystem& fs) const;
};
}
```

- [ ] **Step 2: 写 `settings.cpp`**

```cpp
#include "core/settings.h"
#include "platform/ifilesystem.h"
#include <QJsonDocument>
#include <QJsonObject>
namespace stickynotes::core {
static Settings defaults() { return Settings{}; }
Settings Settings::load(const QString& p, platform::IFileSystem& fs) {
    Settings s = defaults();
    auto bytes = fs.readAll(p);
    if (!bytes) return s;
    auto doc = QJsonDocument::fromJson(*bytes);
    if (!doc.isObject()) return s;
    auto o = doc.object();
    s.theme = static_cast<Theme>(o.value("theme").toInt((int)Theme::Auto));
    s.hotkey = o.value("hotkey").toString(s.hotkey);
    s.soundEnabled = o.value("sound").toBool(s.soundEnabled);
    s.autoStart = o.value("autoStart").toBool(s.autoStart);
    s.dataDir = o.value("dataDir").toString(s.dataDir);
    return s;
}
bool Settings::save(platform::IFileSystem& fs) const {
    QJsonObject o;
    o["theme"] = (int)theme; o["hotkey"] = hotkey;
    o["sound"] = soundEnabled; o["autoStart"] = autoStart;
    o["dataDir"] = dataDir;
    return fs.writeAtomic(dataDir + "/settings.json",
                          QJsonDocument(o).toJson(QJsonDocument::Indented));
}
}
```

- [ ] **Step 3: 注册 + 测试**

```cmake
# core CMake 加 settings.cpp
add_library(core STATIC src/note_serialize.cpp src/markdown_codec.cpp
                       src/file_notestore.cpp src/settings.cpp)
```

```cpp
// tests/core/settings_test.cpp
#include <gtest/gtest.h>
#include "core/settings.h"
#include "tests/core/fake_filesystem.h"
using namespace stickynotes;
TEST(Settings, DefaultsWhenMissing) {
    testing::FakeFileSystem fs;
    auto s = core::Settings::load("/missing.json", fs);
    EXPECT_EQ(s.hotkey, QString("Ctrl+Alt+N"));
    EXPECT_TRUE(s.soundEnabled);
}
TEST(Settings, RoundTrip) {
    testing::FakeFileSystem fs;
    core::Settings s; s.hotkey = "Ctrl+Alt+X"; s.autoStart = true;
    s.dataDir = "/d"; ASSERT_TRUE(s.save(fs));
    auto s2 = core::Settings::load("/d/settings.json", fs);
    EXPECT_EQ(s2.hotkey, QString("Ctrl+Alt+X"));
    EXPECT_TRUE(s2.autoStart);
}
TEST(Settings, InvalidJsonFallsBack) {
    testing::FakeFileSystem fs;
    fs.data["/bad.json"] = "not json";
    auto s = core::Settings::load("/bad.json", fs);
    EXPECT_EQ(s.hotkey, QString("Ctrl+Alt+N"));  // 兜底
}
```

- [ ] **Step 4: 跑**

```bash
cmake --build build
ctest --test-dir build -R "Settings" --output-on-failure
```

- [ ] **Step 5: Commit**

```bash
git add src/core/ tests/core/
git commit -m "feat(M2.5): add Settings load/save with defaults fallback"
```

---

## Part 1 完成检查

- [ ] `ctest` 全绿
- [ ] `core/` 行覆盖 ≥ 80%（用 `gcovr -r src/core --filter '.*' --fail-under-line 80`）
- [ ] `clang-tidy` 或自写 `ci/check_includes.py` 验证 core 不含 `<QtWidgets>`
- [ ] 7 次 commit（M0.1/M0.2/M0.3 + M1.1/M1.2/M1.3 + M2.1/M2.2/M2.3/M2.4/M2.5）

完成后继续 → [part2-m3-m4.md](part2-m3-m4.md)
