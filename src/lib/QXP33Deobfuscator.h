/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXP33DEOBFUSCATOR_H_INCLUDED
#define QXP33DEOBFUSCATOR_H_INCLUDED

#include "libqxp_utils.h"
#include "QXPDeobfuscator.h"

namespace libqxp
{

class QXP33Deobfuscator : public QXPDeobfuscator
{
public:
  QXP33Deobfuscator(uint16_t seed, uint16_t increment);

  void next();

private:
  uint16_t m_increment;
};

}

#endif // QXP33DEOBFUSCATOR_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
