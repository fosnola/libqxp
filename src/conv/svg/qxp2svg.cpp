/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
#include <sstream>

#include <librevenge/librevenge.h>
#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>

#include <libqxp/libqxp.h>

namespace
{

int printUsage()
{
  std::printf("`qxp2svg' converts QuarkXPress documents to SVG.\n");
  std::printf("\n");
  std::printf("Usage: qxp2svg [OPTION] FILE\n");
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
  std::printf("qxp2svg " VERSION "\n");
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
  if (!supported || (type != QXPDocument::TYPE_DOCUMENT && type != QXPDocument::TYPE_TEMPLATE))
  {
    std::cerr << "ERROR: Unsupported file format" << std::endl;
    return 1;
  }

  librevenge::RVNGStringVector vec;
  librevenge::RVNGSVGDrawingGenerator generator(vec, "");
  auto result = QXPDocument::parse(&input, &generator);
  if (QXPDocument::RESULT_OK != result || vec.empty() || vec[0].empty())
  {
    std::cerr << "ERROR: SVG Generation failed!" << std::endl;
    return 1;
  }

#if 1
  std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
  std::cout << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"";
  std::cout << " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
#endif
  std::cout << vec[0].cstr() << std::endl;
  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
