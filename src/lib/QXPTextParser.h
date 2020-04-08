/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPTEXTPARSER_H_INCLUDED
#define QXPTEXTPARSER_H_INCLUDED

#include <memory>
#include <vector>

#include "libqxp_utils.h"
#include "QXPBlockParser.h"

namespace libqxp
{

class QXPHeader;

struct CharFormat;
struct ParagraphFormat;
struct Text;

class QXPTextParser
{
  // disable copying
  QXPTextParser(const QXPTextParser &other) = delete;
  QXPTextParser &operator=(const QXPTextParser &other) = delete;

public:
  QXPTextParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, const std::shared_ptr<QXPHeader> &header);

  std::shared_ptr<Text> parseText(unsigned index,
                                  const std::vector<std::shared_ptr<CharFormat>> &charFormats,
                                  const std::vector<std::shared_ptr<ParagraphFormat>> &paragraphFormats);

private:
  const std::shared_ptr<QXPHeader> m_header;
  const bool be; // big endian
  const char *m_encoding;
  QXPBlockParser m_blockParser;
};

}

#endif // QXPTEXTPARSER_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
