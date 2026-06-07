#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QRect>
#include <QJsonObject>
#include <QMetaType>

namespace stickynotes::core {
struct Note {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QString content MEMBER content)
    Q_PROPERTY(QString categoryId MEMBER categoryId)
    Q_PROPERTY(QStringList tags MEMBER tags)
    Q_PROPERTY(QDateTime createdAt MEMBER createdAt)
    Q_PROPERTY(QDateTime updatedAt MEMBER updatedAt)
    Q_PROPERTY(QDateTime deletedAt MEMBER deletedAt)
    Q_PROPERTY(bool pinned MEMBER pinned)
    Q_PROPERTY(QRect windowGeometry MEMBER windowGeometry)
public:
    QString id;
    QString title;
    QString content;       // 纯文本笔记内容
    QString categoryId;
    QStringList tags;
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime deletedAt;   // 空 = 未删除；非空 = 软删除时间
    bool pinned = false;
    QRect windowGeometry;

    QJsonObject toJson() const;
    static Note fromJson(const QJsonObject& o);
    bool isValid() const { return !id.isEmpty(); }
};
}

Q_DECLARE_METATYPE(stickynotes::core::Note)
