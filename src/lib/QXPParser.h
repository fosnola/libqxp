/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPPARSER_H_INCLUDED
#define QXPPARSER_H_INCLUDED

#include "libqxp_utils.h"
#include "QXPBlockParser.h"
#include "QXPTextParser.h"
#include "QXPTypes.h"

#include <deque>
#include <functional>
#include <map>
#include <set>
#include <vector>

namespace libqxp
{

class QXPCollector;
class QXPHeader;

class QXPParser
{
  // disable copying
  QXPParser(const QXPParser &other) = delete;
  QXPParser &operator=(const QXPParser &other) = delete;

public:
  QXPParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter, const std::shared_ptr<QXPHeader> &header);
  virtual ~QXPParser() = default;

  bool parse();

protected:
  const std::shared_ptr<librevenge::RVNGInputStream> m_input;
  librevenge::RVNGDrawingInterface *m_painter;
  const bool be; // big endian

  QXPBlockParser m_blockParser;
  QXPTextParser m_textParser;

  std::map<unsigned, Color> m_colors;
  std::map<int, std::string> m_fonts;
  std::vector<std::shared_ptr<CharFormat>> m_charFormats;
  std::vector<std::shared_ptr<ParagraphFormat>> m_paragraphFormats;
  std::map<unsigned, LineStyle> m_lineStyles;
  std::vector<Arrow> m_arrows;
  std::deque<std::shared_ptr<HJ>> m_hjs;

  std::set<unsigned> m_groupObjects;

  Color getColor(unsigned id, Color defaultColor = Color(0, 0, 0)) const;
  const LineStyle *getLineStyle(unsigned id) const;
  std::string getFont(int id, std::string defaultFont = "Arial") const;

  void convertCharFormatFlags(unsigned flags, CharFormat &format);
  TabStopType convertTabStopType(unsigned type);

  virtual bool parseDocument(const std::shared_ptr<librevenge::RVNGInputStream> &docStream, QXPCollector &collector) = 0;
  virtual bool parsePages(const std::shared_ptr<librevenge::RVNGInputStream> &stream, QXPCollector &collector) = 0;

  void skipRecord(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  void parseFonts(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  void parseHJs(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  void parseCharFormats(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  void parseCommonCharFormatProps(const std::shared_ptr<librevenge::RVNGInputStream> &stream, CharFormat &result);
  void parseHJProps(const std::shared_ptr<librevenge::RVNGInputStream> &stream, HJ &result);
  TabStop parseTabStop(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  void parseParagraphFormats(const std::shared_ptr<librevenge::RVNGInputStream> &stream);

  virtual CharFormat parseCharFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream) = 0;
  virtual ParagraphFormat parseParagraphFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream) = 0;
  virtual std::shared_ptr<HJ> parseHJ(const std::shared_ptr<librevenge::RVNGInputStream> &stream) = 0;

  void parseCollection(const std::shared_ptr<librevenge::RVNGInputStream>stream, std::function<void()> itemHandler);

  std::vector<PageSettings> parsePageSettings(const std::shared_ptr<librevenge::RVNGInputStream> &stream);

  std::shared_ptr<Text> parseText(unsigned index, unsigned linkId, QXPCollector &collector);

  uint32_t readRecordEndOffset(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  uint8_t readColorComp(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  Rect readObjectBBox(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  Gradient readGradient(const std::shared_ptr<librevenge::RVNGInputStream> &stream, const Color &color1);
  HorizontalAlignment readHorAlign(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  VerticalAlignment readVertAlign(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  Point readYX(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  std::shared_ptr<ParagraphRule> readParagraphRule(const std::shared_ptr<librevenge::RVNGInputStream> &stream);
  uint8_t readParagraphFlags(const std::shared_ptr<librevenge::RVNGInputStream> &stream, bool &incrementalLeading, bool &ruleAbove, bool &ruleBelow);
  uint8_t readObjectFlags(const std::shared_ptr<librevenge::RVNGInputStream> &stream, bool &noColor);
  void readGroupElements(const std::shared_ptr<librevenge::RVNGInputStream> &stream, unsigned count, unsigned objectsCount, unsigned index, std::vector<unsigned> &elements);
  void setArrow(const unsigned index, Frame &frame) const;
  void skipFileInfo(const std::shared_ptr<librevenge::RVNGInputStream> &stream);

private:
  const std::shared_ptr<QXPHeader> m_header;
};

}

#endif // QXPPARSER_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
