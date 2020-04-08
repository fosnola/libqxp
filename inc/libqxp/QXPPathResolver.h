/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBQXP_QXPPATHRESOLVER_H
#define INCLUDED_LIBQXP_QXPPATHRESOLVER_H

#include <librevenge-stream/librevenge-stream.h>

#include "libqxp_api.h"

namespace libqxp
{

/** Provide access to linked files.
  */
class QXPAPI QXPPathResolver
{
public:
  /** Path format.
    */
  enum PathFormat { PATH_FORMAT_MAC, PATH_FORMAT_WINDOWS };

  virtual ~QXPPathResolver() = default;

  /** Resolves a link's path and returns the content as a stream.
    *
    * The path is sent in "native" format.
    *
    * If the path cannot be resolved, nullptr is returned.
    *
    * @param[in] path the path
    * @param[in] format format of the path
    * @returns a stream representing the link's content or nullptr
    */
  virtual librevenge::RVNGInputStream *getStream(const char *path, PathFormat format) = 0;
};

} // namespace libqxp

#endif // INCLUDED_LIBQXP_QXPPATHRESOLVER_H

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
