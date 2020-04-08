/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPMACSTREAM_H_INCLUDED
#define QXPMACSTREAM_H_INCLUDED

#include <memory>
#include <string>

#include <librevenge-stream/librevenge-stream.h>

namespace libqxp
{

class QXPMacFileParser
{
public:
  QXPMacFileParser(std::shared_ptr<librevenge::RVNGInputStream> &dataFork, std::string &type, std::string &creator);

  bool parse(const std::shared_ptr<librevenge::RVNGInputStream> &input);

private:
  std::shared_ptr<librevenge::RVNGInputStream> &m_dataFork;
  std::string &m_type;
  std::string &m_creator;
};

}

#endif

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
