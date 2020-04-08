/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPBLOCKPARSER_H_INCLUDED
#define QXPBLOCKPARSER_H_INCLUDED

#include "libqxp_utils.h"

namespace libqxp
{

class QXPHeader;

class QXPBlockParser
{
  // disable copying
  QXPBlockParser(const QXPBlockParser &other) = delete;
  QXPBlockParser &operator=(const QXPBlockParser &other) = delete;

public:
  QXPBlockParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, const std::shared_ptr<QXPHeader> &header);

  std::shared_ptr<librevenge::RVNGInputStream> getBlock(const uint32_t index);
  std::shared_ptr<librevenge::RVNGInputStream> getChain(const uint32_t index);

private:
  const std::shared_ptr<librevenge::RVNGInputStream> m_input;
  const std::shared_ptr<QXPHeader> m_header;
  const bool be; // big endian

  const unsigned long m_length;
  const uint32_t m_blockLength;
  const uint32_t m_lastBlock;
};

}

#endif // QXPBLOCKPARSER_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
