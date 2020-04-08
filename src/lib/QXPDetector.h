/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPDETECTOR_H_INCLUDED
#define QXPDETECTOR_H_INCLUDED

#include <memory>

#include <librevenge-stream/librevenge-stream.h>

#include <libqxp/libqxp.h>

namespace libqxp
{

class QXPHeader;

class QXPDetector
{
public:
  QXPDetector();
  ~QXPDetector() = default;

  void detect(const std::shared_ptr<librevenge::RVNGInputStream> &input);

  const std::shared_ptr<librevenge::RVNGInputStream> &input() const;
  const std::shared_ptr<QXPHeader> &header() const;
  bool isSupported() const;
  QXPDocument::Type type() const;

private:
  std::shared_ptr<librevenge::RVNGInputStream> m_input;
  std::shared_ptr<QXPHeader> m_header;
  QXPDocument::Type m_type;
  bool m_supported;
};

}

#endif // QXPDETECTOR_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
