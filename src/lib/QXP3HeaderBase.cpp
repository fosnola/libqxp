/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXP3HeaderBase.h"

#include "libqxp_utils.h"

namespace libqxp
{

QXP3HeaderBase::QXP3HeaderBase(const boost::optional<QXPDocument::Type> &fileType)
  : QXPHeader(fileType)
  , m_signature()
{
}

bool QXP3HeaderBase::load(const std::shared_ptr<librevenge::RVNGInputStream> &input)
{
  seek(input, 2);

  m_proc = readU8(input);
  skip(input, 1);
  m_signature = readString(input, 3);

  m_language = readU8(input);

  m_version = readU16(input, isBigEndian());

  return true;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
