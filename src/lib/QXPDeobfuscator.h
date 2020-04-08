/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPDEOBFUSCATOR_H_INCLUDED
#define QXPDEOBFUSCATOR_H_INCLUDED

#include "libqxp_utils.h"

namespace libqxp
{

class QXPDeobfuscator
{
public:
  uint16_t operator()(uint16_t value) const;
  uint8_t operator()(uint8_t value) const;

protected:
  uint16_t m_seed;

  explicit QXPDeobfuscator(uint16_t seed);
};

}

#endif // QXPDEOBFUSCATOR_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
