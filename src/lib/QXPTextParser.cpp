/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPTextParser.h"

#include "QXPHeader.h"
#include "QXPTypes.h"

namespace libqxp
{

using std::make_shared;

namespace
{

template <class Format, class FormatSpec>
void parseFormatSpec(
  const std::shared_ptr<librevenge::RVNGInputStream> &infoStream,
  const std::vector<std::shared_ptr<Format>> &formats,
  unsigned version, bool be,
  std::vector<FormatSpec> &textFormats)
{
  unsigned specLength = readU32(infoStream, be);
  if (specLength > getRemainingLength(infoStream))
    specLength = unsigned(getRemainingLength(infoStream));
  const long end = infoStream->tell() + specLength;
  while (infoStream->tell() < end)
  {
    const unsigned formatIndex = version >= QXP_4 ? readU32(infoStream, be) : readU16(infoStream, be);
    const unsigned length = readU32(infoStream, be);
    const unsigned startIndex = textFormats.empty() ? 0 : textFormats.back().afterEndIndex();

    std::shared_ptr<Format> format;
    if (formatIndex >= formats.size())
    {
      QXP_DEBUG_MSG(("Char format %u not found\n", formatIndex));
      format = formats.empty() ? std::make_shared<Format>() : formats[0];
    }
    else
    {
      format = formats[formatIndex];
    }
    textFormats.push_back(FormatSpec(format, startIndex, length));
  }
}

}

QXPTextParser::QXPTextParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, const std::shared_ptr<QXPHeader> &header)
  : m_header(header)
  , be(header->isBigEndian())
  , m_encoding(header->encoding())
  , m_blockParser(input, header)
{
}

std::shared_ptr<Text> QXPTextParser::parseText(unsigned index,
                                               const std::vector<std::shared_ptr<CharFormat>> &charFormats,
                                               const std::vector<std::shared_ptr<ParagraphFormat>> &paragraphFormats)
{
  auto infoStream = m_blockParser.getChain(index);

  auto text = make_shared<Text>();
  text->encoding = m_encoding;

  skip(infoStream, 4);

  {
    const unsigned blocksSpecLength = readU32(infoStream, be);
    const long end = infoStream->tell() + blocksSpecLength;
    while (infoStream->tell() < end)
    {
      const unsigned blockIndex = readU32(infoStream, be);
      unsigned length = m_header->version() >= QXP_4 ? readU32(infoStream, be) : readU16(infoStream, be);
      auto blockStream = m_blockParser.getBlock(blockIndex);
      if (length > getRemainingLength(blockStream))
        length = getRemainingLength(blockStream);
      text->text += readString(blockStream, length);
    }
  }

  parseFormatSpec(infoStream, charFormats, m_header->version(), be, text->charFormats);
  parseFormatSpec(infoStream, paragraphFormats, m_header->version(), be, text->paragraphs);

  return text;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
