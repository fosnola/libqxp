/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXP33Header.h"

#include "QXP33Parser.h"

namespace libqxp
{

QXP33Header::QXP33Header(const boost::optional<QXPDocument::Type> &fileType)
  : QXP3HeaderBase(fileType)
  , m_pagesCount(0)
  , m_masterPagesCount(0)
  , m_seed(0)
  , m_increment()
  , m_documentProperties()
{
}

bool QXP33Header::load(const std::shared_ptr<librevenge::RVNGInputStream> &input)
{
  QXP3HeaderBase::load(input);

  seek(input, 0x40);
  m_pagesCount = readU16(input, isBigEndian());
  skip(input, 51);
  m_masterPagesCount = readU8(input);

  skip(input, 6);
  m_documentProperties.setAutoLeading(readFraction(input, isBigEndian()));

  skip(input, 84);
  m_documentProperties.superscriptOffset = readFraction(input, isBigEndian());
  m_documentProperties.superscriptHScale = readFraction(input, isBigEndian());
  m_documentProperties.superscriptVScale = readFraction(input, isBigEndian());
  m_documentProperties.subscriptOffset = -readFraction(input, isBigEndian());
  m_documentProperties.subscriptHScale = readFraction(input, isBigEndian());
  m_documentProperties.subscriptVScale = readFraction(input, isBigEndian());
  m_documentProperties.superiorHScale = readFraction(input, isBigEndian());
  m_documentProperties.superiorVScale = readFraction(input, isBigEndian());

  seekRelative(input, 28);
  m_seed = readU16(input, !isLittleEndian());
  m_increment = readU16(input, !isLittleEndian());

  seek(input, 512);

  return true;
}

QXPDocument::Type QXP33Header::getType() const
{
  return m_fileType ? get(m_fileType) : QXPDocument::TYPE_DOCUMENT;
}

std::unique_ptr<QXPParser> QXP33Header::createParser(const std::shared_ptr<librevenge::RVNGInputStream> &input, librevenge::RVNGDrawingInterface *painter)
{
  return std::unique_ptr<QXPParser>(new QXP33Parser(input, painter, shared_from_this()));
}

uint16_t QXP33Header::pagesCount() const
{
  return m_pagesCount;
}

uint16_t QXP33Header::masterPagesCount() const
{
  return m_masterPagesCount;
}

uint16_t QXP33Header::seed() const
{
  return m_seed;
}

uint16_t QXP33Header::increment() const
{
  return m_increment;
}

const QXPDocumentProperties &QXP33Header::documentProperties() const
{
  return m_documentProperties;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
