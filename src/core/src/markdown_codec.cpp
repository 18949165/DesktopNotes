#include "core/markdown_codec.h"
#include <QRegularExpression>

namespace stickynotes::core {
static QString deEntitize(QString s) {
    s.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">")
     .replace("&quot;", "\"").replace("&nbsp;", " ");
    return s;
}

QString MarkdownCodec::htmlToMarkdown(const QString& html) {
    QString s = html;
    s.replace(QRegularExpression("<(strong|b)>(.*?)</\\1>",
              QRegularExpression::DotMatchesEverythingOption), "**\\2**");
    s.replace(QRegularExpression("<(em|i)>(.*?)</\\1>",
              QRegularExpression::DotMatchesEverythingOption), "*\\2*");
    s.replace(QRegularExpression("<a [^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>",
              QRegularExpression::DotMatchesEverythingOption), "[\\2](\\1)");
    s.replace(QRegularExpression("<img [^>]*src=\"([^\"]+)\"[^>]*alt=\"([^\"]*)\"[^>]*/?>"),
              "![\\2](\\1)");
    s.replace(QRegularExpression("<ul>(.*?)</ul>",
              QRegularExpression::DotMatchesEverythingOption), "[LIST]\\1[/LIST]");
    s.replace(QRegularExpression("<li>(.*?)</li>",
              QRegularExpression::DotMatchesEverythingOption), "- \\1\n");
    s.replace(QRegularExpression("\\[LIST\\](.*?)\\[/LIST\\]",
              QRegularExpression::DotMatchesEverythingOption), "\\1");
    s.replace(QRegularExpression("<br ?/?>"), "\n");
    s.replace(QRegularExpression("</?p>"), "");
    s.replace(QRegularExpression("<[^>]+>"), "");
    return deEntitize(s.trimmed());
}

QString MarkdownCodec::markdownToHtml(const QString& md) {
    QString s = md;
    QStringList paras = s.split("\n\n", Qt::SkipEmptyParts);
    for (auto& p : paras) {
        QString inner = p.toHtmlEscaped();
        inner.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
        inner.replace(QRegularExpression("\\*(.+?)\\*"), "<i>\\1</i>");
        inner.replace(QRegularExpression("!\\[(.*?)\\]\\((.+?)\\)"),
                     "<img src=\"\\2\" alt=\"\\1\"/>");
        inner.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)"),
                     "<a href=\"\\2\">\\1</a>");
        inner.replace(QRegularExpression("^- (.+)$", QRegularExpression::MultilineOption),
                     "</p><ul><li>\\1</li></ul><p>");
        if (!inner.startsWith("<p>")) inner = "<p>" + inner;
        if (!inner.endsWith("</p>")) inner += "</p>";
        p = inner;
    }
    return paras.join("\n");
}
}
