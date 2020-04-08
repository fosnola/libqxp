/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXP1Parser.h"

#include "QXP1Header.h"
#include "QXPCollector.h"

namespace libqxp
{

using librevenge::RVNGInputStream;
using std::make_shared;
using std::shared_ptr;

namespace
{

double getShade(const unsigned shadeId)
{
  if (shadeId < 3)
    return 0.1 * shadeId;
  else if (shadeId < 6)
    return 0.2 * (shadeId - 1);
  else
    return 1.0;
}

}

QXP1Parser::QXP1Parser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter, const std::shared_ptr<QXP1Header> &header)
  : QXPParser(input, painter, header)
  , m_header(header)
{
}

void QXP1Parser::adjust(double &pos, unsigned adjustment)
{
  pos += double(adjustment - 0x8000) / 0x10000;
}

bool QXP1Parser::parseDocument(const std::shared_ptr<librevenge::RVNGInputStream> &docStream, QXPCollector &)
{
  parseCharFormats(docStream);
  parseParagraphFormats(docStream);
  return true;
}

bool QXP1Parser::parsePages(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXPCollector &collector)
{
  Page page;
  page.pageSettings.resize(1);
  page.pageSettings[0].offset.bottom = m_header->pageHeight();
  page.pageSettings[0].offset.right = m_header->pageWidth();

  for (unsigned i = 0; i != m_header->pages(); ++i)
  {
    const bool empty = parsePage(stream);
    collector.startPage(page);
    bool last = !empty;
    while (!last)
    {
      last = parseObject(stream, collector);
    }
    collector.endPage();
  }

  return false;
}

CharFormat QXP1Parser::parseCharFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  CharFormat result;

  skip(stream, 2);

  const int fontIndex = readS16(stream, true);
  (void) fontIndex;
  result.fontName = "Helvetica"; // TODO: How to determine font name? Is the list of fonts fixed?

  result.fontSize = readU16(stream, true) / 4.0;

  const unsigned flags = readU16(stream, true);
  convertCharFormatFlags(flags, result);

  skip(stream, 2);

  const unsigned colorId = readU8(stream);
  const unsigned shadeId = readU8(stream);
  result.color = getColor(colorId).applyShade(getShade(shadeId));

  return result;
}

ParagraphFormat QXP1Parser::parseParagraphFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  ParagraphFormat result;

  skip(stream, 3);
  result.alignment = readHorAlign(stream);
  skip(stream, 2);
  result.margin.left = readFraction(stream, true);
  result.firstLineIndent = readFraction(stream, true);
  result.margin.right = readFraction(stream, true);
  result.leading = readFraction(stream, true);
  result.margin.top = readFraction(stream, true);
  result.margin.bottom = readFraction(stream, true);

  for (unsigned i = 0; i < 20; ++i)
  {
    TabStop tabStop;

    const uint8_t type = readU8(stream);
    tabStop.type = convertTabStopType(type);
    tabStop.fillChar.append(char(readU8(stream)));
    tabStop.position = readFraction(stream, true);

    if (tabStop.isDefined())
      result.tabStops.push_back(tabStop);
  }

  return result;
}

std::shared_ptr<HJ> QXP1Parser::parseHJ(const std::shared_ptr<librevenge::RVNGInputStream> &)
{
  return std::shared_ptr<HJ>();
}

bool QXP1Parser::parsePage(const std::shared_ptr<librevenge::RVNGInputStream> &input)
{
  skip(input, 15);
  const unsigned empty = readU8(input);
  switch (empty)
  {
  case 1:
    return false;
  case 2:
    return true;
  default:
    QXP_DEBUG_MSG(("QXP1Parser::parsePage: unknown 'is empty' value %d, cannot continue\n", empty));
    throw ParseError();
  }
}

bool QXP1Parser::parseObject(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector)
{
  const unsigned type = readU8(input);

  bool transparent = false;
  const unsigned transVal = readU8(input);
  switch (transVal)
  {
  default:
    QXP_DEBUG_MSG(("QXP1Parser::parseObject: unexpected 'is transparent' value: %d\n", transVal));
    QXP_FALLTHROUGH;
  case 1:
    transparent = true;
    break;
  case 0:
    transparent = false;
    break;
  }

  unsigned contentIndex = readU16(input);
  skip(input, 2);

  Rect bbox;
  parseCoordPair(input, bbox.left, bbox.top, bbox.right, bbox.bottom);

  const unsigned textOffset = readU32(input, true) >> 8;
  skip(input, 8);
  const unsigned linkId = readU32(input, true);
  const unsigned shadeId = readU8(input);
  const unsigned colorId = readU8(input);
  const auto &color = getColor(colorId).applyShade(getShade(shadeId));

  switch (type)
  {
  case 0:
  case 1:
    parseLine(input, collector, bbox, color, transparent);
    break;
  case 3:
    parseText(input, collector, bbox, color, transparent, contentIndex, textOffset, linkId);
    break;
  case 4:
  case 5:
  case 6:
    parsePicture(input, collector, bbox, color, transparent);
    break;
  default:
    QXP_DEBUG_MSG(("QXP1Parser::parseObject: unknown object type %d, cannot continue\n", type));
    throw ParseError();
  }

  const unsigned lastObject = readU8(input);
  switch (lastObject)
  {
  case 0:
  case 1:
    return false;
  case 2:
    return true;
  default:
    QXP_DEBUG_MSG(("QXP1Parser::parseObject: unknown 'last object' value %d, cannot continue\n", lastObject));
    throw ParseError();
  }
}

void QXP1Parser::parseLine(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector, const Rect &bbox, const Color &color, bool transparent)
{
  (void) collector;
  (void) bbox;
  (void) color;
  (void) transparent;

  skip(input, 25);
}

void QXP1Parser::parseText(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector, const Rect &bbox, const Color &color, bool transparent, unsigned content, unsigned textOffset, unsigned linkID)
{
  (void) collector;
  (void) bbox;
  (void) color;
  (void) transparent;
  (void) content;
  (void) textOffset;
  (void) linkID;

  skip(input, 40);
}

void QXP1Parser::parsePicture(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector, const Rect &bbox, const Color &color, bool transparent)
{
  (void) collector;
  (void) bbox;
  (void) color;
  (void) transparent;

  skip(input, 54);
}

void QXP1Parser::parseCoordPair(const std::shared_ptr<librevenge::RVNGInputStream> &input, double &x1, double &y1, double &x2, double &y2)
{
  y1 = readU16(input, true);
  x1 = readU16(input, true);
  y2 = readU16(input, true);
  x2 = readU16(input, true);
  const unsigned y1Adj = readU16(input, true);
  const unsigned x1Adj = readU16(input, true);
  const unsigned y2Adj = readU16(input, true);
  const unsigned x2Adj = readU16(input, true);
  QXP1Parser::adjust(y1, y1Adj);
  QXP1Parser::adjust(x1, x1Adj);
  QXP1Parser::adjust(y2, y2Adj);
  QXP1Parser::adjust(x2, x2Adj);
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
