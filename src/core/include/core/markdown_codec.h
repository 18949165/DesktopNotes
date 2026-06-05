#pragma once
#include <QString>

namespace stickynotes::core {
class MarkdownCodec {
public:
    static QString htmlToMarkdown(const QString& html);
    static QString markdownToHtml(const QString& md);
};
}
