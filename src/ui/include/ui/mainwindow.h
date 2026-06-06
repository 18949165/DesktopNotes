#pragma once
#include <ElaWindow.h>
#include <QHash>
#include <functional>
#include "app/app_context.h"

class ElaLineEdit;
class ElaListView;
class ElaMenu;
class ElaMenuBar;
class ElaToolBar;
class QTextEdit;
class QLabel;
class QButtonGroup;

namespace stickynotes::ui {
class NoteEditor;
class StickyWindow;

class MainWindow : public ElaWindow {
    Q_OBJECT
public:
    explicit MainWindow(app::AppContext& ctx, QWidget* parent = nullptr);
    void showAndRaise();
    void openStickyWindow(const QString& noteId);
    void createNewNote();
    // 注入 hotkey 重新注册回调（由 Runtime 注入）
    void setReregisterHotkey(std::function<void(const QString&)> cb) { reregisterHotkey_ = std::move(cb); }
public slots:
    void onNoteSelected(const QString& id);
protected:
    void closeEvent(QCloseEvent* e) override;
private slots:
    void onNewNote();
    void onDeleteNote();
    void onSetReminder();
    void onOpenSticky();
    void onExportNotes();
    void onImportNotes();
    void onShowSettings();
    void onCategoryChanged(int id);
private:
    void buildUi();
    void buildMenuBar();
    void buildToolBar();
    void buildBody();
    void buildStatusBar();
    void wireSignals();
    void refreshCategories();
    void refreshList();
    void updateStatusBar();

    app::AppContext& ctx_;
    std::function<void(const QString&)> reregisterHotkey_;

    // 当前过滤
    QString currentCategoryId_ = "inbox"; // inbox/today/pinned/<categoryId>
    QString currentNoteId_;

    // 中间三栏
    ElaListView* list_ = nullptr;
    NoteEditor* editor_ = nullptr;
    QWidget* editorPage_ = nullptr;

    // 工具栏
    ElaToolBar* toolBar_ = nullptr;
    QButtonGroup* catGroup_ = nullptr;

    // 状态栏
    QLabel* statusNoteCount_ = nullptr;
    QLabel* statusTheme_ = nullptr;

    // 便签窗口缓存
    QHash<QString, StickyWindow*> stickyByNote_;
};
}