/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>

#include <librevenge/librevenge.h>
#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>

#include <libqxp/libqxp.h>

namespace
{

int printUsage()
{
  std::printf("`qxp2text' converts QuarkXPress documents to plain text.\n");
  std::printf("\n");
  std::printf("Usage: qxp2text [OPTION] FILE\n");
  std::printf("\n");
  std::printf("Options:\n");
  std::printf("\t--help                show this help message\n");
  std::printf("\t--version             print version and exit\n");
  std::printf("\n");
  std::printf("Report bugs to <http://bugs.documentfoundation.org/>.\n");
  return -1;
}

int printVersion()
{
  std::printf("qxp2text " VERSION "\n");
  return 0;
}

} // anonymous namespace

using libqxp::QXPDocument;

int main(int argc, char *argv[])
{
  if (argc < 2)
    return printUsage();

  char *file = 0;

  for (int i = 1; i < argc; i++)
  {
    if (!std::strcmp(argv[i], "--version"))
      return printVersion();
    else if (!file && std::strncmp(argv[i], "--", 2))
      file = argv[i];
    else
      return printUsage();
  }

  if (!file)
    return printUsage();

  librevenge::RVNGFileStream input(file);

  QXPDocument::Type type = QXPDocument::TYPE_UNKNOWN;
  const bool supported = QXPDocument::isSupported(&input, &type);
  if (!supported|| (type != QXPDocument::TYPE_DOCUMENT && type != QXPDocument::TYPE_TEMPLATE))
  {
    std::cerr << "ERROR: Unsupported file format" << std::endl;
    return 1;
  }

  librevenge::RVNGStringVector pages;
  librevenge::RVNGTextDrawingGenerator documentGenerator(pages);

  if (QXPDocument::RESULT_OK != QXPDocument::parse(&input, &documentGenerator))
    return 1;

  for (unsigned i = 0; i != pages.size(); ++i)
  {
    std::puts(pages[i].cstr());
    std::puts("\n");
  }

  return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
