#include "core/note.h"
#include "core/category.h"
#include <QJsonArray>
#include <QJsonObject>

namespace stickynotes::core {
QJsonObject Note::toJson() const {
    QJsonObject o;
    o["id"] = id;
    o["title"] = title;
    o["content"] = content;
    o["categoryId"] = categoryId;
    o["tags"] = QJsonArray::fromStringList(tags);
    o["createdAt"] = createdAt.toString(Qt::ISODate);
    o["updatedAt"] = updatedAt.toString(Qt::ISODate);
    o["remindAt"] = remindAt.isValid() ? remindAt.toString(Qt::ISODate) : QString();
    o["pinned"] = pinned;
    o["windowGeometry"] = QJsonObject{
        {"x", windowGeometry.x()}, {"y", windowGeometry.y()},
        {"w", windowGeometry.width()}, {"h", windowGeometry.height()}};
    return o;
}

Note Note::fromJson(const QJsonObject& o) {
    Note n;
    n.id = o.value("id").toString();
    n.title = o.value("title").toString();
    // 兼容老数据：优先读 content，缺失则回退到 contentMd
    n.content = o.value("content").toString();
    if (n.content.isEmpty()) {
        n.content = o.value("contentMd").toString();
    }
    // 兼容老数据：title 为空时回退到 content 首行
    if (n.title.isEmpty() && !n.content.isEmpty()) {
        n.title = n.content.section('\n', 0, 0).left(80);
    }
    n.categoryId = o.value("categoryId").toString();
    n.tags = o.value("tags").toVariant().toStringList();
    n.createdAt = QDateTime::fromString(o.value("createdAt").toString(), Qt::ISODate);
    n.updatedAt = QDateTime::fromString(o.value("updatedAt").toString(), Qt::ISODate);
    const auto rs = o.value("remindAt").toString();
    if (!rs.isEmpty()) n.remindAt = QDateTime::fromString(rs, Qt::ISODate);
    const auto ds = o.value("deletedAt").toString();
    if (!ds.isEmpty()) n.deletedAt = QDateTime::fromString(ds, Qt::ISODate);
    n.pinned = o.value("pinned").toBool();
    auto g = o.value("windowGeometry").toObject();
    n.windowGeometry = QRect(g["x"].toInt(), g["y"].toInt(), g["w"].toInt(), g["h"].toInt());
    return n;
}

QJsonObject Category::toJson() const {
    return QJsonObject{
        {"id", id}, {"name", name}, {"color", color}, {"parentId", parentId}};
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
