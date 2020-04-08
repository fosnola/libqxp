/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPHeader.h"

#include "libqxp_utils.h"

namespace libqxp
{

QXPHeader::QXPHeader(const boost::optional<QXPDocument::Type> &fileType)
  : m_proc(0)
  , m_version(QXPVersion::UNKNOWN)
  , m_language(0)
  , m_fileType(fileType)
{
}

bool QXPHeader::isLittleEndian() const
{
  return m_proc == 'I';
}

bool QXPHeader::isBigEndian() const
{
  return !isLittleEndian();
}

unsigned QXPHeader::version() const
{
  return m_version;
}

const char *QXPHeader::encoding() const
{
  switch (m_language)
  {
  default:
    QXP_DEBUG_MSG(("Unknown language %u\n", m_language));
    QXP_FALLTHROUGH; // pick a default
  case 0x33:
    return isLittleEndian() ? "cp1252" : "macroman";
  }
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
