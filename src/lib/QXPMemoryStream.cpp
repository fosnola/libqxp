/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPMemoryStream.h"

#include <algorithm>

namespace libqxp
{

QXPMemoryStream::QXPMemoryStream(const unsigned char *data, unsigned length)
  : m_data()
  , m_length((long) length)
  , m_pos(0)
{
  if (length > 0)
  {
    m_data.reset(new unsigned char[length]);
    std::copy(data, data + length, m_data.get());
  }
}

QXPMemoryStream::~QXPMemoryStream()
{
}

bool QXPMemoryStream::isStructured()
{
  return false;
}

unsigned QXPMemoryStream::subStreamCount()
{
  return 0;
}

const char *QXPMemoryStream::subStreamName(unsigned)
{
  return 0;
}

bool QXPMemoryStream::existsSubStream(const char *)
{
  return false;
}

librevenge::RVNGInputStream *QXPMemoryStream::getSubStreamByName(const char *)
{
  return 0;
}

librevenge::RVNGInputStream *QXPMemoryStream::getSubStreamById(unsigned)
{
  return 0;
}

const unsigned char *QXPMemoryStream::read(unsigned long numBytes, unsigned long &numBytesRead) try
{
  numBytesRead = 0;

  if ((0 == numBytes) || (0 == m_length))
    return 0;

  if ((static_cast<unsigned long>(m_pos) + numBytes) >= static_cast<unsigned long>(m_length))
    numBytes = static_cast<unsigned long>(m_length - m_pos);

  const long oldPos = m_pos;
  m_pos += numBytes;

  numBytesRead = numBytes;
  return m_data.get() + oldPos;
}
catch (...)
{
  return 0;
}

int QXPMemoryStream::seek(const long offset, librevenge::RVNG_SEEK_TYPE seekType) try
{
  long pos = 0;
  switch (seekType)
  {
  case librevenge::RVNG_SEEK_SET :
    pos = offset;
    break;
  case librevenge::RVNG_SEEK_CUR :
    pos = offset + m_pos;
    break;
  case librevenge::RVNG_SEEK_END :
    pos = offset + m_length;
    break;
  default :
    return -1;
  }

  if ((pos < 0) || (pos > m_length))
    return 1;

  m_pos = pos;
  return 0;
}
catch (...)
{
  return -1;
}

long QXPMemoryStream::tell()
{
  return m_pos;
}

bool QXPMemoryStream::isEnd()
{
  return m_length == m_pos;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
