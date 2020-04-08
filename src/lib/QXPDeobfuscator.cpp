/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPDeobfuscator.h"

namespace libqxp
{

QXPDeobfuscator::QXPDeobfuscator(uint16_t seed)
  : m_seed(seed)
{
}

uint16_t QXPDeobfuscator::operator()(uint16_t value) const
{
  return value + m_seed - ((value & m_seed) << 1);
}

uint8_t QXPDeobfuscator::operator()(uint8_t value) const
{
  return operator()(uint16_t(value)) & 0xff;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
