/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPHEADER_H_INCLUDED
#define QXPHEADER_H_INCLUDED

#include <memory>

#include <boost/optional.hpp>

#include <libqxp/libqxp.h>

namespace libqxp
{

enum QXPVersion
{
  UNKNOWN = 0,
  QXP_1 = 0x20, // NOTE: this is 1.10L, but we use for all 1.x versions, as we've no idea if there are any differences anyway
  QXP_31_MAC = 0x39,
  QXP_31 = 0x3e,
  QXP_33 = 0x3f,
  QXP_4 = 0x41,
  QXP_5 = 0x42,
  QXP_6 = 0x43,
  QXP_7 = 0x44,
  QXP_8 = 0x45,
};

class QXPParser;

struct QXPDocumentProperties;

class QXPHeader
{
public:
  explicit QXPHeader(const boost::optional<QXPDocument::Type> &fileType = boost::none);
  virtual ~QXPHeader() = default;

  virtual bool load(const std::shared_ptr<librevenge::RVNGInputStream> &input) = 0;

  virtual QXPDocument::Type getType() const = 0;

  virtual std::unique_ptr<QXPParser> createParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter) = 0;

  bool isLittleEndian() const;
  bool isBigEndian() const;
  unsigned version() const;
  const char *encoding() const;

protected:
  unsigned m_proc;
  unsigned m_version;
  unsigned m_language;
  boost::optional<QXPDocument::Type> m_fileType;
};

}

#endif // QXPHEADER_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
