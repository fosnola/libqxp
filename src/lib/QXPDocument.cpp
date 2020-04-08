/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cstring>
#include <memory>

#include <librevenge-stream/librevenge-stream.h>

#include <libqxp/libqxp.h>

#include "libqxp_utils.h"
#include "QXPDetector.h"
#include "QXPHeader.h"
#include "QXPParser.h"

using librevenge::RVNGInputStream;

using std::equal;
using std::shared_ptr;

namespace libqxp
{

QXPAPI bool QXPDocument::isSupported(librevenge::RVNGInputStream *const input, Type *const type) try
{
  QXPDetector detector;
  detector.detect(std::shared_ptr<librevenge::RVNGInputStream>(input, QXPDummyDeleter()));
  if (type)
    *type = detector.type();
  return detector.isSupported();
}
catch (...)
{
  return false;
}

QXPAPI QXPDocument::Result QXPDocument::parse(librevenge::RVNGInputStream *const input, librevenge::RVNGDrawingInterface *const document, QXPPathResolver * /*resolver*/) try
{
  QXPDetector detector;
  detector.detect(std::shared_ptr<librevenge::RVNGInputStream>(input, QXPDummyDeleter()));
  if (!detector.isSupported())
    return RESULT_UNSUPPORTED_FORMAT;

  if (detector.type() != QXPDocument::TYPE_DOCUMENT && detector.type() != QXPDocument::TYPE_TEMPLATE)
    return QXPDocument::RESULT_UNSUPPORTED_FORMAT;

  auto parser = detector.header()->createParser(detector.input(), document);

  return parser->parse() ? RESULT_OK : RESULT_UNKNOWN_ERROR;
}
catch (const FileAccessError &)
{
  return RESULT_FILE_ACCESS_ERROR;
}
catch (const UnsupportedFormat &)
{
  return RESULT_UNSUPPORTED_FORMAT;
}
catch (...)
{
  return RESULT_UNKNOWN_ERROR;
}

} // namespace libqxp

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
