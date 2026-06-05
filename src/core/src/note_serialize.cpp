#include "core/note.h"
#include "core/category.h"
#include <QJsonArray>
#include <QJsonObject>

namespace stickynotes::core {
QJsonObject Note::toJson() const {
    QJsonObject o;
    o["id"] = id;
    o["title"] = title;
    o["contentMd"] = contentMd;
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
