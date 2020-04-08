/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXP1HEADER_H_INCLUDED
#define QXP1HEADER_H_INCLUDED

#include "QXPHeader.h"

namespace libqxp
{

class QXP1Header : public QXPHeader, public std::enable_shared_from_this<QXP1Header>
{
public:
  QXP1Header();

  bool load(const std::shared_ptr<librevenge::RVNGInputStream> &input) override;

  QXPDocument::Type getType() const override;

  std::unique_ptr<QXPParser> createParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter) override;

  unsigned pages() const
  {
    return m_pages;
  }

  double pageHeight() const
  {
    return m_pageHeight;
  }

  double pageWidth() const
  {
    return m_pageWidth;
  }

private:
  unsigned m_pages;
  double m_pageHeight;
  double m_pageWidth;
};

}

#endif // QXP1HEADER_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
