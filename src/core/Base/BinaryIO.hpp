/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BinaryIO.hpp
//! @brief Binary encoding of CAP objects: BinaryWriter, BinaryReader, CRC-32 and the CAP record format.
#ifndef CAP__BinaryIO
#define CAP__BinaryIO
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <istream>
#include <ostream>
#include <fstream>
#include "Aliases.hpp"
#include "Exceptions.hpp"

namespace CAP
{

//!
//! @brief Minimal, portable binary encoding used to save objects to files, independent of ROOT.
//!
//! All multi-byte values are written in little-endian byte order, byte by byte, so files are portable across
//! platforms and compilers. Encodings:
//!  - unsigned integers: LEB128 variable-length ("varint"): 7 bits per byte, 1 byte for values < 128
//!  - signed integers: zigzag mapping (0,-1,1,-2,... -> 0,1,2,3,...) then varint, so small negative numbers stay short
//!  - float32 / float64: IEEE 754 bit pattern, 4 / 8 bytes, little-endian
//!  - strings: varint length followed by the bytes
//! crc32() computes the standard CRC-32 (IEEE 802.3, as used by zip and png) to detect corrupted data.
//!
class BinaryWriter
{
public:
  //! @brief Bytes written so far. @return buffer
  const std::string & buffer() const { return _buffer; }
  //! @brief Write one byte. @param v value
  void writeUInt8(uint8_t v) { _buffer.push_back(char(v)); }
  //! @brief Write 4 bytes, little-endian. @param v value
  void writeUInt32(uint32_t v) { for (int i=0; i<4; i++) writeUInt8(uint8_t(v>>(8*i))); }
  //! @brief Write 8 bytes, little-endian. @param v value
  void writeUInt64(uint64_t v) { for (int i=0; i<8; i++) writeUInt8(uint8_t(v>>(8*i))); }
  //! @brief Write an unsigned integer as a varint. @param v value
  void writeVarUInt(uint64_t v) { while (v>=0x80) { writeUInt8(uint8_t(v|0x80)); v >>= 7; } writeUInt8(uint8_t(v)); }
  //! @brief Write a signed integer as a zigzag varint. @param v value
  void writeVarInt(int64_t v) { writeVarUInt((uint64_t(v)<<1) ^ uint64_t(v>>63)); }
  //! @brief Write a float as 4 bytes (IEEE 754). @param v value
  void writeFloat32(float v) { uint32_t u; std::memcpy(&u,&v,4); writeUInt32(u); }
  //! @brief Write a double as 8 bytes (IEEE 754). @param v value
  void writeFloat64(double v) { uint64_t u; std::memcpy(&u,&v,8); writeUInt64(u); }
  //! @brief Write raw bytes. @param data bytes @param n number of bytes
  void writeBytes(const char * data, std::size_t n) { _buffer.append(data,n); }
  //! @brief Write a string (varint length + bytes). @param s string
  void writeString(const String & s) { writeVarUInt(std::size_t(s.length())); writeBytes(s.data(),std::size_t(s.length())); }

private:
  std::string _buffer;   //!< bytes written so far
};

//!
//! @brief Reads the encodings of BinaryWriter from a memory buffer; every read is bounds-checked.
//!
class BinaryReader
{
public:
  //! @brief Reader over n bytes at data (not copied: must outlive the reader). @param data bytes @param n number of bytes
  BinaryReader(const char * data, std::size_t n) : _data(data), _size(n), _position(0) {}
  //! @brief Number of bytes not yet read. @return remaining bytes
  std::size_t remaining() const { return _size-_position; }
  //! @brief Read one byte. @return value @throws Exception at the end of the buffer
  uint8_t readUInt8() { need(1); return uint8_t(_data[_position++]); }
  //! @brief Read 4 bytes, little-endian. @return value
  uint32_t readUInt32() { need(4); uint32_t v=0; for (int i=0; i<4; i++) v |= uint32_t(uint8_t(_data[_position++]))<<(8*i); return v; }
  //! @brief Read 8 bytes, little-endian. @return value
  uint64_t readUInt64() { need(8); uint64_t v=0; for (int i=0; i<8; i++) v |= uint64_t(uint8_t(_data[_position++]))<<(8*i); return v; }
  //! @brief Read a varint. @return value @throws Exception if malformed
  uint64_t readVarUInt()
  {
    uint64_t v = 0;
    for (int shift=0; shift<64; shift+=7)
      {
      uint8_t b = readUInt8();
      v |= uint64_t(b & 0x7F) << shift;
      if (!(b & 0x80)) return v;
      }
    throw Exception("Malformed variable-length integer","BinaryReader::readVarUInt()");
  }
  //! @brief Read a zigzag varint. @return value
  int64_t readVarInt() { uint64_t u = readVarUInt(); return int64_t(u>>1) ^ -int64_t(u & 1); }
  //! @brief Read a float (4 bytes). @return value
  float readFloat32() { uint32_t u = readUInt32(); float v; std::memcpy(&v,&u,4); return v; }
  //! @brief Read a double (8 bytes). @return value
  double readFloat64() { uint64_t u = readUInt64(); double v; std::memcpy(&v,&u,8); return v; }
  //! @brief Read a string. @return value
  String readString()
  {
    uint64_t n = readVarUInt();
    need(n);
    std::string s(_data+_position,std::size_t(n));
    _position += std::size_t(n);
    return String(s.c_str());
  }
  //! @brief Take the next n bytes without copying them. @param n number of bytes @return pointer to them, valid as long
  //! as the data of the reader
  const char * readSpan(std::size_t n) { need(n); const char * p = _data+_position; _position += n; return p; }

private:
  //! @brief Throw unless n more bytes can be read. @param n number of bytes @throws Exception at the end of the data
  void need(uint64_t n) const
  {
    if (n > remaining()) throw Exception("Unexpected end of binary data","BinaryReader");
  }
  const char * _data;   //!< data read (not owned)
  std::size_t  _size;   //!< number of bytes of the data
  std::size_t  _position;   //!< position of the next byte read
};

inline uint32_t crc32Update(uint32_t state, const char * data, std::size_t n);

//!
//! @brief Standard CRC-32 (polynomial 0xEDB88320, as used by zip, gzip and png) of n bytes.
//! @param data bytes @param n number of bytes @return checksum
//!
inline uint32_t crc32(const char * data, std::size_t n)
{
  return crc32Update(0xFFFFFFFFu,data,n) ^ 0xFFFFFFFFu;
}

//!
//! @brief Incremental CRC-32: crc32(data,n) is crc32Update(0xFFFFFFFF, data, n) ^ 0xFFFFFFFF, and the data can be
//! given in pieces, each call continuing from the state returned by the previous one.
//! @param state 0xFFFFFFFF to start, then the value returned by the previous call @param data bytes @param n number of
//! bytes @return new state
//!
inline uint32_t crc32Update(uint32_t state, const char * data, std::size_t n)
{
  // "slicing by 8": 8 tables, built once on first use (thread-safe initialization of a function-local static), let
  // the loop take 8 bytes at a time; t[0] is the usual byte table
  static const std::array<std::array<uint32_t,256>,8> t = []
    {
    std::array<std::array<uint32_t,256>,8> tables{};
    for (uint32_t i=0; i<256; i++)
      {
      uint32_t c = i;
      for (int k=0; k<8; k++) c = (c & 1) ? 0xEDB88320u ^ (c>>1) : (c>>1);
      tables[0][i] = c;
      }
    for (uint32_t i=0; i<256; i++)
      for (int k=1; k<8; k++) tables[k][i] = (tables[k-1][i]>>8) ^ tables[0][tables[k-1][i] & 0xFF];
    return tables;
    }();
  const uint8_t * p = reinterpret_cast<const uint8_t *>(data);
  uint32_t crc = state;
  for (; n>=8; n-=8, p+=8)
    {
    uint32_t low  = crc ^ (uint32_t(p[0]) | uint32_t(p[1])<<8 | uint32_t(p[2])<<16 | uint32_t(p[3])<<24);
    uint32_t high =        uint32_t(p[4]) | uint32_t(p[5])<<8 | uint32_t(p[6])<<16 | uint32_t(p[7])<<24;
    crc = t[7][low & 0xFF] ^ t[6][(low>>8) & 0xFF] ^ t[5][(low>>16) & 0xFF] ^ t[4][low>>24]
        ^ t[3][high & 0xFF] ^ t[2][(high>>8) & 0xFF] ^ t[1][(high>>16) & 0xFF] ^ t[0][high>>24];
    }
  for (; n>0; n--, p++) crc = t[0][(crc ^ *p) & 0xFF] ^ (crc>>8);
  return crc;
}

//! @name Arrays of doubles
//! A flag byte selects the smaller of two layouts: dense (count, then every value as float64), or sparse
//! (count, number of non-zero values, then for each non-zero value the gap in index since the previous one and the
//! value). Values are restored exactly, including -0.0 and NaN.
//! @{

//! @brief Write an array of doubles (dense or sparse, whichever is smaller). @param w writer @param values values
inline void writeDoubles(BinaryWriter & w, const std::vector<double> & values)
{
  auto isZero = [](double v) { uint64_t u; std::memcpy(&u,&v,8); return u==0; };   // +0.0 only
  uint64_t nonZero = 0;
  for (double v : values) if (!isZero(v)) nonZero++;
  w.writeVarUInt(values.size());
  if (nonZero*10 < uint64_t(values.size())*8)
    {
    w.writeUInt8(1);
    w.writeVarUInt(nonZero);
    std::size_t next = 0;
    for (std::size_t k=0; k<values.size(); k++)
      {
      if (isZero(values[k])) continue;
      w.writeVarUInt(k-next);
      w.writeFloat64(values[k]);
      next = k+1;
      }
    }
  else
    {
    w.writeUInt8(0);
    for (double v : values) w.writeFloat64(v);
    }
}

//! @brief Read an array of doubles written by writeDoubles(). @param r reader @return values @throws Exception if malformed
inline std::vector<double> readDoubles(BinaryReader & r)
{
  uint64_t n = r.readVarUInt();
  uint8_t layout = r.readUInt8();
  if (layout==0)
    {
    if (n > r.remaining()/8) throw Exception("Array larger than the data","readDoubles()");
    std::vector<double> values(static_cast<std::size_t>(n));
    for (auto & v : values) v = r.readFloat64();
    return values;
    }
  if (layout!=1) throw Exception("Unknown array layout","readDoubles()");
  uint64_t nonZero = r.readVarUInt();
  if (nonZero > n || nonZero > r.remaining()/9 || n > (uint64_t(1)<<32)) throw Exception("Invalid sparse array","readDoubles()");
  std::vector<double> values(std::size_t(n),0.0);
  uint64_t k = 0;
  for (uint64_t i=0; i<nonZero; i++)
    {
    k += r.readVarUInt();
    if (k>=n) throw Exception("Array index out of range","readDoubles()");
    values[std::size_t(k)] = r.readFloat64();
    k++;
    }
  return values;
}

//! @}

//! @name Binary records and files
//!
//! Objects are saved as records, one after the other in a file or stream:
//!   tag (4 bytes, "CAP" + one letter identifying the class), format version (1 byte), payload length (varint),
//!   payload, CRC-32 of the payload (4 bytes, little-endian).
//! The payload always starts with the object's name (a string, empty if the class has no name).
//! Records of other classes are skipped when reading, so one file can hold objects of several classes.
//!
//! A class T saved with the templates below provides:
//!  - static const char * binaryTag()        : its 4-character tag
//!  - static unsigned int binaryVersion()    : the format version it writes (and the highest it can read)
//!  - String binaryName() const               : the name stored in the record ("" if none)
//!  - void encodeBinary(BinaryWriter &) const : writes the payload after the name
//!  - void decodeBinary(BinaryReader &, const String & name, unsigned int version) : rebuilds the object from it,
//!    throwing Exception if the data is invalid (it is called on a fresh, default-constructed object)
//! @{

//! @brief Write one record. @param out stream @param tag 4-character tag @param version format version
//! @param payload payload bytes @param where file or object name, for error messages
inline void writeBinaryRecord(std::ostream & out, const char * tag, unsigned int version, const std::string & payload, const String & where)
{
  BinaryWriter header;
  header.writeBytes(tag,4);
  header.writeUInt8(uint8_t(version));
  header.writeVarUInt(payload.size());
  BinaryWriter trailer;
  trailer.writeUInt32(crc32(payload.data(),payload.size()));
  out.write(header.buffer().data(),std::streamsize(header.buffer().size()));
  out.write(payload.data(),std::streamsize(payload.size()));
  out.write(trailer.buffer().data(),std::streamsize(trailer.buffer().size()));
  if (!out) throw FileException(where,"Error writing binary record","writeBinaryRecord()");
}

//!
//! @brief Read the next record, of any class.
//! @param in stream @param tag output: 4-character tag @param version output: format version @param payload output: payload
//! @param where file or object name, for error messages
//! @return false if the stream is at its end before the record starts
//! @throws FileException if the data is not a CAP record, or is truncated or corrupted
//!
inline bool readBinaryRecord(std::istream & in, std::string & tag, unsigned int & version, std::string & payload, const String & where)
{
  char t[4];
  in.read(t,4);
  if (in.gcount()==0 && in.eof()) return false;
  if (in.gcount()!=4 || t[0]!='C' || t[1]!='A' || t[2]!='P')
    throw FileException(where,"Not a CAP binary record (bad tag)","readBinaryRecord()");
  tag.assign(t,4);
  int v = in.get();
  if (v==EOF) throw FileException(where,"Truncated binary record","readBinaryRecord()");
  version = unsigned(v);
  uint64_t length = 0;
  for (int shift=0; ; shift+=7)
    {
    int b = in.get();
    if (b==EOF || shift>=64) throw FileException(where,"Truncated or malformed binary record","readBinaryRecord()");
    length |= uint64_t(b & 0x7F) << shift;
    if (!(b & 0x80)) break;
    }
  if (length > (uint64_t(1)<<40)) throw FileException(where,"Implausible binary record length","readBinaryRecord()");
  payload.resize(std::size_t(length));
  if (length>0) in.read(&payload[0],std::streamsize(length));
  char c[4];
  in.read(c,4);
  if (!in) throw FileException(where,"Truncated binary record","readBinaryRecord()");
  BinaryReader crcReader(c,4);
  if (crcReader.readUInt32()!=crc32(payload.data(),payload.size()))
    throw FileException(where,"Corrupted binary record (checksum mismatch)","readBinaryRecord()");
  return true;
}

//!
//! @brief Read one record held in memory, without copying its payload.
//! @param r reader positioned at the start of the record @param tag output: 4-character tag @param version output:
//! format version @param payload output: start of the payload (inside the reader's data) @param length output: payload
//! length @param where file or object name, for error messages
//! @throws FileException if the data is not a CAP record, or is truncated or corrupted
//!
inline void readBinaryRecord(BinaryReader & r, std::string & tag, unsigned int & version, const char * & payload,
                             std::size_t & length, const String & where)
{
  try
    {
    const char * t = r.readSpan(4);
    if (t[0]!='C' || t[1]!='A' || t[2]!='P') throw FileException(where,"Not a CAP binary record (bad tag)","readBinaryRecord()");
    tag.assign(t,4);
    version = r.readUInt8();
    uint64_t n = r.readVarUInt();
    if (n > r.remaining()) throw FileException(where,"Truncated binary record","readBinaryRecord()");
    length  = std::size_t(n);
    payload = r.readSpan(length);
    if (r.readUInt32()!=crc32(payload,length))
      throw FileException(where,"Corrupted binary record (checksum mismatch)","readBinaryRecord()");
    }
  catch (FileException &)
    {
    throw;
    }
  catch (Exception &)
    {
    throw FileException(where,"Truncated or malformed binary record","readBinaryRecord()");
    }
}

//! @brief Name stored at the start of a payload. @param payload payload @return name
inline String binaryPayloadName(const std::string & payload)
{
  BinaryReader r(payload.data(),payload.size());
  return r.readString();
}

//! @brief Decode a payload of class T into object (unchanged if the data is invalid). @throws FileException if invalid
template <typename T>
void decodeBinaryPayload(const std::string & payload, unsigned int version, T & object, const String & where)
{
  if (version==0 || version>T::binaryVersion())
    throw FileException(where,"Binary record of an unsupported format version (written by a newer version?)","decodeBinaryPayload()");
  try
    {
    BinaryReader r(payload.data(),payload.size());
    String name = r.readString();
    T decoded;
    decoded.decodeBinary(r,name,version);
    if (r.remaining()!=0) throw Exception("Unexpected data at the end of the record","decodeBinaryPayload()");
    object = std::move(decoded);   // a copy for classes without move assignment
    }
  catch (FileException &)
    {
    throw;
    }
  catch (Exception & e)
    {
    throw FileException(where,String("Invalid binary record: ")+e.what(),"decodeBinaryPayload()");
    }
}

//! @brief Write object as one record to a stream (open in binary mode). @throws FileException if writing fails
template <typename T>
void writeBinaryObject(std::ostream & out, const T & object)
{
  BinaryWriter w;
  w.writeString(object.binaryName());
  object.encodeBinary(w);
  writeBinaryRecord(out,T::binaryTag(),T::binaryVersion(),w.buffer(),object.binaryName());
}

//!
//! @brief Read the next record of class T from a stream into object, skipping records of other classes.
//! @return false at the end of the stream (object unchanged) @throws FileException if the data is invalid (object unchanged)
//!
template <typename T>
bool readBinaryObject(std::istream & in, T & object)
{
  std::string tag, payload;
  unsigned int version;
  while (readBinaryRecord(in,tag,version,payload,object.binaryName()))
    {
    if (tag!=T::binaryTag()) continue;
    decodeBinaryPayload(payload,version,object,object.binaryName());
    return true;
    }
  return false;
}

//! @brief Save object to a file, replacing it or appending to it. @throws FileException if the file cannot be written
template <typename T>
void saveBinaryObject(const String & fileName, const T & object, bool append)
{
  std::ofstream out(fileName.data(),std::ios::binary | (append ? std::ios::app : std::ios::trunc));
  if (!out) throw FileException(fileName,"Cannot open file for writing","saveBinaryObject()");
  writeBinaryObject(out,object);
  out.close();
  if (!out) throw FileException(fileName,"Error writing file","saveBinaryObject()");
}

//! @brief Save several objects to one file (replacing it). @throws FileException if the file cannot be written
template <typename T>
void saveBinaryObjects(const String & fileName, const std::vector<T> & objects)
{
  std::ofstream out(fileName.data(),std::ios::binary | std::ios::trunc);
  if (!out) throw FileException(fileName,"Cannot open file for writing","saveBinaryObjects()");
  for (const auto & object : objects) writeBinaryObject(out,object);
  out.close();
  if (!out) throw FileException(fileName,"Error writing file","saveBinaryObjects()");
}

//!
//! @brief Load from a file the first object of class T, or the first one with the given name.
//! @throws FileException if the file cannot be read, is corrupted, or holds no such object (object unchanged)
//!
template <typename T>
void loadBinaryObject(const String & fileName, T & object, const String & name)
{
  std::ifstream in(fileName.data(),std::ios::binary);
  if (!in) throw FileException(fileName,"Cannot open file for reading","loadBinaryObject()");
  std::string tag, payload;
  unsigned int version;
  while (readBinaryRecord(in,tag,version,payload,fileName))
    {
    if (tag!=T::binaryTag()) continue;
    if (name.length()>0 && !(binaryPayloadName(payload)==name)) continue;
    decodeBinaryPayload(payload,version,object,fileName);
    return;
    }
  if (name.length()==0) throw FileException(fileName,"File contains no object of this class","loadBinaryObject()");
  throw FileException(fileName,String("No object of this class named ")+name,"loadBinaryObject()");
}

//!
//! @brief Load from a file the object of class T at position index (0 = first) among the objects of that class.
//! @throws FileException if the file cannot be read, is corrupted, or holds fewer objects (object unchanged)
//!
template <typename T>
void loadBinaryObjectAt(const String & fileName, T & object, unsigned int index)
{
  std::ifstream in(fileName.data(),std::ios::binary);
  if (!in) throw FileException(fileName,"Cannot open file for reading","loadBinaryObjectAt()");
  std::string tag, payload;
  unsigned int version;
  unsigned int count = 0;
  while (readBinaryRecord(in,tag,version,payload,fileName))
    {
    if (tag!=T::binaryTag()) continue;
    if (count++ < index) continue;
    decodeBinaryPayload(payload,version,object,fileName);
    return;
    }
  throw FileException(fileName,"File contains fewer objects of this class than requested","loadBinaryObjectAt()");
}

//! @brief Load all the objects of class T from a file, in order. @throws FileException if the file cannot be read or is corrupted
template <typename T>
std::vector<T> loadAllBinaryObjects(const String & fileName)
{
  std::ifstream in(fileName.data(),std::ios::binary);
  if (!in) throw FileException(fileName,"Cannot open file for reading","loadAllBinaryObjects()");
  std::vector<T> objects;
  T object;
  while (readBinaryObject(in,object)) objects.push_back(object);
  return objects;
}

//! @}

} // namespace CAP

#endif
