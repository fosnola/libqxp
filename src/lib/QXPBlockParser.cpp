/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPBlockParser.h"

#include <librevenge-stream/librevenge-stream.h>
#include <algorithm>
#include <cassert>
#include <memory>
#include <set>
#include <vector>
#include <iterator>

#include "QXPHeader.h"
#include "QXPMemoryStream.h"
#include "libqxp_utils.h"

namespace libqxp
{

using librevenge::RVNGInputStream;
using std::make_shared;
using std::vector;

namespace
{

unsigned long getLength(RVNGInputStream *const input)
{
  assert(input);
  const unsigned long pos = input->tell();
  seek(input, 0);
  const unsigned long len = getRemainingLength(input);
  seek(input, pos);
  return len;
}

}

QXPBlockParser::QXPBlockParser(const std::shared_ptr<RVNGInputStream> &input, const std::shared_ptr<QXPHeader> &header)
  : m_input(input)
  , m_header(header)
  , be(header->isBigEndian())
  , m_length(getLength(m_input.get()))
  , m_blockLength(256)
  , m_lastBlock(m_length > 0 ? m_length / m_blockLength + 1 : 0)
{
}

std::shared_ptr<RVNGInputStream> QXPBlockParser::getBlock(const uint32_t index)
{
  if (index > 0 && index <= m_lastBlock)
  {
    seek(m_input, (index - 1) * m_blockLength);
    unsigned long bytes = 0;
    auto block = m_input->read(m_blockLength, bytes);
    if (bytes > 0)
      return make_shared<QXPMemoryStream>(block, bytes);
  }
  return nullptr;
}

std::shared_ptr<RVNGInputStream> QXPBlockParser::getChain(const uint32_t index)
{
  bool bigIdx = m_header->version() >= QXPVersion::QXP_31_MAC;

  vector<unsigned char> chain;
  bool isBig = false;
  uint32_t next = index;
  try
  {
    std::set<uint32_t> visited;

    while (next > 0 && next <= m_lastBlock)
    {
      seek(m_input, (next - 1) * m_blockLength);
      uint16_t count = isBig ? readU16(m_input, be) : 1;
      if (count > m_lastBlock - next)
        count = m_lastBlock - next;
      bool stop = false;

      // Cycle/overlap detection
      // If this is a big block, we read data up to the previously read
      // block and only then stop.
      for (uint32_t i = next - 1; i < next - 1 + count; ++i)
      {
        stop = !visited.insert(i).second;
        if (stop)
          count = uint16_t(i - next - 1);
      }
      if (count == 0)
        break;

      uint32_t len = (next - 1 + count) * m_blockLength - (bigIdx ? 4 : 2) - m_input->tell();
      unsigned long bytes = 0;
      auto block = m_input->read(len, bytes);
      if (bool(block) && bytes > 0)
        std::copy(block, block + bytes, std::back_inserter(chain));

      if (stop || bytes < len) // A cycle was detected or we're at the end already
        break;

      const int32_t nextVal = bigIdx ? readS32(m_input, be) : readS16(m_input, be);
      isBig = nextVal < 0;
      next = abs(nextVal);
    }
  }
  catch (...)
  {
    // Just retrieve what's possible
  }
  return make_shared<QXPMemoryStream>(chain.data(), chain.size());
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
