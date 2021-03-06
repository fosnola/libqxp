/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXP33Deobfuscator.h"

namespace libqxp
{

QXP33Deobfuscator::QXP33Deobfuscator(uint16_t seed, uint16_t increment)
  : QXPDeobfuscator(seed)
  , m_increment(increment)
{
}

void QXP33Deobfuscator::next()
{
  m_seed += m_increment;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
