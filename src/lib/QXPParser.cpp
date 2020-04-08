/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPParser.h"

#include "QXPContentCollector.h"
#include "QXPHeader.h"

#include <cmath>
#include <memory>

namespace libqxp
{

using std::make_shared;

QXPParser::QXPParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter, const std::shared_ptr<QXPHeader> &header)
  : m_input(input)
  , m_painter(painter)
  , be(header->isBigEndian())
  , m_blockParser(input, header)
  , m_textParser(input, header)
  , m_colors()
  , m_fonts()
  , m_charFormats()
  , m_paragraphFormats()
  , m_lineStyles()
  , m_arrows()
  , m_hjs()
  , m_groupObjects()
  , m_header(header)
{
  // default colors, in case parsing fails
  m_colors[0] = Color(255, 255, 255); // white
  m_colors[1] = Color(0, 0, 0); // black
  m_colors[2] = Color(255, 0, 0); // red
  m_colors[3] = Color(0, 255, 0); // green
  m_colors[4] = Color(0, 0, 255); // blue
  m_colors[5] = Color(1, 160, 198); // cyan
  m_colors[6] = Color(239, 4, 127); // magenta
  m_colors[7] = Color(255, 255, 0); // yellow
  m_colors[8] = Color(0, 0, 0); // registration

  // customm dashes are available only from 4.0
  m_lineStyles[0] = LineStyle({}, true, 1.0, LineCapType::BUTT, LineJoinType::MITER);
  m_lineStyles[1] = LineStyle({0.6, 0.4}, true, 5.0, LineCapType::BUTT, LineJoinType::MITER);
  m_lineStyles[2] = LineStyle({0.75, 0.25}, true, 4.0, LineCapType::BUTT, LineJoinType::MITER);
  m_lineStyles[3] = LineStyle({0.5455, 0.1818, 0.0909, 0.1818}, true, 11.0, LineCapType::BUTT, LineJoinType::MITER);
  m_lineStyles[4] = LineStyle({0.0, 1.0}, true, 2.0, LineCapType::ROUND, LineJoinType::MITER);

  m_arrows = std::vector<Arrow>( // should be in ctor, but breaks astyle
  {
    // does viewbox has any effect?
    Arrow("m9 0 l-9 25 l6 -1.5 l6 0 l6 1.5 z", "0 0 18 25", 3),
    Arrow("m9 5 l-9 -5 l0 20 l6 10 l6 0 l6 -10 l0 -20 z", "0 0 18 35", 2.5)
  });
}

bool QXPParser::parse()
{
  QXPContentCollector collector(m_painter);

  collector.startDocument();

  auto docStream = m_blockParser.getChain(3);
  if (!parseDocument(docStream, collector))
    return false;

  if (!parsePages(docStream, collector))
    return false;

  collector.endDocument();

  return true;
}

Color QXPParser::getColor(unsigned id, Color defaultColor) const
{
  auto it = m_colors.find(id);
  if (it == m_colors.end())
  {
    QXP_DEBUG_MSG(("Color %u not found\n", id));
    return defaultColor;
  }
  return (*it).second;
}

const LineStyle *QXPParser::getLineStyle(unsigned id) const
{
  auto it = m_lineStyles.find(id);
  if (it == m_lineStyles.end())
  {
    QXP_DEBUG_MSG(("Line style %u not found\n", id));
    return nullptr;
  }
  return &it->second;
}

std::string QXPParser::getFont(int id, std::string defaultFont) const
{
  auto it = m_fonts.find(id);
  if (it == m_fonts.end())
  {
    QXP_DEBUG_MSG(("Font %d not found\n", id));
    return defaultFont;
  }
  return (*it).second;
}

void QXPParser::skipRecord(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  unsigned length = readU32(stream, be);
  if (length > 0)
  {
    skip(stream, length);
  }
}

void QXPParser::parseFonts(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  unsigned length = readU32(stream, be);
  if (length > getRemainingLength(stream))
  {
    QXP_DEBUG_MSG(("Invalid record length %u\n", length));
    throw ParseError();
  }
  const long end = stream->tell() + length;

  try
  {
    unsigned count = readU16(stream, be);
    for (unsigned i = 0; i < count; ++i)
    {
      int index = readS16(stream, be);
      if (m_header->version() >= QXP_4)
      {
        skip(stream, 2);
      }
      auto name = readPlatformString(stream, be);
      readPlatformString(stream, be); // skip

      m_fonts[index] = name;
    }
  }
  catch (...)
  {
    QXP_DEBUG_MSG(("Failed to parse fonts, offset %ld\n", stream->tell()));
  }

  seek(stream, end);
}

void QXPParser::parseHJs(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  parseCollection(stream, [=]()
  {
    m_hjs.push_back(parseHJ(stream));
  });
}

void QXPParser::parseCharFormats(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  m_charFormats.clear();
  parseCollection(stream, [=]()
  {
    m_charFormats.push_back(make_shared<CharFormat>(parseCharFormat(stream)));
  });
}

void QXPParser::parseHJProps(const std::shared_ptr<librevenge::RVNGInputStream> &stream, HJ &result)
{
  skip(stream, 1);
  result.minBefore = readU8(stream);
  result.minAfter = readU8(stream);
  result.maxInRow = readU8(stream);
  skip(stream, 4);
  result.singleWordJustify = readU8(stream) == 0;
  skip(stream, 1);
  result.hyphenate = readU8(stream) != 0;
  skip(stream, 33);
}

void QXPParser::parseCommonCharFormatProps(const std::shared_ptr<librevenge::RVNGInputStream> &stream, CharFormat &result)
{
  const int fontIndex = readS16(stream, be);
  result.fontName = getFont(fontIndex).c_str();

  const unsigned flags = readU16(stream, be);
  convertCharFormatFlags(flags, result);

  result.fontSize = readFraction(stream, be);
}

TabStop QXPParser::parseTabStop(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  TabStop tabStop;

  const uint8_t type = readU8(stream);
  tabStop.type = convertTabStopType(type);

  const uint8_t alignChar = readU8(stream);
  tabStop.alignChar.clear();
  switch (alignChar)
  {
  case 1:
    tabStop.alignChar.append('.');
    break;
  case 2:
    tabStop.alignChar.append(',');
    break;
  default:
    tabStop.alignChar.append(char(alignChar));
    break;
  }

  tabStop.fillChar.clear();
  tabStop.fillChar.append(char(readU16(stream, be)));

  tabStop.position = readFraction(stream, be);

  return tabStop;
}

void QXPParser::parseParagraphFormats(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  m_paragraphFormats.clear();
  parseCollection(stream, [=]()
  {
    m_paragraphFormats.push_back(make_shared<ParagraphFormat>(parseParagraphFormat(stream)));
  });
}

void QXPParser::parseCollection(const std::shared_ptr<librevenge::RVNGInputStream> stream, std::function<void()> itemHandler)
{
  unsigned length = readU32(stream, be);
  if (length > getRemainingLength(stream))
  {
    QXP_DEBUG_MSG(("Invalid record length %u\n", length));
    throw ParseError();
  }
  const long end = stream->tell() + length;

  try
  {
    while (stream->tell() < end)
    {
      itemHandler();
    }
  }
  catch (...)
  {
    QXP_DEBUG_MSG(("Failed to parse collection, offset %ld\n", stream->tell()));
  }

  seek(stream, end);
}

std::vector<PageSettings> QXPParser::parsePageSettings(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  skip(stream, 6);
  const unsigned count = readU16(stream, be);
  if (count == 0 || count > 2)
  {
    QXP_DEBUG_MSG(("Invalid page settings blocks count %u\n", count));
    throw ParseError();
  }
  skip(stream, 2);

  std::vector<PageSettings> pages;
  pages.resize(count);
  for (auto &page : pages)
  {
    page.offset.top = readFraction(stream, be);
    page.offset.left = readFraction(stream, be);
    page.offset.bottom = readFraction(stream, be);
    page.offset.right = readFraction(stream, be);
    skip(stream, 36);
    skip(stream, m_header->version() >= QXP_4 ? 12 : 8);
  }

  for (unsigned i = 0; i <= count; ++i)
  {
    const unsigned length = readU32(stream, be);
    skip(stream, length + 4);
  }

  if (!be)
  {
    skip(stream, 4);
  }
  const unsigned nameLength = readU32(stream, be);
  skip(stream, nameLength);

  return pages;
}

std::shared_ptr<Text> QXPParser::parseText(unsigned index, unsigned linkId, QXPCollector &collector)
{
  try
  {
    auto text = m_textParser.parseText(index, m_charFormats, m_paragraphFormats);
    collector.collectText(text, linkId);
    return text;
  }
  catch (...)
  {
    QXP_DEBUG_MSG(("Failed to parse text %u\n", index));
    return make_shared<Text>();
  }
}

uint32_t QXPParser::readRecordEndOffset(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  unsigned length = readU32(stream, be);
  return stream->tell() + length;
}

uint8_t QXPParser::readColorComp(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  return uint8_t(std::round(255 * readFloat16(stream, be)));
}

Rect QXPParser::readObjectBBox(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  Rect bbox;
  bbox.top = readFraction(stream, be);
  bbox.left = readFraction(stream, be);
  bbox.bottom = readFraction(stream, be);
  bbox.right = readFraction(stream, be);
  return bbox;
}

Gradient QXPParser::readGradient(const std::shared_ptr<librevenge::RVNGInputStream> &stream, const Color &color1)
{
  Gradient gradient;
  gradient.color1 = color1;

  skip(stream, m_header->version() >= QXP_4 ? 20 : 14);

  const uint8_t type = readU16(stream, be) & 0xff;
  switch (type)
  {
  default:
    QXP_DEBUG_MSG(("Unknown gradient type %u\n", type));
    QXP_FALLTHROUGH; // pick a default
  case 0x10:
    gradient.type = GradientType::LINEAR;
    break;
  case 0x18:
    gradient.type = GradientType::MIDLINEAR;
    break;
  case 0x19:
    gradient.type = GradientType::RECTANGULAR;
    break;
  case 0x1a:
    gradient.type = GradientType::DIAMOND;
    break;
  case 0x1b:
    gradient.type = GradientType::CIRCULAR;
    break;
  case 0x1c:
    gradient.type = GradientType::FULLCIRCULAR;
    break;
  }
  skip(stream, 4);

  unsigned colorId;
  if (m_header->version() >= QXP_4)
  {
    colorId = readU16(stream, be);
  }
  else
  {
    colorId = readU8(stream);
    skip(stream, 1);
  }
  const double shade = readFraction(stream, be);
  gradient.color2 = getColor(colorId).applyShade(shade);

  gradient.angle = readFraction(stream, be);
  skip(stream, 4);

  return gradient;
}

HorizontalAlignment QXPParser::readHorAlign(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  const uint8_t align = readU8(stream);
  switch (align)
  {
  default:
    QXP_DEBUG_MSG(("Unknown hor. align %u\n", align));
    QXP_FALLTHROUGH; // pick a default
  case 0:
    return HorizontalAlignment::LEFT;
  case 1:
    return HorizontalAlignment::CENTER;
  case 2:
    return HorizontalAlignment::RIGHT;
  case 3:
    return HorizontalAlignment::JUSTIFIED;
  case 4:
    return HorizontalAlignment::FORCED;
  }
}

VerticalAlignment QXPParser::readVertAlign(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  const uint8_t align = readU8(stream);
  switch (align)
  {
  default:
    QXP_DEBUG_MSG(("Unknown vert. align %u\n", align));
    QXP_FALLTHROUGH; // pick a default
  case 0:
    return VerticalAlignment::TOP;
  case 1:
    return VerticalAlignment::CENTER;
  case 2:
    return VerticalAlignment::BOTTOM;
  case 3:
    return VerticalAlignment::JUSTIFIED;
  }
}

Point QXPParser::readYX(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  Point p;
  p.y = readFraction(stream, be);
  p.x = readFraction(stream, be);
  return p;
}

std::shared_ptr<ParagraphRule> QXPParser::readParagraphRule(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  auto rule = make_shared<ParagraphRule>();

  rule->width = readFraction(stream, be);

  const unsigned styleIndex = m_header->version() >= QXP_4 ? readU16(stream, be) : readU8(stream);
  rule->lineStyle = getLineStyle(styleIndex);

  const unsigned colorId = m_header->version() >= QXP_4 ? readU16(stream, be) : readU8(stream);
  const double shade = readFraction(stream, be);
  rule->color = getColor(colorId).applyShade(shade);

  rule->leftMargin = readFraction(stream, be);
  rule->rightMargin = readFraction(stream, be);
  rule->offset = readFraction(stream, be);

  return rule;
}

uint8_t QXPParser::readParagraphFlags(const std::shared_ptr<librevenge::RVNGInputStream> &stream, bool &incrementalLeading, bool &ruleAbove, bool &ruleBelow)
{
  const uint8_t flags = readU8(stream);
  if (be)
  {
    ruleBelow = flags & 0x2;
    ruleAbove = flags & 0x4;
    incrementalLeading = flags & 0x20;
  }
  else
  {
    incrementalLeading = flags & 0x4;
    ruleAbove = flags & 0x20;
    ruleBelow = flags & 0x40;
  }
  return flags;
}

uint8_t QXPParser::readObjectFlags(const std::shared_ptr<librevenge::RVNGInputStream> &stream, bool &noColor)
{
  const uint8_t flags = readU8(stream);
  if (be)
  {
    noColor = flags & 0x80;
  }
  else
  {
    noColor = flags & 0x1;
  }
  return flags;
}

void QXPParser::setArrow(const unsigned index, Frame &frame) const
{
  switch (index)
  {
  case 1:
    frame.endArrow = &m_arrows[0];
    break;
  case 2:
    frame.startArrow = &m_arrows[0];
    break;
  case 3:
    frame.startArrow = &m_arrows[1];
    frame.endArrow = &m_arrows[0];
    break;
  case 4:
    frame.startArrow = &m_arrows[0];
    frame.endArrow = &m_arrows[1];
    break;
  case 5:
    frame.startArrow = &m_arrows[0];
    frame.endArrow = &m_arrows[0];
    break;
  }
}

void QXPParser::skipFileInfo(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  const unsigned length = readU32(stream, be);
  if (length > 0)
  {
    skip(stream, length);
  }
}

void QXPParser::convertCharFormatFlags(unsigned flags, CharFormat &format)
{
  format.bold = flags & 0x1;
  format.italic = flags & 0x2;
  format.underline = flags & 0x4;
  format.outline = flags & 0x8;
  format.shadow = flags & 0x10;
  format.superscript = flags & 0x20;
  format.subscript = flags & 0x40;
  format.superior = flags & 0x100;
  format.strike = flags & 0x200;
  format.allCaps = flags & 0x400;
  format.smallCaps = flags & 0x800;
  format.wordUnderline = flags & 0x1000;
}

TabStopType QXPParser::convertTabStopType(unsigned type)
{
  switch (type)
  {
  default:
    QXP_DEBUG_MSG(("Unknown tab stop type %u\n", type));
    QXP_FALLTHROUGH; // pick a default
  case 0:
    return TabStopType::LEFT;
  case 1:
    return TabStopType::CENTER;
  case 2:
    return TabStopType::RIGHT;
  case 3:
    return TabStopType::ALIGN;
  }
}

void QXPParser::readGroupElements(const std::shared_ptr<librevenge::RVNGInputStream> &stream, unsigned count, const unsigned objectsCount, const unsigned index, std::vector<unsigned> &elements)
{
  elements.reserve(count);
  for (unsigned i = 0; i < count; ++i)
  {
    const unsigned ind = readU32(stream, be);
    if (ind >= objectsCount || ind == index)
    {
      QXP_DEBUG_MSG(("Invalid group element index %u\n", ind));
      continue;
    }
    if (m_groupObjects.insert(ind).second)
      elements.push_back(ind);
  }
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
