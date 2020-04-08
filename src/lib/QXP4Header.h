/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXP4HEADER_H_INCLUDED
#define QXP4HEADER_H_INCLUDED

#include <string>

#include "libqxp_utils.h"
#include "QXP3HeaderBase.h"
#include "QXPTypes.h"

namespace libqxp
{

class QXP4Header : public QXP3HeaderBase, public std::enable_shared_from_this<QXP4Header>
{
public:
  explicit QXP4Header(const boost::optional<QXPDocument::Type> &fileType = boost::none);

  bool load(const std::shared_ptr<librevenge::RVNGInputStream> &input) override;

  QXPDocument::Type getType() const override;

  std::unique_ptr<QXPParser> createParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter) override;

  uint16_t pagesCount() const;
  uint16_t masterPagesCount() const;
  uint16_t seed() const;
  uint16_t increment() const;
  const QXPDocumentProperties &documentProperties() const;

private:
  std::string m_type;
  uint16_t m_pagesCount;
  uint16_t m_masterPagesCount;
  uint16_t m_seed;
  uint16_t m_increment;
  QXPDocumentProperties m_documentProperties;
};

}

#endif // QXP4HEADER_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
