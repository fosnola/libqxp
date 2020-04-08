/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

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

template<typename T>
shared_ptr<T> createBox(const QXP1Parser::ObjectHeader &header)
{
  auto box = make_shared<T>();
  box->boundingBox = header.boundingBox;
  box->boxType = header.boxType;
  box->fill = header.fill;
  return box;
}

template<typename T>
shared_ptr<T> createLine(const QXP1Parser::ObjectHeader &header)
{
  auto line = make_shared<T>();
  line->boundingBox = header.boundingBox;
  //line->runaround = header.runaround;
  //line->rotation = header.rotation;
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

QXP1Parser::QXP1Parser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter, const std::shared_ptr<QXP1Header> &header)
  : QXPParser(input, painter, header)
  , m_header(header)
{
}

void QXP1Parser::adjust(double &pos, unsigned adjustment)
{
  pos += double(adjustment - 0x8000) / 0x10000;
}

double QXP1Parser::getShade(const unsigned shadeId) const
{
  if (m_header->version()>=QXPVersion::QXP_2)
    return double(shadeId)/100;
  if (shadeId < 3)
    return 0.1 * shadeId;
  else if (shadeId < 6)
    return 0.2 * (shadeId - 1);
  else
    return 1.0;
}

bool QXP1Parser::parseDocument(const std::shared_ptr<librevenge::RVNGInputStream> &docStream, QXPCollector &)
{
  if (m_header->version()>=QXPVersion::QXP_2)
  {
    parseFonts(docStream);
    parseColors(docStream);
    skipRecord(docStream); // don't need stylesheets, everything is included in the current style
    parseHJs(docStream);
  }
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

  QXPDummyCollector dummyCollector;
  for (unsigned i = 0; i < 2+m_header->pages(); ++i)
  {
    // don't output master pages, everything is included in normal pages
    QXPCollector &coll = i < 2 ? dummyCollector : collector;

    const bool empty = parsePage(stream);
    coll.startPage(page);
    bool last = !empty;
    while (!last)
    {
      last = parseObject(stream, coll);
    }
    coll.endPage(false);
  }

  return true;
}

void QXP1Parser::parseColors(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
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
      skip(stream, 40);
      readName(stream); // skip
    }
  }
  catch (...)
  {
    QXP_DEBUG_MSG(("Failed to parse colors, offset %ld\n", stream->tell()));
  }

  seek(stream, end);
}

CharFormat QXP1Parser::parseCharFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  CharFormat result;

  skip(stream, 2);

  const unsigned fontIndex = readU16(stream, true);
  // see MWAWFontConverter[limwaw] to get more current names
  char const *(fontNames[]) = { nullptr /* system font*/, nullptr /* appli font */,
                                "NewYork", "Geneva", "Monaco", "Venice", "London", "Athens", "SanFran", "Toronto", // 2-9
                                nullptr, "Cairo", "LosAngeles", "Zapf Dingbats", "Bookman", // 10-14
                                nullptr, "Palatino", nullptr, "Zapf Chancery", nullptr, // 15-19
                                "Times", "Helvetica", "Courier", "Symbol", "Mobile"
                              }; // 20-24
  if (fontIndex<25 && fontNames[fontIndex])
    result.fontName = fontNames[fontIndex];
  else
    result.fontName = "Helvetica";

  result.fontSize = readU16(stream, true) / 4.0;

  const unsigned flags = readU16(stream, true);
  convertCharFormatFlags(flags, result);
  result.horizontalScaling=double(readU16(stream, true))/double(0x800);
  const unsigned colorId = readU8(stream);
  const unsigned shadeId = readU8(stream);
  result.color = getColor(colorId).applyShade(getShade(shadeId));
  result.baselineShift = -double(readU16(stream, true))/double(0x8000);

  return result;
}

std::shared_ptr<HJ> QXP1Parser::parseHJ(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  auto hj = make_shared<HJ>();

  skip(stream, 4);
  parseHJProps(stream, *hj);
  readName(stream);

  return hj;
}

ParagraphFormat QXP1Parser::parseParagraphFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  ParagraphFormat result;

  skip(stream, 3); // flag: keepline, break status...
  result.alignment = readHorAlign(stream);
  const unsigned hj = readU8(stream);
  if (hj < m_hjs.size())
    result.hj = m_hjs[hj];
  skip(stream, 1); // always 1?
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

bool QXP1Parser::parsePage(const std::shared_ptr<librevenge::RVNGInputStream> &input)
{
  // 0: num page
  // 2: format -1: none, 1:numeric, .. 5: alpha
  // 3: 0|80, 0, 4, 0, 0, 0, 0, 0, 1
  skip(input, 16);
  return true;
}

bool QXP1Parser::parseObject(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector)
{
  std::cout << std::hex << input->tell() << std::dec << "\n";
  ObjectHeader object;
  const unsigned type = readU8(input);
  if (m_header->version()>=QXPVersion::QXP_2)
  {
    switch (type)
    {
    case 0:
      object.shapeType = ShapeType::LINE;
      object.contentType = ContentType::NONE;
      break;
    case 1:
      object.shapeType = ShapeType::ORTHOGONAL_LINE;
      object.contentType = ContentType::NONE;
      break;
    case 3:
    case 0xfd: // main textbox
      object.shapeType = ShapeType::RECTANGLE;
      object.contentType = ContentType::TEXT;
      break;
    case 0xb:
      object.shapeType = ShapeType::RECTANGLE;
      object.contentType = ContentType::PICTURE;
      break;
    case 0xc:
      object.shapeType = ShapeType::CORNERED_RECTANGLE;
      object.contentType = ContentType::PICTURE;
      break;
    case 0xd:
      object.shapeType = ShapeType::OVAL;
      object.contentType = ContentType::PICTURE;
      break;
    default:
      QXP_DEBUG_MSG(("Unknown object type %u\n", type));
      throw ParseError();
    }
  }
  else
  {
    switch (type)
    {
    case 0:
      object.shapeType = ShapeType::LINE;
      object.contentType = ContentType::NONE;
      break;
    case 1:
      object.shapeType = ShapeType::ORTHOGONAL_LINE;
      object.contentType = ContentType::NONE;
      break;
    case 3:
    case 0xfd: // main textbox
      object.shapeType = ShapeType::RECTANGLE;
      object.contentType = ContentType::TEXT;
      break;
    case 4:
      object.shapeType = ShapeType::RECTANGLE;
      object.contentType = ContentType::PICTURE;
      break;
    case 5:
      object.shapeType = ShapeType::CORNERED_RECTANGLE;
      object.contentType = ContentType::PICTURE;
      break;
    case 6:
      object.shapeType = ShapeType::OVAL;
      object.contentType = ContentType::PICTURE;
      break;
    default:
      QXP_DEBUG_MSG(("Unknown object type %u\n", type));
      throw ParseError();
    }
  }
  const unsigned transVal = readU8(input);
  bool transparent = (transVal&1)==1;
  // |2: habillage

  object.contentIndex = readU16(input, true);
  skip(input, 2); // flags: |0x8000: locked

  parseCoordPair(input, object.boundingBox.left, object.boundingBox.top, object.boundingBox.right, object.boundingBox.bottom);

  object.textOffset = readU32(input, true) >> 8;
  skip(input, 8);
  object.linkIndex = readU32(input, true);
  const unsigned shadeId = readU8(input);
  const unsigned colorId = readU8(input);
  const auto &color = getColor(colorId).applyShade(getShade(shadeId));

  if (type<2 || !transparent)
  {
    object.fill = color;
  }

  unsigned lastObject;
  switch (object.shapeType)
  {
  case ShapeType::LINE:
  case ShapeType::ORTHOGONAL_LINE:
    parseLine(input, collector, object, lastObject);
    break;
  case ShapeType::RECTANGLE:
  case ShapeType::CORNERED_RECTANGLE:
  case ShapeType::OVAL:
    if (object.contentType == ContentType::TEXT)
      parseTextBox(input, collector, object, lastObject);
    else
      parsePictureBox(input, collector, object, lastObject);
    break;
  default:
    QXP_DEBUG_MSG(("QXP1Parser::parseObject: unknown object type %d, cannot continue\n", type));
    throw ParseError();
  }

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

void QXP1Parser::parseLine(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXPCollector &collector, QXP1Parser::ObjectHeader const &header, unsigned &lastObject)
{
  auto line = createLine<Line>(header);

  Rect &bbox = line->boundingBox;
  parseCoordPair(stream, bbox.left, bbox.top, bbox.right, bbox.bottom);
  skip(stream, 2); // 1?
  line->style.width = double(readU16(stream, true))/double(0x8000);
  const unsigned styleIndex = readU8(stream);
  const bool isStripe = (styleIndex >> 7) == 1;
  if (!isStripe)
  {
    line->style.lineStyle = getLineStyle(styleIndex);
  }

  const uint8_t arrowType = readU8(stream);
  setArrow(arrowType, line->style);
  collector.collectLine(line);

  skip(stream, 3);

  lastObject = readU8(stream);
}

void QXP1Parser::parseTextBox(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXPCollector &collector, QXP1Parser::ObjectHeader const &header, unsigned &lastObject)
{
  auto textbox = createBox<TextBox>(header);
  textbox->linkSettings.linkId = header.linkIndex;
  textbox->linkSettings.offsetIntoText = header.textOffset;
  textbox->frame = readFrame(stream);
  textbox->settings.columnsCount = readU8(stream);
  skip(stream, 5); // 0: column separator[4], 4: 0|40[1]
  textbox->settings.inset.top =
    textbox->settings.inset.left =
      textbox->settings.inset.right =
        textbox->settings.inset.bottom = double(readU32(stream, true))/double(0x1000000);
  skip(stream, 1); // 0: 0[1]
  textbox->linkSettings.nextLinkedIndex = readU16(stream, be);
  skip(stream, 4); // some id
  auto val=readU8(stream);
  if (val)
    skip(stream, 3);
  if (header.contentIndex==0)
    skip(stream, 12);
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
      textbox->text = parseText(header.contentIndex, header.linkIndex, collector);
    }

    collector.collectTextBox(textbox);
  }

  lastObject = readU8(stream);
}

void QXP1Parser::parsePictureBox(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXPCollector &collector, QXP1Parser::ObjectHeader const &header, unsigned &lastObject)
{
  auto picturebox = createBox<PictureBox>(header);
  picturebox->contentIndex=header.contentIndex;
  picturebox->frame = readFrame(stream);
  skip(stream, 5); // 0: column count, 1: column separator[4]
  picturebox->scaleHor = readFraction(stream, true);
  picturebox->scaleVert = readFraction(stream, true);
  skip(stream, 4); // 0
  auto index=readU32(stream, true);
  skip(stream, 17); // then 1,0,1,0,0x24,0,0
  if (m_header->version()>=QXPVersion::QXP_2)
    skip(stream, 10);
  lastObject = readU8(stream);

  if (index)
  {
    for (int i=0; i<2; ++i)
    {
      auto sz=readU16(stream, true);
      if (sz)
        skip(stream, sz);
    }
  }
  collector.collectPictureBox(picturebox);

  if (header.contentIndex)
    parsePicture(header.contentIndex, collector);
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

Frame QXP1Parser::readFrame(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  Frame frame;
  skip(stream, 1);
  frame.width = double(readU8(stream));
  skip(stream, 2);
  const unsigned shadeId = readU8(stream);
  const unsigned colorId = readU8(stream);
  frame.color = getColor(colorId).applyShade(getShade(shadeId));
  const unsigned styleIndex = readU8(stream);
  const bool isStripe = (styleIndex >> 7) == 1;
  if (!isStripe)
    frame.lineStyle = getLineStyle(styleIndex);
  else
    frame.width=0;
  return frame;
}

std::string QXP1Parser::readName(const std::shared_ptr<librevenge::RVNGInputStream> &stream)
{
  const long start = stream->tell();
  auto name = readPlatformString(stream, true);
  if ((stream->tell() - start) % 2 == 1)
  {
    skip(stream, 1);
  }
  return name;
}
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
