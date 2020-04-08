/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPMacFileParser.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <vector>

#include "QXPMemoryStream.h"
#include "libqxp_utils.h"

namespace libqxp
{

/*! \class MWAWInputStream
 * \brief Internal class used to read the file stream
 *  Internal class used to read the file stream,
 *    this class adds some usefull functions to the basic librevenge::RVNGInputStream:
 *  - read number (int8, int16, int32) in low or end endian
 *  - selection of a section of a stream
 *  - read block of data
 *  - interface with modified librevenge::RVNGOLEStream
 */
class MWAWInputStream
{
public:
  /*!\brief creates a stream with given endian
   * \param input the given input
   * \param inverted must be set to true for pc doc and ole part and to false for mac doc
   */
  MWAWInputStream(std::shared_ptr<librevenge::RVNGInputStream> input, bool inverted);

  /*!\brief creates a stream with given endian from an existing input
   *
   * Note: this functions does not delete input
   */
  MWAWInputStream(librevenge::RVNGInputStream *input, bool inverted, bool checkCompression=false);
  //! destructor
  ~MWAWInputStream();

  //! returns the basic librevenge::RVNGInputStream
  std::shared_ptr<librevenge::RVNGInputStream> input()
  {
    return m_stream;
  }
  //! returns a new input stream corresponding to a librevenge::RVNGBinaryData
  static std::shared_ptr<MWAWInputStream> get(librevenge::RVNGBinaryData const &data, bool inverted);

  //! returns the endian mode (see constructor)
  bool readInverted() const
  {
    return m_inverseRead;
  }
  //! sets the endian mode
  void setReadInverted(bool newVal)
  {
    m_inverseRead = newVal;
  }
  //
  // Position: access
  //

  /*! \brief seeks to a offset position, from actual, beginning or ending position
   * \return 0 if ok
   * \sa pushLimit popLimit
   */
  int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType);
  //! returns actual offset position
  long tell();
  //! returns the stream size
  long size() const
  {
    return m_streamSize;
  }
  //! checks if a position is or not a valid file position
  bool checkPosition(long pos) const
  {
    if (pos < 0) return false;
    if (m_readLimit > 0 && pos > m_readLimit) return false;
    return pos<=m_streamSize;
  }
  //! returns true if we are at the end of the section/file
  bool isEnd();

  /*! \brief defines a new section in the file (from actualPos to newLimit)
   * next call of seek, tell, atEos, ... will be restrained to this section
   */
  void pushLimit(long newLimit)
  {
    m_prevLimits.push_back(m_readLimit);
    m_readLimit = newLimit > m_streamSize ? m_streamSize : newLimit;
  }
  //! pops a section defined by pushLimit
  void popLimit()
  {
    if (m_prevLimits.size())
    {
      m_readLimit = m_prevLimits.back();
      m_prevLimits.pop_back();
    }
    else m_readLimit = -1;
  }

  //
  // get data
  //

  //! returns a uint8, uint16, uint32 readed from actualPos
  unsigned long readULong(int num)
  {
    return readULong(m_stream.get(), num, 0, m_inverseRead);
  }
  //! return a int8, int16, int32 readed from actualPos
  long readLong(int num);
  //! try to read a double of size 8: 1.5 bytes exponent, 6.5 bytes mantisse
  bool readDouble8(double &res, bool &isNotANumber);
  //! try to read a double of size 8: 6.5 bytes mantisse, 1.5 bytes exponent
  bool readDoubleReverted8(double &res, bool &isNotANumber);
  //! try to read a double of size 10: 2 bytes exponent, 8 bytes mantisse
  bool readDouble10(double &res, bool &isNotANumber);

  /**! reads numbytes data, WITHOUT using any endian or section consideration
   * \return a pointer to the read elements
   */
  const uint8_t *read(size_t numBytes, unsigned long &numBytesRead);
  /*! \brief internal function used to read num byte,
   *  - where a is the previous read data
   */
  static unsigned long readULong(librevenge::RVNGInputStream *stream, int num, unsigned long a, bool inverseRead);

  //! reads a librevenge::RVNGBinaryData with a given size in the actual section/file
  bool readDataBlock(long size, librevenge::RVNGBinaryData &data);
  //! reads a librevenge::RVNGBinaryData from actPos to the end of the section/file
  bool readEndDataBlock(librevenge::RVNGBinaryData &data);

  //
  // OLE/Zip access
  //

  //! return true if the stream is ole
  bool isStructured();
  //! returns the number of substream
  unsigned subStreamCount();
  //! returns the name of the i^th substream
  std::string subStreamName(unsigned id);

  //! return a new stream for a ole zone
  std::shared_ptr<MWAWInputStream> getSubStreamByName(std::string const &name);
  //! return a new stream for a ole zone
  std::shared_ptr<MWAWInputStream> getSubStreamById(unsigned id);

  //
  // Finder Info access
  //
  /** returns the finder info type and creator (if known) */
  bool getFinderInfo(std::string &type, std::string &creator) const
  {
    if (!m_fInfoType.length() || !m_fInfoCreator.length())
    {
      type = creator = "";
      return false;
    }
    type = m_fInfoType;
    creator = m_fInfoCreator;
    return true;
  }

  //
  // Resource Fork access
  //

  /** returns true if the data fork block exists */
  bool hasDataFork() const
  {
    return bool(m_stream);
  }
  /** returns true if the resource fork block exists */
  bool hasResourceFork() const
  {
    return bool(m_resourceFork);
  }
  /** returns the resource fork if find */
  std::shared_ptr<MWAWInputStream> getResourceForkStream()
  {
    return m_resourceFork;
  }


protected:
  //! update the stream size ( must be called in the constructor )
  void updateStreamSize();
  //! internal function used to read a byte
  static uint8_t readU8(librevenge::RVNGInputStream *stream);

  //! unbinhex the data in the file is a BinHex 4.0 file of a mac file
  bool unBinHex();
  //! unzip the data in the file is a zip file of a mac file
  bool unzipStream();
  //! check if some stream are in MacMIME format, if so de MacMIME
  bool unMacMIME();
  //! de MacMIME an input stream
  bool unMacMIME(MWAWInputStream *input,
                 std::shared_ptr<librevenge::RVNGInputStream> &dataInput,
                 std::shared_ptr<librevenge::RVNGInputStream> &rsrcInput) const;
  //! check if a stream is an internal merge stream
  bool unsplitInternalMergeStream();

private:
  MWAWInputStream(MWAWInputStream const &orig) = delete;
  MWAWInputStream &operator=(MWAWInputStream const &orig) = delete;

protected:
  //! the initial input
  std::shared_ptr<librevenge::RVNGInputStream> m_stream;
  //! the stream size
  long m_streamSize;

  //! actual section limit (-1 if no limit)
  long m_readLimit;
  //! list of previous limits
  std::vector<long> m_prevLimits;

  //! finder info type
  mutable std::string m_fInfoType;
  //! finder info type
  mutable std::string m_fInfoCreator;
  //! the resource fork
  std::shared_ptr<MWAWInputStream> m_resourceFork;
  //! big or normal endian
  bool m_inverseRead;
};

MWAWInputStream::MWAWInputStream(std::shared_ptr<librevenge::RVNGInputStream> inp, bool inverted)
  : m_stream(inp), m_streamSize(0), m_readLimit(-1), m_prevLimits(),
    m_fInfoType(""), m_fInfoCreator(""), m_resourceFork(), m_inverseRead(inverted)
{
  updateStreamSize();
}

MWAWInputStream::MWAWInputStream(librevenge::RVNGInputStream *inp, bool inverted, bool checkCompression)
  : m_stream(), m_streamSize(0), m_readLimit(-1), m_prevLimits(),
    m_fInfoType(""), m_fInfoCreator(""), m_resourceFork(), m_inverseRead(inverted)
{
  if (!inp) return;

  m_stream = std::shared_ptr<librevenge::RVNGInputStream>(inp, QXPDummyDeleter());
  updateStreamSize();
  if (!checkCompression)
    return;

  // first check if the file is "local" structure one
  if (unsplitInternalMergeStream())
    updateStreamSize();
  // then check the zip format
  if (unzipStream())
    updateStreamSize();

  // then check if the data are in binhex format
  if (unBinHex())
    updateStreamSize();

  // now check for MacMIME format in m_stream or in m_resourceFork
  if (unMacMIME())
    updateStreamSize();
  if (m_stream)
    seek(0, librevenge::RVNG_SEEK_SET);
  if (m_resourceFork)
    m_resourceFork->seek(0, librevenge::RVNG_SEEK_SET);
}

MWAWInputStream::~MWAWInputStream()
{
}

std::shared_ptr<MWAWInputStream> MWAWInputStream::get(librevenge::RVNGBinaryData const &data, bool inverted)
{
  std::shared_ptr<MWAWInputStream> res;
  if (!data.size())
    return res;
  librevenge::RVNGInputStream *dataStream = const_cast<librevenge::RVNGInputStream *>(data.getDataStream());
  if (!dataStream)
  {
    QXP_DEBUG_MSG(("MWAWInputStream::get: can not retrieve a librevenge::RVNGInputStream\n"));
    return res;
  }
  res.reset(new MWAWInputStream(dataStream, inverted));
  if (res && res->size()>=long(data.size()))
  {
    res->seek(0, librevenge::RVNG_SEEK_SET);
    return res;
  }
  QXP_DEBUG_MSG(("MWAWInputStream::get: the final stream seems bad\n"));
  res.reset();
  return res;
}

void MWAWInputStream::updateStreamSize()
{
  if (!m_stream)
    m_streamSize=0;
  else
  {
    long actPos = tell();
    m_stream->seek(0, librevenge::RVNG_SEEK_END);
    m_streamSize=tell();
    m_stream->seek(actPos, librevenge::RVNG_SEEK_SET);
  }
}

const uint8_t *MWAWInputStream::read(size_t numBytes, unsigned long &numBytesRead)
{
  if (!hasDataFork())
    throw FileAccessError();
  return m_stream->read(numBytes, numBytesRead);
}

long MWAWInputStream::tell()
{
  if (!hasDataFork())
    return 0;
  return m_stream->tell();
}

int MWAWInputStream::seek(long offset, librevenge::RVNG_SEEK_TYPE seekType)
{
  if (!hasDataFork())
  {
    if (offset == 0)
      return 0;
    throw FileAccessError();
  }

  if (seekType == librevenge::RVNG_SEEK_CUR)
    offset += tell();
  else if (seekType==librevenge::RVNG_SEEK_END)
    offset += m_streamSize;

  if (offset < 0)
    offset = 0;
  if (m_readLimit > 0 && offset > long(m_readLimit))
    offset = long(m_readLimit);
  if (offset > size())
    offset = size();

  return m_stream->seek(offset, librevenge::RVNG_SEEK_SET);
}

bool MWAWInputStream::isEnd()
{
  if (!hasDataFork())
    return true;
  long pos = m_stream->tell();
  if (m_readLimit > 0 && pos >= m_readLimit) return true;
  if (pos >= size()) return true;

  return m_stream->isEnd();
}

unsigned long MWAWInputStream::readULong(librevenge::RVNGInputStream *stream, int num, unsigned long a, bool inverseRead)
{
  if (!stream || num <= 0 || stream->isEnd()) return a;
  if (num > 8)   // reading more than 8 bytes does not make any sense, ...
  {
    QXP_DEBUG_MSG(("MWAWInputStream::readULong: argh called with %d bytes, resets it to 8 bytes\n", num));
    num = 8;
  }
  else if (num > 4)   // normally, must be (not recursively) called with 1, 2, 4 bytes, so...
  {
    QXP_DEBUG_MSG(("MWAWInputStream::readULong: argh called with %d bytes\n", num));
  }
  if (inverseRead)
  {
    unsigned long val = readU8(stream);
    return val + (readULong(stream, num-1, 0, inverseRead) << 8);
  }
  switch (num)
  {
  case 4:
  case 2:
  case 1:
  {
    unsigned long numBytesRead;
    uint8_t const *p = stream->read(static_cast<unsigned long>(num), numBytesRead);
    if (!p || int(numBytesRead) != num)
      return 0;
    switch (num)
    {
    case 4:
      return static_cast<unsigned long>(p[3])|(static_cast<unsigned long>(p[2])<<8)|
             (static_cast<unsigned long>(p[1])<<16)|(static_cast<unsigned long>(p[0])<<24)|((a<<16)<<16);
    case 2:
      return (static_cast<unsigned long>(p[1]))|(static_cast<unsigned long>(p[0])<<8)|(a<<16);
    case 1:
      return (static_cast<unsigned long>(p[0]))|(a<<8);
    default:
      break;
    }
    break;
  }
  default:
    return readULong(stream, num-1,(a<<8) + static_cast<unsigned long>(readU8(stream)), inverseRead);
  }
  return 0;
}

long MWAWInputStream::readLong(int num)
{
  long v = long(readULong(num));
  switch (num)
  {
  case 4:
    return static_cast<int32_t>(v);
  case 2:
    return static_cast<int16_t>(v);
  case 1:
    return static_cast<int8_t>(v);
  default:
    break;
  }

  if ((v & long(0x1 << (num*8-1))) == 0) return v;
  return v | long(0xFFFFFFFF << 8*num);
}

uint8_t MWAWInputStream::readU8(librevenge::RVNGInputStream *stream)
{
  if (!stream)
    return 0;
  unsigned long numBytesRead;
  uint8_t const *p = stream->read(sizeof(uint8_t), numBytesRead);

  if (!p || numBytesRead != sizeof(uint8_t))
    return 0;

  return *p;
}

bool MWAWInputStream::readDouble8(double &res, bool &isNotANumber)
{
  if (!m_stream) return false;
  long pos=tell();
  if (m_readLimit > 0 && pos+8 > m_readLimit) return false;
  if (pos+8 > m_streamSize) return false;

  isNotANumber=false;
  res=0;
  int mantExp=int(readULong(1));
  int val=static_cast<int>(readULong(1));
  int exp=(mantExp<<4)+(val>>4);
  double mantisse=double(val&0xF)/16.;
  double factor=1./16/256.;
  for (int j = 0; j < 6; ++j, factor/=256)
    mantisse+=double(readULong(1))*factor;
  int sign = 1;
  if (exp & 0x800)
  {
    exp &= 0x7ff;
    sign = -1;
  }
  if (exp == 0)
  {
    if (mantisse <= 1.e-5 || mantisse >= 1-1.e-5)
      return true;
    // a Nan representation ?
    return false;
  }
  if (exp == 0x7FF)
  {
    if (mantisse >= 1.-1e-5)
    {
      isNotANumber=true;
      res=std::numeric_limits<double>::quiet_NaN();
      return true; // ok 0x7FF and 0xFFF are nan
    }
    return false;
  }
  exp -= 0x3ff;
  res = std::ldexp(1.+mantisse, exp);
  if (sign == -1)
    res *= -1.;
  return true;
}

bool MWAWInputStream::readDouble10(double &res, bool &isNotANumber)
{
  if (!m_stream) return false;
  long pos=tell();
  if (m_readLimit > 0 && pos+10 > m_readLimit) return false;
  if (pos+10 > m_streamSize) return false;

  int exp = static_cast<int>(readULong(2));
  int sign = 1;
  if (exp & 0x8000)
  {
    exp &= 0x7fff;
    sign = -1;
  }
  exp -= 0x3fff;

  isNotANumber=false;
  unsigned long mantisse = static_cast<unsigned long>(readULong(4));
  if ((mantisse & 0x80000001) == 0)
  {
    // unormalized number are not frequent, but can appear at least for date, ...
    if (readULong(4) != 0)
      seek(-4, librevenge::RVNG_SEEK_CUR);
    else
    {
      if (exp == -0x3fff && mantisse == 0)
      {
        res=0;
        return true; // ok zero
      }
      if (exp == 0x4000 && (mantisse & 0xFFFFFFL)==0)   // ok Nan
      {
        isNotANumber = true;
        res=std::numeric_limits<double>::quiet_NaN();
        return true;
      }
      return false;
    }
  }
  // or std::ldexp((total value)/double(0x80000000), exp);
  res=std::ldexp(double(readULong(4)), exp-63)+std::ldexp(double(mantisse), exp-31);
  if (sign == -1)
    res *= -1.;
  return true;
}

bool MWAWInputStream::readDoubleReverted8(double &res, bool &isNotANumber)
{
  if (!m_stream) return false;
  long pos=tell();
  if (m_readLimit > 0 && pos+8 > m_readLimit) return false;
  if (pos+8 > m_streamSize) return false;

  isNotANumber=false;
  res=0;
  int bytes[6];
  for (int i=0; i<6; ++i) bytes[i]=static_cast<int>(readULong(1));

  int val=static_cast<int>(readULong(1));
  int mantExp=int(readULong(1));
  int exp=(mantExp<<4)+(val>>4);
  double mantisse=double(val&0xF)/16.;
  double factor=1./16./256.;
  for (int j = 0; j < 6; ++j, factor/=256)
    mantisse+=double(bytes[5-j])*factor;
  int sign = 1;
  if (exp & 0x800)
  {
    exp &= 0x7ff;
    sign = -1;
  }
  if (exp == 0)
  {
    if (mantisse <= 1.e-5 || mantisse >= 1-1.e-5)
      return true;
    // a Nan representation ?
    return false;
  }
  if (exp == 0x7FF)
  {
    if (mantisse >= 1.-1e-5)
    {
      isNotANumber=true;
      res=std::numeric_limits<double>::quiet_NaN();
      return true; // ok 0x7FF and 0xFFF are nan
    }
    return false;
  }
  exp -= 0x3ff;
  res = std::ldexp(1.+mantisse, exp);
  if (sign == -1)
    res *= -1.;
  return true;
}

////////////////////////////////////////////////////////////
//
// BinHex 4.0
//
////////////////////////////////////////////////////////////
bool MWAWInputStream::unBinHex()
{
  if (!hasDataFork() || size() < 45)
    return false;
  // check header
  seek(0, librevenge::RVNG_SEEK_SET);
  unsigned long nRead;
  char const *str=reinterpret_cast<char const *>(read(45, nRead));
  if (str==0 || nRead!=45
      || strncmp(str, "(This file must be converted with BinHex 4.0)", 45))
    return false;
  int numEOL = 0;
  while (!isEnd())
  {
    char c = char(readLong(1));
    if (c == '\n')
    {
      numEOL++;
      continue;
    }
    seek(-1, librevenge::RVNG_SEEK_CUR);
    break;
  }
  if (isEnd() || !numEOL || (char(readLong(1)))!= ':')
    return false;

  // first phase reconstruct the file
  static char const binChar[65] = "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";
  std::map<unsigned char, int>  binMap;
  for (int i = 0; i < 64; i++) binMap[static_cast<unsigned char>(binChar[i])]=i;
  bool endData = false;
  int numActByte = 0, actVal = 0;
  librevenge::RVNGBinaryData content;
  bool findRepetitif = false;
  while (1)
  {
    if (isEnd())
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: do not find ending ':' character\n"));
      return false;
    }
    unsigned char c = static_cast<unsigned char>(readLong(1));
    if (c == '\n') continue;
    int readVal = 0;
    if (c == ':')
      endData = true;
    else if (binMap.find(c) == binMap.end())
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: find unexpected char when decoding file\n"));
      return false;
    }
    else
      readVal = binMap.find(c)->second;
    int wVal = -1;
    if (numActByte==0)
      actVal |= (readVal<<2);
    else if (numActByte==2)
    {
      wVal = (actVal | readVal);
      actVal = 0;
    }
    else if (numActByte==4)
    {
      wVal = actVal | ((readVal>>2)&0xF);
      actVal = (readVal&0x3)<<6;
    }
    else if (numActByte==6)
    {
      wVal = actVal | ((readVal>>4)&0x3);
      actVal = (readVal&0xf)<<4;
    }
    numActByte = (numActByte+6)%8;

    int maxToWrite = (endData&&actVal) ? 2 : 1;
    for (int wPos = 0; wPos < maxToWrite; wPos++)
    {
      int value = wPos ? actVal : wVal;
      if (value == -1) continue;
      if (!findRepetitif && value != 0x90)
      {
        content.append(static_cast<unsigned char>(value));
        continue;
      }
      if (value == 0x90 && !findRepetitif)
      {
        findRepetitif = true;
        continue;
      }

      if (value == 1 || value == 2)
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: find bad value after repetitif character\n"));
        return false;
      }
      findRepetitif = false;
      if (value == 0)
      {
        content.append(0x90);
        continue;
      }
      if (content.size()==0)
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: find repetitif character in the first position\n"));
        return false;
      }
      unsigned char lChar = content.getDataBuffer()[content.size()-1];
      for (int i = 0; i < value-1; i++)
        content.append(lChar);
      continue;
    }
    if (endData) break;
  }
  if (findRepetitif)
  {
    QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: find repetitif character in the last position\n"));
    return false;
  }
  long contentSize=long(content.size());
  if (contentSize < 27)
  {
    QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: the content file is too small\n"));
    return false;
  }
  librevenge::RVNGInputStream *contentInput=const_cast<librevenge::RVNGInputStream *>(content.getDataStream());
  int fileLength = static_cast<int>(readU8(contentInput));
  if (fileLength < 1 || fileLength > 64 || long(fileLength+21) > contentSize)
  {
    QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: the file name size seems odd\n"));
    return false;
  }
  contentInput->seek(fileLength+1, librevenge::RVNG_SEEK_CUR); // filename + version
  // creator, type
  std::string type(""), creator("");
  for (int p = 0; p < 4; p++)
  {
    char c = char(readU8(contentInput));
    if (c)
      type += c;
  }
  for (int p = 0; p < 4; p++)
  {
    char c = char(readU8(contentInput));
    if (c)
      creator += c;
  }
  if (creator.length()==4 && type.length()==4)
  {
    m_fInfoType = type;
    m_fInfoCreator = creator;
  }
  else if (creator.length() || type.length())
  {
    QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: the file name size seems odd\n"));
  }
  contentInput->seek(2, librevenge::RVNG_SEEK_CUR); // skip flags
  long dataLength = long(readULong(contentInput, 4, 0, false));
  long rsrcLength = long(readULong(contentInput, 4, 0, false));
  long pos = contentInput->tell()+2; // skip CRC
  if (dataLength<0 || rsrcLength < 0 || (dataLength==0 && rsrcLength==0) ||
      pos+dataLength+rsrcLength+4 > contentSize)
  {
    QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: the data/rsrc fork size seems odd\n"));
    return false;
  }
  // now read the rsrc and the data fork
  if (rsrcLength && getResourceForkStream())
  {
    QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: I already have a resource fork!!!!\n"));
  }
  else if (rsrcLength)
  {
    contentInput->seek(pos+dataLength+2, librevenge::RVNG_SEEK_SET);
    unsigned long numBytesRead = 0;
    const unsigned char *data =
      contentInput->read(static_cast<unsigned long>(rsrcLength), numBytesRead);
    if (numBytesRead != static_cast<unsigned long>(rsrcLength) || !data)
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: can not read the resource fork\n"));
    }
    else
    {
      std::shared_ptr<librevenge::RVNGInputStream> rsrc(new QXPMemoryStream(data, static_cast<unsigned int>(numBytesRead)));
      m_resourceFork.reset(new MWAWInputStream(rsrc, false));
    }
  }
  if (!dataLength)
    m_stream.reset();
  else
  {
    contentInput->seek(pos, librevenge::RVNG_SEEK_SET);
    unsigned long numBytesRead = 0;
    const unsigned char *data =
      contentInput->read(static_cast<unsigned long>(dataLength), numBytesRead);
    if (numBytesRead != static_cast<unsigned long>(dataLength) || !data)
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unBinHex: can not read the data fork\n"));
      return false;
    }
    m_stream.reset(new QXPMemoryStream(data, static_cast<unsigned int>(numBytesRead)));
  }

  return true;
}

////////////////////////////////////////////////////////////
//
// Internal : if the main stream is a structured file, which
//   contains a DataFork, a RsrcFork and a InfoFork or RsrcInfo
//   stream, ...
//
////////////////////////////////////////////////////////////
bool MWAWInputStream::unsplitInternalMergeStream()
{
  if (!isStructured() || m_resourceFork || !m_stream->existsSubStream("DataFork"))
    return false;
  try
  {
    if (m_stream->subStreamCount()==2 && m_stream->existsSubStream("RsrcInfo"))
    {
      std::shared_ptr<librevenge::RVNGInputStream> newRsrcFork(m_stream->getSubStreamByName("RsrcInfo"));
      if (newRsrcFork)   // rsrcinfo must be a MacMIME file, it can not be empty
      {
        m_stream.reset(m_stream->getSubStreamByName("DataFork")); // empty data fork is rare, but possible
        m_resourceFork.reset(new MWAWInputStream(newRsrcFork, m_inverseRead)); // rsrcinfo will be decoded by unMacMIME
      }
      else
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unsplitInternalMergeStream: sorry, can not extract the data fork or the rsrc info fork\n"));
      }
    }
    else if (m_stream->subStreamCount()==3 && m_stream->existsSubStream("RsrcFork") && m_stream->existsSubStream("InfoFork"))
    {
      std::shared_ptr<librevenge::RVNGInputStream> newRsrcFork(m_stream->getSubStreamByName("RsrcFork"));
      if (newRsrcFork)
      {
        std::shared_ptr<librevenge::RVNGInputStream> newInfo(m_stream->getSubStreamByName("InfoFork"));
        m_stream.reset(m_stream->getSubStreamByName("DataFork"));
        m_resourceFork.reset(new MWAWInputStream(newRsrcFork, m_inverseRead));
        // we must decode the information here
        unsigned long numBytesRead = 0;
        const unsigned char *data = newInfo ? newInfo->read(8, numBytesRead) : 0;
        if (numBytesRead != 8 || !data)
        {
          QXP_DEBUG_MSG(("MWAWInputStream::unsplitInternalMergeStream: sorry, unknown file information size\n"));
        }
        else
        {
          bool ok = true;
          std::string type(""), creator("");
          for (int p = 0; p < 4; p++)
          {
            if (!data[p])
            {
              ok = false;
              break;
            }
            type += char(data[p]);
          }
          for (int p = 4; ok && p < 8; p++)
          {
            if (!data[p])
            {
              ok = false;
              break;
            }
            creator += char(data[p]);
          }
          if (ok)
          {
            m_fInfoType = type;
            m_fInfoCreator = creator;
          }
          else if (type.length())
          {
            QXP_DEBUG_MSG(("MWAWInputStream::unsplitInternalMergeStream: can not read find info\n"));
          }
        }
      }
      else
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unsplitInternalMergeStream: sorry, can not extract the data fork or the rsrc info fork\n"));
      }
    }
  }
  catch (...)
  {
  }
  return false;
}

////////////////////////////////////////////////////////////
//
// Zip
//
////////////////////////////////////////////////////////////
bool MWAWInputStream::unzipStream()
{
  if (!isStructured()) return false;
  seek(0, librevenge::RVNG_SEEK_SET);
  unsigned numStream=m_stream->subStreamCount();
  std::vector<std::string> names;
  for (unsigned n=0; n < numStream; ++n)
  {
    char const *nm=m_stream->subStreamName(n);
    if (!nm) continue;
    std::string name(nm);
    if (name.empty() || name[name.length()-1]=='/') continue;
    names.push_back(nm);
  }
  if (names.size() == 1)
  {
    // ok as the OLE file must have at least MN and MN0 OLE
    m_stream.reset(m_stream->getSubStreamByName(names[0].c_str()));
    return true;
  }
  if (names.size() != 2)
    return false;
  // test if XXX and ._XXX or __MACOSX/._XXX
  if (names[1].length() < names[0].length())
  {
    std::string tmp = names[1];
    names[1] = names[0];
    names[0] = tmp;
  }
  size_t length = names[0].length();
  std::string prefix("");
  if (names[1].length()==length+2)
    prefix = "._";
  else if (names[1].length()==length+11)
    prefix = "__MACOSX/._";
  prefix += names[0];
  if (prefix != names[1]) return false;
  std::shared_ptr<librevenge::RVNGInputStream> rsrcPtr(m_stream->getSubStreamByName(names[1].c_str()));
  m_resourceFork.reset(new MWAWInputStream(rsrcPtr, false));
  m_stream.reset(m_stream->getSubStreamByName(names[0].c_str()));
  return true;
}

////////////////////////////////////////////////////////////
//
// MacMIME part
//
////////////////////////////////////////////////////////////
bool MWAWInputStream::unMacMIME()
{
  if (m_resourceFork)
  {
    std::shared_ptr<librevenge::RVNGInputStream> newDataInput, newRsrcInput;
    bool ok = unMacMIME(m_resourceFork.get(), newDataInput, newRsrcInput);
    if (ok && newDataInput)
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: Argh!!! find data stream in the resource block\n"));
      ok = false;
    }
    if (ok && newRsrcInput)
      m_resourceFork.reset(new MWAWInputStream(newRsrcInput, false));
    else if (ok)
      m_resourceFork.reset();
  }

  if (m_stream)
  {
    std::shared_ptr<librevenge::RVNGInputStream> newDataInput, newRsrcInput;
    bool ok = unMacMIME(this, newDataInput, newRsrcInput);
    if (ok && !newDataInput)
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: Argh!!! data block contains only resources\n"));
    }
    if (ok)
    {
      m_stream = newDataInput;
      if (newRsrcInput)
      {
        if (m_resourceFork)
        {
          QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: Oops!!! find a second resource block, ignored\n"));
        }
        else
          m_resourceFork.reset(new MWAWInputStream(newRsrcInput, false));
      }
    }
  }
  return true;
}

/* freely inspired from http://tools.ietf.org/html/rfc1740#appendix-A
 */
bool MWAWInputStream::unMacMIME(MWAWInputStream *inp,
                                std::shared_ptr<librevenge::RVNGInputStream> &dataInput,
                                std::shared_ptr<librevenge::RVNGInputStream> &rsrcInput) const
{
  dataInput.reset();
  rsrcInput.reset();
  if (!inp || !inp->hasDataFork() || inp->size()<26) return false;

  try
  {
    inp->seek(0, librevenge::RVNG_SEEK_SET);
    long magicNumber = long(inp->readULong(4));
    if (magicNumber != 0x00051600 && magicNumber != 0x00051607)
      return false;
    long version = long(inp->readULong(4));
    if (version != 0x20000)
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: unknown version: %lx\n", static_cast<long unsigned int>(version)));
      return false;
    }
    inp->seek(16, librevenge::RVNG_SEEK_CUR); // filename
    long numEntries = long(inp->readULong(2));
    if (inp->checkPosition(inp->tell() + 12 * numEntries)) // minimal sanity check
      numEntries = (inp->size() - inp->tell()) / 12;
    if (inp->isEnd() || numEntries == 0)
    {
      QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: can not read number of entries\n"));
      return false;
    }
#ifdef DEBUG
    static const char *what[] =
    {
      "", "Data", "Rsrc", "FileName", "Comment", "IconBW", "IconColor", "",
      "FileDates", "FinderInfo", "MacInfo", "ProDosInfo",
      "MSDosInfo", "AFPName", "AFPInfo", "AFPDirId"
    };
#endif
    for (int i = 0; i < numEntries && !inp->isEnd(); i++)
    {
      long pos = inp->tell();
      int wh = static_cast<int>(inp->readULong(4));
      if (wh <= 0 || wh >= 16 || inp->isEnd())
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: find unknown id: %d\n", wh));
        return false;
      }
      QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: find %s entry\n", what[wh]));
      if (wh > 2 && wh != 9)
      {
        inp->seek(8, librevenge::RVNG_SEEK_CUR);
        continue;
      }
      long entryPos = long(inp->readULong(4));
      unsigned long entrySize = inp->readULong(4);
      if (entrySize == 0)
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: %s entry is empty\n", what[wh]));
        continue;
      }
      if (entryPos <= pos || entrySize == 0)
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: find bad entry pos\n"));
        return false;
      }
      /* try to read the data */
      if (inp->seek(entryPos, librevenge::RVNG_SEEK_SET) != 0)
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: can not seek entry pos %lx\n", static_cast<long unsigned int>(entryPos)));
        return false;
      }
      unsigned long numBytesRead = 0;
      const unsigned char *data = inp->read(entrySize, numBytesRead);
      if (numBytesRead != entrySize || !data)
      {
        QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: can not read %lX byte\n", static_cast<long unsigned int>(entryPos)));
        return false;
      }
      if (wh==1)
        dataInput.reset(new QXPMemoryStream(data, static_cast<unsigned int>(numBytesRead)));
      else if (wh==2)
        rsrcInput.reset(new QXPMemoryStream(data, static_cast<unsigned int>(numBytesRead)));
      else   // the finder info
      {
        if (entrySize < 8)
        {
          QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: finder info size is odd\n"));
        }
        else
        {
          bool ok = true;
          std::string type(""), creator("");
          for (int p = 0; p < 4; p++)
          {
            if (!data[p])
            {
              ok = false;
              break;
            }
            type += char(data[p]);
          }
          for (int p = 4; ok && p < 8; p++)
          {
            if (!data[p])
            {
              ok = false;
              break;
            }
            creator += char(data[p]);
          }
          if (ok)
          {
            m_fInfoType = type;
            m_fInfoCreator = creator;
          }
          else if (type.length())
          {
            QXP_DEBUG_MSG(("MWAWInputStream::unMacMIME: can not read find info\n"));
          }
        }
      }

      inp->seek(pos+12, librevenge::RVNG_SEEK_SET);
    }
  }
  catch (...)
  {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////
//
// OLE part
//
////////////////////////////////////////////////////////////

bool MWAWInputStream::isStructured()
{
  if (!m_stream) return false;
  long pos=m_stream->tell();
  bool ok=m_stream->isStructured();
  m_stream->seek(pos, librevenge::RVNG_SEEK_SET);
  return ok;
}

unsigned MWAWInputStream::subStreamCount()
{
  if (!m_stream || !m_stream->isStructured())
  {
    QXP_DEBUG_MSG(("MWAWInputStream::subStreamCount: called on unstructured file\n"));
    return 0;
  }
  return m_stream->subStreamCount();
}

std::string MWAWInputStream::subStreamName(unsigned id)
{
  if (!m_stream || !m_stream->isStructured())
  {
    QXP_DEBUG_MSG(("MWAWInputStream::subStreamName: called on unstructured file\n"));
    return std::string("");
  }
  char const *nm=m_stream->subStreamName(id);
  if (!nm)
  {
    QXP_DEBUG_MSG(("MWAWInputStream::subStreamName: can not find stream %d\n", int(id)));
    return std::string("");
  }
  return std::string(nm);
}

std::shared_ptr<MWAWInputStream> MWAWInputStream::getSubStreamByName(std::string const &name)
{
  std::shared_ptr<MWAWInputStream> empty;
  if (!m_stream || !m_stream->isStructured() || name.empty())
  {
    QXP_DEBUG_MSG(("MWAWInputStream::getSubStreamByName: called on unstructured file\n"));
    return empty;
  }

  long actPos = tell();
  seek(0, librevenge::RVNG_SEEK_SET);
  std::shared_ptr<librevenge::RVNGInputStream> res(m_stream->getSubStreamByName(name.c_str()));
  seek(actPos, librevenge::RVNG_SEEK_SET);

  if (!res)
    return empty;
  std::shared_ptr<MWAWInputStream> inp(new MWAWInputStream(res, m_inverseRead));
  inp->seek(0, librevenge::RVNG_SEEK_SET);
  return inp;
}

std::shared_ptr<MWAWInputStream> MWAWInputStream::getSubStreamById(unsigned id)
{
  std::shared_ptr<MWAWInputStream> empty;
  if (!m_stream || !m_stream->isStructured())
  {
    QXP_DEBUG_MSG(("MWAWInputStream::getSubStreamById: called on unstructured file\n"));
    return empty;
  }

  long actPos = tell();
  seek(0, librevenge::RVNG_SEEK_SET);
  std::shared_ptr<librevenge::RVNGInputStream> res(m_stream->getSubStreamById(id));
  seek(actPos, librevenge::RVNG_SEEK_SET);

  if (!res)
    return empty;
  std::shared_ptr<MWAWInputStream> inp(new MWAWInputStream(res, m_inverseRead));
  inp->seek(0, librevenge::RVNG_SEEK_SET);
  return inp;
}

////////////////////////////////////////////////////////////
//
//  a function to read a data block
//
////////////////////////////////////////////////////////////

bool MWAWInputStream::readDataBlock(long sz, librevenge::RVNGBinaryData &data)
{
  if (!hasDataFork()) return false;

  data.clear();
  if (sz < 0) return false;
  if (sz == 0) return true;
  long endPos=tell()+sz;
  if (endPos > size()) return false;
  if (m_readLimit > 0 && endPos > long(m_readLimit)) return false;

  const unsigned char *readData;
  unsigned long sizeRead;
  if ((readData=m_stream->read(static_cast<unsigned long>(sz), sizeRead)) == 0 || long(sizeRead)!=sz)
    return false;
  data.append(readData, sizeRead);
  return true;
}

bool MWAWInputStream::readEndDataBlock(librevenge::RVNGBinaryData &data)
{
  data.clear();
  if (!hasDataFork()) return false;

  long endPos=m_readLimit>0 ? m_readLimit : size();
  return readDataBlock(endPos-tell(), data);
}

}

namespace libqxp
{

QXPMacFileParser::QXPMacFileParser(std::shared_ptr<librevenge::RVNGInputStream> &dataFork, std::string &type, std::string &creator)
  : m_dataFork(dataFork)
  , m_type(type)
  , m_creator(creator)
{
}

bool QXPMacFileParser::parse(const std::shared_ptr<librevenge::RVNGInputStream> &input)
{
  MWAWInputStream strm(input.get(), false, true);
  m_dataFork = strm.input();
  return strm.hasDataFork() && strm.getFinderInfo(m_type, m_creator);
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
