/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXP3HEADERBASE_H_INCLUDED
#define QXP3HEADERBASE_H_INCLUDED

#include <string>

#include "QXPHeader.h"

namespace libqxp
{

class QXP3HeaderBase : public QXPHeader
{
public:
  explicit QXP3HeaderBase(const boost::optional<QXPDocument::Type> &fileType = boost::none);

  bool load(const std::shared_ptr<librevenge::RVNGInputStream> &input) override;

protected:
  std::string m_signature;
};

}

#endif // QXP3HEADERBASE_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
