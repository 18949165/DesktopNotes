#pragma once
#include <QString>
#include <QJsonObject>
#include <QMetaType>

namespace stickynotes::core {
struct Category {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString color MEMBER color)
    Q_PROPERTY(QString parentId MEMBER parentId)
public:
    QString id;
    QString name;
    QString color;     // hex
    QString parentId;  // v1 仅一级，留扩展

    QJsonObject toJson() const;
    static Category fromJson(const QJsonObject& o);
};
}

Q_DECLARE_METATYPE(stickynotes::core::Category)
