/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXP4Deobfuscator.h"

namespace libqxp
{

namespace
{

uint16_t fill(uint16_t value, uint16_t shift, uint16_t mask)
{
  uint16_t r = shift;
  uint16_t v = value;
  while ((v & 1) == 0 && r > 0)
  {
    v >>= 1;
    r--;
  }
  const uint16_t s = shift - r;
  const uint16_t m = (0xffff >> s) << s;
  return (value | m) & mask;
}

uint16_t shift(uint16_t value, uint16_t count)
{
  const uint16_t mask = 0xffff >> (16 - count);
  const uint16_t highinit = value & mask;
  const uint16_t high = fill(highinit | (value >> 15), count, mask) << (16 - count);
  return high | (value >> count);
}

}

QXP4Deobfuscator::QXP4Deobfuscator(uint16_t seed, uint16_t increment)
  : QXPDeobfuscator(seed)
  , m_increment(increment)
{
}

void QXP4Deobfuscator::next(uint16_t block)
{
  m_seed += m_increment;
  m_increment = shift(m_increment, block & 0xf);
}

void QXP4Deobfuscator::nextRev()
{
  m_seed += 0xffff - m_increment;
}

void QXP4Deobfuscator::nextShift(uint16_t count)
{
  m_seed = shift(m_seed, count & 0xf);
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
