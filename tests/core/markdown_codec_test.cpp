#include <gtest/gtest.h>
#include "core/markdown_codec.h"
using stickynotes::core::MarkdownCodec;

TEST(MD, Bold)      { EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<p><b>x</b></p>"), "**x**"); }
TEST(MD, Italic)    { EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<p><i>x</i></p>"), "*x*"); }
TEST(MD, Link)      { EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<a href=\"u\">t</a>"), "[t](u)"); }
TEST(MD, Img)       { EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<img src=\"a.png\" alt=\"x\"/>"), "![x](a.png)"); }
TEST(MD, List)      { EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<ul><li>a</li><li>b</li></ul>"),
                                  "- a\n- b"); }
TEST(MD, MD2HTML_Bold)   { EXPECT_EQ(MarkdownCodec::markdownToHtml("**x**"), "<p><b>x</b></p>"); }
TEST(MD, MD2HTML_Italic) { EXPECT_EQ(MarkdownCodec::markdownToHtml("*x*"), "<p><i>x</i></p>"); }
TEST(MD, MD2HTML_Link)   { EXPECT_EQ(MarkdownCodec::markdownToHtml("[t](u)"), "<p><a href=\"u\">t</a></p>"); }
TEST(MD, MD2HTML_Img)    { EXPECT_EQ(MarkdownCodec::markdownToHtml("![x](a.png)"), "<p><img src=\"a.png\" alt=\"x\"/></p>"); }
TEST(MD, MD2HTML_List)   {
    auto out = MarkdownCodec::markdownToHtml("- a\n- b");
    EXPECT_TRUE(out.contains("<ul>"));
    EXPECT_TRUE(out.contains("<li>a</li>"));
    EXPECT_TRUE(out.contains("<li>b</li>"));
}
TEST(MD, EntitiesDecode) { EXPECT_EQ(MarkdownCodec::htmlToMarkdown("<p>a &amp; b</p>"), "a & b"); }
