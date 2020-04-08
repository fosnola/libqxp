/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXP33Parser.h"

#include "QXP33Deobfuscator.h"
#include "QXP33Header.h"
#include "QXPCollector.h"
#include "QXPTypes.h"

namespace libqxp
{

using librevenge::RVNGInputStream;
using std::make_shared;
using std::vector;
using std::shared_ptr;

namespace
{

template<typename T>
shared_ptr<T> createBox(const QXP33Parser::ObjectHeader &header)
{
  auto box = make_shared<T>();
  box->boundingBox = header.boundingBox;
  box->runaround = header.runaround;
  box->boxType = header.boxType;
  box->fill = header.fill;
  box->cornerType = header.cornerType;
  box->cornerRadius = header.cornerRadius;
  box->rotation = header.rotation;
  return box;
}

template<typename T>
shared_ptr<T> createLine(const QXP33Parser::ObjectHeader &header)
{
  auto line = make_shared<T>();
  line->boundingBox = header.boundingBox;
  line->runaround = header.runaround;
  line->rotation = header.rotation;
  if (auto fill = header.fill.get_ptr())
  {
    if (auto color = boost::get<Color>(fill))
    {
      line->style.color = *color;
    }
    else
    {
      QXP_DEBUG_MSG(("Unsupported line fill type\n"));
    }
  }
  return line;
}

}

QXP33Parser::QXP33Parser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter, const std::shared_ptr<QXP33Header> &header)
  : QXPParser(input, painter, header)
  , m_header(header)
{
}

bool QXP33Parser::parseDocument(const std::shared_ptr<librevenge::RVNGInputStream> &docStream, QXPCollector &collector)
{
  collector.collectDocumentProperties(m_header->documentProperties());

  for (int i = 0; i < 4; ++i)
  {
    skipRecord(docStream);
  }

  parseFonts(docStream);

  if (m_header->version() == QXP_33)
    skipRecord(docStream);

  parseColors(docStream);

  skipRecord(docStream);
  skipRecord(docStream); // don't need stylesheets, everything is included in the current style

  parseHJs(docStream);

  skipRecord(docStream);

  parseCharFormats(docStream);

  parseParagraphFormats(docStream);

  skipRecord(docStream);

  return true;
}

bool QXP33Parser::parsePages(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXPCollector &collector)
{
  QXP33Deobfuscator deobfuscate(m_header->seed(), m_header->increment());
  QXPDummyCollector dummyCollector;

  for (unsigned ind = 0; ind < m_header->pagesCount() + m_header->masterPagesCount(); ++ind)
  {
    // don't output master pages, everything is included in normal pages
    QXPCollector &coll = ind < m_header->masterPagesCount() ? dummyCollector : collector;

    auto page = parsePage(stream);
    coll.startPage(page);

    for (unsigned i = 0; i < page.objectsCount; ++i)
    {

      parseObject(stream, deobfuscate, coll, page, i);
      deobfuscate.next();
    }

    m_groupObjects.clear();
    coll.endPage();
  }

  return true;
}

void QXP33Parser::parseColors(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  const unsigned end = readRecordEndOffset(stream);

  try
  {
    skip(stream, 1);
    unsigned count = readU8(stream);
    skip(stream, 32);
    for (unsigned i = 0; i < count; ++i)
    {
      unsigned id = readU8(stream);
      skip(stream, 1);
      Color color;
      color.red = readColorComp(stream);
      color.green = readColorComp(stream);
      color.blue = readColorComp(stream);
      m_colors[id] = color;
      skip(stream, 42);
      readName(stream); // skip
    }
  }
  catch (...)
  {
    QXP_DEBUG_MSG(("Failed to parse colors, offset %ld\n", stream->tell()));
  }

  seek(stream, end);
}

CharFormat QXP33Parser::parseCharFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  skip(stream, 2);

  CharFormat result;
  parseCommonCharFormatProps(stream, result);

  skip(stream, 4);

  const unsigned colorId = readU8(stream);

  skip(stream, 1);

  const double shade = readFraction(stream, be);
  result.color = getColor(colorId).applyShade(shade);

  skip(stream, 8);
  result.baselineShift = readFraction(stream, be);

  result.isControlChars = readU8(stream) != 0;

  skip(stream, 13);

  return result;
}

ParagraphFormat QXP33Parser::parseParagraphFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  ParagraphFormat result;

  skip(stream, 2);

  bool hasRuleAbove, hasRuleBelow;
  readParagraphFlags(stream, result.incrementalLeading, hasRuleAbove, hasRuleBelow);

  skip(stream, 2);
  result.alignment = readHorAlign(stream);

  skip(stream, 4);
  const unsigned hj = readU16(stream, be);
  if (hj < m_hjs.size())
    result.hj = m_hjs[hj];
  skip(stream, 2);

  result.margin.left = readFraction(stream, be);
  result.firstLineIndent = readFraction(stream, be);
  result.margin.right = readFraction(stream, be);
  result.leading = readFraction(stream, be);
  result.margin.top = readFraction(stream, be);
  result.margin.bottom = readFraction(stream, be);

  auto ruleAbove = readParagraphRule(stream);
  auto ruleBelow = readParagraphRule(stream);
  if (hasRuleAbove)
    result.ruleAbove = ruleAbove;
  if (hasRuleBelow)
    result.ruleBelow = ruleBelow;

  skip(stream, 8);

  for (unsigned i = 0; i < 20; ++i)
  {
    const auto tabStop = parseTabStop(stream);
    if (tabStop.isDefined())
    {
      result.tabStops.push_back(tabStop);
    }
  }

  skip(stream, 6);

  return result;
}

std::shared_ptr<HJ> QXP33Parser::parseHJ(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  auto hj = make_shared<HJ>();

  skip(stream, 4);
  parseHJProps(stream, *hj);
  readName(stream);

  return hj;
}

Page QXP33Parser::parsePage(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  Page page;
  page.pageSettings = parsePageSettings(stream);
  page.objectsCount = readU32(stream, be);
  return page;
}

void QXP33Parser::parseObject(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXP33Deobfuscator &deobfuscate, QXPCollector &collector, const Page &page, const unsigned index)
{
  const auto header = parseObjectHeader(stream, deobfuscate);

  switch (header.contentType)
  {
  case ContentType::NONE:
    switch (header.shapeType)
    {
    case ShapeType::LINE:
    case ShapeType::ORTHOGONAL_LINE:
      parseLine(stream, header, collector);
      break;
    case ShapeType::RECTANGLE:
    case ShapeType::CORNERED_RECTANGLE:
    case ShapeType::OVAL:
    case ShapeType::POLYGON:
      parseEmptyBox(stream, header, collector);
      break;
    default:
      QXP_DEBUG_MSG(("Unsupported shape\n"));
      throw GenericException();
    }
    break;
  case ContentType::PICTURE:
    parsePictureBox(stream, header, collector);
    break;
  case ContentType::TEXT:
    parseTextBox(stream, header, collector);
    break;
  case ContentType::OBJECTS:
    parseGroup(stream, header, collector, page, index);
    break;
  default:
    QXP_DEBUG_MSG(("Unsupported content\n"));
    throw GenericException();
  }
}

QXP33Parser::ObjectHeader QXP33Parser::parseObjectHeader(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXP33Deobfuscator &deobfuscate)
{
  ObjectHeader result;

  const unsigned objectType = deobfuscate(readU8(stream, be));
  if (m_header->version() < QXP_33)
  {
    switch (objectType)
    {
    case 0:
      result.shapeType = ShapeType::LINE;
      result.contentType = ContentType::NONE;
      break;
    case 1:
      result.shapeType = ShapeType::ORTHOGONAL_LINE;
      result.contentType = ContentType::NONE;
      break;
    case 3:
      result.shapeType = ShapeType::RECTANGLE;
      result.contentType = ContentType::TEXT;
      break;
    case 11:
      result.shapeType = ShapeType::RECTANGLE;
      result.contentType = ContentType::OBJECTS;
      break;
    case 12:
      result.shapeType = ShapeType::RECTANGLE;
      result.contentType = ContentType::PICTURE;
      break;
    case 13:
      result.shapeType = ShapeType::CORNERED_RECTANGLE;
      result.contentType = ContentType::PICTURE;
      break;
    case 14:
      result.shapeType = ShapeType::OVAL;
      result.contentType = ContentType::PICTURE;
      break;
    case 15:
      result.shapeType = ShapeType::POLYGON;
      result.contentType = ContentType::PICTURE;
      break;
    default:
      QXP_DEBUG_MSG(("Unknown object type %u\n", objectType));
      throw ParseError();
    }
  }

  const unsigned colorId = readU8(stream);
  const double shade = readFraction(stream, be);
  const auto color = getColor(colorId).applyShade(shade);

  result.contentIndex = deobfuscate(uint16_t(readU32(stream, be) & 0xffff));

  bool noColor;
  bool noRunaround;
  readObjectFlags(stream, noColor, noRunaround);
  if (!noColor)
  {
    result.fill = color;
  }
  result.runaround = !noRunaround;

  skip(stream, 1);

  result.rotation = readFraction(stream, be);
  result.skew = readFraction(stream, be);

  result.linkId = readU32(stream, be);
  result.gradientId = readU32(stream, be);

  skip(stream, 4);

  const uint8_t boxFlag1 = readU8(stream);
  const uint8_t boxFlag2 = readU8(stream);
  bool beveled;
  bool concave;
  if (be)
  {
    result.hflip = boxFlag1 & 0x80;
    result.vflip = boxFlag2 & 0x80;
    beveled = boxFlag2 & 0x20;
    concave = boxFlag2 & 0x40;
  }
  else
  {
    result.hflip = boxFlag1 & 0x1;
    result.vflip = boxFlag2 & 0x1;
    beveled = boxFlag2 & 0x2;
    concave = boxFlag2 & 0x4;
  }

  if (m_header->version() == QXP_33)
  {
    const uint8_t contentType = readU8(stream);
    switch (contentType)
    {
    case 1:
      result.contentType = ContentType::OBJECTS;
      break;
    case 2:
    case 4:
      result.contentType = ContentType::NONE;
      break;
    case 3:
      result.contentType = ContentType::TEXT;
      break;
    case 5:
      result.contentType = ContentType::PICTURE;
      break;
    default:
      QXP_DEBUG_MSG(("Unknown content type %u\n", contentType));
      throw ParseError();
    }

    const uint8_t shapeType = readU8(stream);
    switch (shapeType)
    {
    case 0:
      result.shapeType = ShapeType::LINE;
      break;
    case 1:
      result.shapeType = ShapeType::ORTHOGONAL_LINE;
      break;
    case 2:
      result.shapeType = ShapeType::RECTANGLE;
      break;
    case 3:
      result.shapeType = ShapeType::CORNERED_RECTANGLE;
      break;
    case 4:
      result.shapeType = ShapeType::OVAL;
      break;
    case 5:
      result.shapeType = ShapeType::POLYGON;
      break;
    default:
      QXP_DEBUG_MSG(("Unknown shape type %u\n", shapeType));
      throw ParseError();
    }
  }

  switch (result.shapeType)
  {
  case ShapeType::RECTANGLE:
  case ShapeType::CORNERED_RECTANGLE:
    result.boxType = BoxType::RECTANGLE;
    break;
  case ShapeType::OVAL:
    result.boxType = BoxType::OVAL;
    break;
  case ShapeType::POLYGON:
    result.boxType = BoxType::POLYGON;
    break;
  default:
    break;
  }

  if (result.shapeType == ShapeType::CORNERED_RECTANGLE)
  {
    if (concave)
      result.cornerType = CornerType::CONCAVE;
    else if (beveled)
      result.cornerType = CornerType::BEVELED;
    else
      result.cornerType = CornerType::ROUNDED;
  }

  if (m_header->version() == QXP_33)
    result.cornerRadius = readFraction(stream, be);

  if (result.gradientId != 0)
  {
    result.fill = readGradient(stream, color);
  }

  result.boundingBox = readObjectBBox(stream);

  return result;
}

void QXP33Parser::readObjectFlags(const std::shared_ptr<librevenge::RVNGInputStream> &stream, bool &noColor, bool &noRunaround)
{
  const uint8_t flags = QXPParser::readObjectFlags(stream, noColor);
  if (be)
  {
    noRunaround = flags & 0x02;
  }
  else
  {
    noRunaround = flags & 0x20;
  }
}

void QXP33Parser::parseLine(const std::shared_ptr<librevenge::RVNGInputStream> &stream, const QXP33Parser::ObjectHeader &header, QXPCollector &collector)
{
  auto line = createLine<Line>(header);

  line->style.width = readFraction(stream, be);

  const unsigned styleIndex = readU8(stream);
  const bool isStripe = (styleIndex >> 7) == 1;
  if (!isStripe)
  {
    line->style.lineStyle = getLineStyle(styleIndex);
  }

  const uint8_t arrowType = readU8(stream);
  setArrow(arrowType, line->style);

  collector.collectLine(line);
}

void QXP33Parser::parseTextBox(const std::shared_ptr<librevenge::RVNGInputStream> &stream, const QXP33Parser::ObjectHeader &header, QXPCollector &collector)
{
  auto textbox = createBox<TextBox>(header);
  textbox->linkSettings.linkId = header.linkId;

  textbox->frame = readFrame(stream);
  skip(stream, 4);
  const unsigned runaroundId = readU32(stream, be);

  textbox->linkSettings.offsetIntoText = readU32(stream, be);
  skip(stream, 4);
  textbox->settings.gutterWidth = readFraction(stream, be);
  textbox->settings.inset.top = readFraction(stream, be);
  textbox->settings.inset.left = readFraction(stream, be);
  textbox->settings.inset.right = readFraction(stream, be);
  textbox->settings.inset.bottom = readFraction(stream, be);
  textbox->settings.rotation = readFraction(stream, be);
  textbox->settings.skew = readFraction(stream, be);
  textbox->settings.columnsCount = readU8(stream);
  textbox->settings.verticalAlignment = readVertAlign(stream);
  skip(stream, 8);
  textbox->linkSettings.nextLinkedIndex = readU32(stream, be);
  skip(stream, 8);

  if (header.shapeType == ShapeType::POLYGON)
  {
    textbox->customPoints = readPolygonData(stream);
  }

  if (header.contentIndex == 0 || textbox->linkSettings.offsetIntoText == 0)
  {
    skip(stream, 4);
    const unsigned fileInfoId = readU32(stream, be);
    skip(stream, 4);
    if (fileInfoId != 0)
    {
      skipFileInfo(stream);
    }
    if (header.contentIndex == 0)
    {
      skip(stream, 12);
    }
  }

  if (runaroundId != 0)
  {
    const unsigned length = readU32(stream, be);
    skip(stream, length);
  }

  if (header.contentIndex == 0)
  {
    collector.collectBox(textbox);
  }
  else
  {
    if (textbox->linkSettings.offsetIntoText > 0)
    {
      textbox->linkSettings.linkedIndex = header.contentIndex;
    }
    else
    {
      textbox->text = parseText(header.contentIndex, header.linkId, collector);
    }

    collector.collectTextBox(textbox);
  }
}

void QXP33Parser::parsePictureBox(const std::shared_ptr<librevenge::RVNGInputStream> &stream, const QXP33Parser::ObjectHeader &objHeader, QXPCollector &collector)
{
  const auto frame = readFrame(stream);

  skip(stream, 4);

  unsigned runaroundId = 0;
  unsigned clipId = 0;

  QXP33Parser::ObjectHeader header(objHeader);

  if (m_header->version() == QXP_33)
  {
    runaroundId = readU32(stream, be);
    skip(stream, 2);
    clipId = readU32(stream, be);
    skip(stream, 14);
  }
  else
  {
    skip(stream, 4);
    if (header.shapeType == ShapeType::CORNERED_RECTANGLE)
    {
      header.cornerRadius = readFraction(stream, be);
      const unsigned cornerType = readU8(stream, be);
      switch (cornerType)
      {
      case 0:
        header.cornerType = CornerType::BEVELED;
        break;
      default:
        QXP_DEBUG_MSG(("Unknown corner type %u\n", cornerType));
        QXP_FALLTHROUGH; // pick a default
      case 1:
        header.cornerType = CornerType::ROUNDED;
        break;
      case 2:
        header.cornerType = CornerType::CONCAVE;
        break;
      }
    }
    else if (header.shapeType == ShapeType::POLYGON)
    {
      skip(stream, 5);
    }
  }

  auto picturebox = createBox<PictureBox>(header);
  picturebox->frame = frame;

  picturebox->pictureRotation = readFraction(stream, be);
  picturebox->pictureSkew = readFraction(stream, be);
  picturebox->offsetLeft = readFraction(stream, be);
  picturebox->offsetTop = readFraction(stream, be);
  picturebox->scaleHor = readFraction(stream, be);
  picturebox->scaleVert = readFraction(stream, be);
  skip(stream, 30);

  if (header.shapeType == ShapeType::POLYGON)
  {
    picturebox->customPoints = readPolygonData(stream);
  }

  if (runaroundId != 0)
  {
    const unsigned rlength = readU32(stream, be);
    skip(stream, rlength);

    if (clipId != 0)
    {
      const unsigned clength = readU32(stream, be);
      skip(stream, clength);
    }
  }

  collector.collectBox(picturebox);
}

void QXP33Parser::parseEmptyBox(const std::shared_ptr<librevenge::RVNGInputStream> &stream, const QXP33Parser::ObjectHeader &header, QXPCollector &collector)
{
  auto box = createBox<Box>(header);

  box->frame = readFrame(stream);
  skip(stream, 4);
  const unsigned runaroundId = readU32(stream, be);

  skip(stream, 74);

  if (header.shapeType == ShapeType::POLYGON)
  {
    box->customPoints = readPolygonData(stream);
  }

  if (runaroundId != 0)
  {
    const unsigned length = readU32(stream, be);
    skip(stream, length);
  }

  collector.collectBox(box);
}

void QXP33Parser::parseGroup(const std::shared_ptr<librevenge::RVNGInputStream> &stream, const QXP33Parser::ObjectHeader &header, QXPCollector &collector, const Page &page, const unsigned index)
{
  skip(stream, 10);
  const unsigned count = readU16(stream, be);
  if (count > page.objectsCount - 1)
  {
    QXP_DEBUG_MSG(("Invalid group elements count %u\n", count));
    throw ParseError();
  }
  skip(stream, 6);

  auto group = make_shared<Group>();
  group->boundingBox = header.boundingBox;

  readGroupElements(stream, count, page.objectsCount, index, group->objectsIndexes);

  collector.collectGroup(group);
}

Frame QXP33Parser::readFrame(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  Frame frame;
  frame.width = readFraction(stream, be);
  const double shade = readFraction(stream, be);
  const unsigned colorId = readU8(stream);
  frame.color = getColor(colorId).applyShade(shade);
  skip(stream, 1);
  return frame;
}

std::vector<Point> QXP33Parser::readPolygonData(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  const unsigned length = readU32(stream, be);
  if (length < 18 || length > getRemainingLength(stream))
  {
    QXP_DEBUG_MSG(("Invalid polygon data length %u\n", length));
    throw ParseError();
  }
  skip(stream, 18);

  const unsigned count = (length - 18) / 8;
  std::vector<Point> points;
  points.resize(count);
  for (auto &p : points)
  {
    p = readYX(stream);
  }

  return points;
}

std::string QXP33Parser::readName(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  const long start = stream->tell();
  auto name = readPlatformString(stream, be);
  if ((stream->tell() - start) % 2 == 1)
  {
    skip(stream, 1);
  }
  return name;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
