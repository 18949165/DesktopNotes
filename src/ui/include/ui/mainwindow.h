#pragma once
#include <QMainWindow>
#include "app/app_context.h"

class QLineEdit;
class QListView;
class QTreeView;

namespace stickynotes::ui {
class NoteEditor;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(app::AppContext& ctx, QWidget* parent = nullptr);
    void showAndRaise();
public slots:
    void onNoteSelected(const QString& id);
private:
    void buildUi();
    void wireSignals();
    void refreshCategories();
    void refreshList(const QString& categoryId, const QString& keyword = {});
    
    app::AppContext& ctx_;
    QLineEdit* search_ = nullptr;
    QTreeView* tree_ = nullptr;
    QListView* list_ = nullptr;
    NoteEditor* editor_ = nullptr;
    QString currentCategoryId_ = "inbox";
    QString currentNoteId_;
};
}
