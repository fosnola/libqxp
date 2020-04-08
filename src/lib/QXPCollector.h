/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPCOLLECTOR_H_INCLUDED
#define QXPCOLLECTOR_H_INCLUDED

#include "libqxp_utils.h"

namespace libqxp
{

struct Box;
struct Group;
struct Line;
struct Page;
struct QXPDocumentProperties;
struct Text;
struct TextBox;
struct TextPath;

class QXPCollector
{
  // disable copying
  QXPCollector(const QXPCollector &other) = delete;
  QXPCollector &operator=(const QXPCollector &other) = delete;

public:
  QXPCollector() = default;
  virtual ~QXPCollector() = default;

  virtual void startDocument() { }
  virtual void endDocument() { }

  virtual void startPage(const Page &) { }
  virtual void endPage() { }

  virtual void collectDocumentProperties(const QXPDocumentProperties &) { }

  virtual void collectLine(const std::shared_ptr<Line> &) { }
  virtual void collectBox(const std::shared_ptr<Box> &) { }
  virtual void collectTextBox(const std::shared_ptr<TextBox> &) { }
  virtual void collectTextPath(const std::shared_ptr<TextPath> &) { }
  virtual void collectGroup(const std::shared_ptr<Group> &) { }

  virtual void collectText(const std::shared_ptr<Text> &, const unsigned) { }
};

class QXPDummyCollector : public QXPCollector
{
public:
  QXPDummyCollector() = default;
};

}

#endif // QXPCOLLECTOR_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
