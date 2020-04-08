/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXP1PARSER_H_INCLUDED
#define QXP1PARSER_H_INCLUDED

#include "QXPParser.h"

namespace libqxp
{

class QXP1Header;

class QXP1Parser : public QXPParser
{
public:
  QXP1Parser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter, const std::shared_ptr<QXP1Header> &header);

  static void adjust(double &pos, unsigned adjustment);

  enum class ShapeType
  {
    UNKNOWN,
    LINE,
    ORTHOGONAL_LINE,
    RECTANGLE,
    CORNERED_RECTANGLE,
    OVAL
  };

  struct ObjectHeader
  {
    boost::optional<Fill> fill;

    unsigned contentIndex;
    unsigned linkIndex;

    ContentType contentType;
    ShapeType shapeType;

    Rect boundingBox;

    BoxType boxType;
    unsigned textOffset;

    ObjectHeader()
      : fill(), contentIndex(0), linkIndex(0),
        contentType(ContentType::UNKNOWN), shapeType(ShapeType::UNKNOWN), boundingBox(),
        boxType(BoxType::UNKNOWN), textOffset(0)
    { }
  };
private:
  bool parseDocument(const std::shared_ptr<librevenge::RVNGInputStream> &docStream, QXPCollector &collector) override;
  bool parsePages(const std::shared_ptr<librevenge::RVNGInputStream> &pagesStream, QXPCollector &collector) override;

  CharFormat parseCharFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream) override;
  ParagraphFormat parseParagraphFormat(const std::shared_ptr<librevenge::RVNGInputStream> &stream) override;
  std::shared_ptr<HJ> parseHJ(const std::shared_ptr<librevenge::RVNGInputStream> &stream) override;

  bool parsePage(const std::shared_ptr<librevenge::RVNGInputStream> &input);
  bool parseObject(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector);

  void parseLine(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector, ObjectHeader const &header, unsigned &lastObject);
  void parsePictureBox(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector, ObjectHeader const &header, unsigned &lastObject);
  void parseTextBox(const std::shared_ptr<librevenge::RVNGInputStream> &input, QXPCollector &collector, ObjectHeader const &header, unsigned &lastObject);

  void parseCoordPair(const std::shared_ptr<librevenge::RVNGInputStream> &input, double &x1, double &y1, double &x2, double &y2);
  Frame readFrame(const std::shared_ptr<librevenge::RVNGInputStream> &stream);

private:
  const std::shared_ptr<QXP1Header> m_header;
};

}

#endif // QXP1PARSER_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
