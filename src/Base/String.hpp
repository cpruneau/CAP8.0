/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__String
#define CAP__String
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <string>
#include <type_traits>
#include <utility>

namespace CAP
{

//!
//! @brief Character string of CAP: a light replacement for ROOT's TString, built on std::string.
//!
//! String holds a std::string (it does not inherit from it) and offers the operations used throughout CAP, with the
//! behaviour of their TString counterparts but CAP-style names (equalTo(), beginsWith(), atof(), ...). It depends on the
//! standard library only.
//!
//! <b>Appending numbers.</b> As with TString, appending a number appends its decimal representation:
//! @code
//!   CAP::String key = "PARTICLE_";
//!   key += 3;          // "PARTICLE_3" (std::string would append the character of code 3)
//!   key += 2.5;        // "PARTICLE_32.5" (doubles are written with 17 significant digits, as TString does)
//!   key += true;       // "PARTICLE_32.51"
//! @endcode
//! Characters (char) are appended as characters.
//!
//! <b>Conversions.</b> A String converts implicitly to const char*, so it can be passed to C and ROOT functions
//! (TH1 constructors, std::ifstream, ...). str() gives the underlying std::string.
//!
//! <b>Numbers in strings.</b> isDec(), isDigit(), isFloat(), atoi(), atoll() and atof() follow the TString rules
//! (see each function). isInteger() also accepts a leading sign.
//!
//! Positions are indices from 0; functions that search return kNPOS (-1) when nothing is found.
//!
class String
{
public:

  //! @brief Value returned by the search functions when nothing is found.
  static const int kNPOS = -1;

  //! @name Construction
  //! @{
  //! @brief Empty string.
  String() : _s() {}
  //! @brief Copy of a C string (an empty string if s is null). @param s C string
  String(const char * s) : _s(s ? s : "") {}
  //! @brief Copy of the first n characters of a C string. @param s characters @param n number of characters
  String(const char * s, std::size_t n) : _s(s ? std::string(s,n) : std::string()) {}
  //! @brief Copy of a std::string. @param s string
  String(const std::string & s) : _s(s) {}
  //! @brief Takes a std::string. @param s string
  String(std::string && s) : _s(std::move(s)) {}
  //!
  //! @brief Copy of any string type with a Data() member returning the characters, e.g. ROOT's TString, so that such
  //! strings can be passed where a String is expected (String does not depend on ROOT).
  //! @param t string
  //!
  template <class T, class = typename std::enable_if<std::is_convertible<decltype(std::declval<const T &>().Data()),const char *>::value>::type>
  String(const T & t) : _s(t.Data() ? t.Data() : "") {}
  //! @brief String of one character. @param c character
  explicit String(char c) : _s(1,c) {}
  //! @brief String of n copies of a character. @param c character @param n number of copies
  String(char c, std::size_t n) : _s(n,c) {}
  //! @brief Copy constructor.
  String(const String &) = default;
  //! @brief Move constructor.
  String(String &&) = default;
  //! @brief Copy assignment.
  String & operator=(const String &) = default;
  //! @brief Move assignment.
  String & operator=(String &&) = default;
  //! @brief Assign a C string (empty if null). @param s C string @return this string
  String & operator=(const char * s) { _s = s ? s : ""; return *this; }
  //! @brief Assign a std::string. @param s string @return this string
  String & operator=(const std::string & s) { _s = s; return *this; }
  //! @brief Assign one character. @param c character @return this string
  String & operator=(char c) { _s.assign(1,c); return *this; }
  //! @brief Destructor.
  ~String() = default;
  //! @}

  //! @name Access
  //! @{
  //! @brief Characters, null-terminated. @return C string (valid until the string changes)
  const char * data() const { return _s.c_str(); }
  //! @brief Characters, null-terminated (same as data()). @return C string
  const char * c_str() const { return _s.c_str(); }
  //! @brief Conversion to a C string. @return data()
  operator const char * () const { return _s.c_str(); }
  //! @brief Underlying std::string. @return string
  const std::string & str() const { return _s; }
  //! @brief Underlying std::string (modifiable). @return string
  std::string & str() { return _s; }
  //! @brief Number of characters. @return length
  int length() const { return int(_s.size()); }
  //! @brief Whether the string is empty. @return true if it has no character
  bool isNull() const { return _s.empty(); }
  //! @brief Whether the string is empty. @return true if it has no character
  bool isEmpty() const { return _s.empty(); }
  //! @brief Character at a position (not checked). @param i position @return character
  char operator[](int i) const { return _s[std::size_t(i)]; }
  //! @brief Character at a position (not checked). @param i position @return character
  char & operator[](int i) { return _s[std::size_t(i)]; }
  //! @brief Character at a position (not checked). @param i position @return character
  char operator[](unsigned int i) const { return _s[i]; }
  //! @brief Character at a position (not checked). @param i position @return character
  char & operator[](unsigned int i) { return _s[i]; }
  //! @brief Character at a position (not checked). @param i position @return character
  char operator[](long i) const { return _s[std::size_t(i)]; }
  //! @brief Character at a position (not checked). @param i position @return character
  char & operator[](long i) { return _s[std::size_t(i)]; }
  //! @brief Character at a position (not checked). @param i position @return character
  char operator[](unsigned long i) const { return _s[i]; }
  //! @brief Character at a position (not checked). @param i position @return character
  char & operator[](unsigned long i) { return _s[i]; }
  //! @brief Substring (as TString::operator()). @param start first position @param n number of characters
  //! @return substring (empty if start is beyond the end)
  String operator()(int start, int n) const { return subString(start,n); }
  //! @brief Substring. @param start first position @param n number of characters (to the end if negative or too large)
  //! @return substring (empty if start is beyond the end)
  String subString(int start, int n=-1) const;
  //! @}

  //! @name Comparison and search
  //! @{
  //! @brief Whether the string equals s. @param s string @param ignoreCase compare without case @return true if equal
  bool equalTo(const char * s, bool ignoreCase=false) const;
  //! @brief Whether the string equals s. @param s string @param ignoreCase compare without case @return true if equal
  bool equalTo(const String & s, bool ignoreCase=false) const { return equalTo(s.data(),ignoreCase); }
  //! @brief Whether the string begins with s. @param s prefix @param ignoreCase compare without case @return true if so
  bool beginsWith(const char * s, bool ignoreCase=false) const;
  //! @brief Whether the string begins with s. @param s prefix @param ignoreCase compare without case @return true if so
  bool beginsWith(const String & s, bool ignoreCase=false) const { return beginsWith(s.data(),ignoreCase); }
  //! @brief Whether the string ends with s. @param s suffix @param ignoreCase compare without case @return true if so
  bool endsWith(const char * s, bool ignoreCase=false) const;
  //! @brief Whether the string ends with s. @param s suffix @param ignoreCase compare without case @return true if so
  bool endsWith(const String & s, bool ignoreCase=false) const { return endsWith(s.data(),ignoreCase); }
  //! @brief Whether the string contains s. @param s string @param ignoreCase compare without case @return true if so
  bool contains(const char * s, bool ignoreCase=false) const { return index(s,0,ignoreCase)!=kNPOS; }
  //! @brief Whether the string contains s. @param s string @param ignoreCase compare without case @return true if so
  bool contains(const String & s, bool ignoreCase=false) const { return contains(s.data(),ignoreCase); }
  //! @brief Whether the string contains a character. @param c character @return true if so
  bool contains(char c) const { return _s.find(c)!=std::string::npos; }
  //! @brief Position of the first occurrence of s at or after start. @param s string @param start first position searched
  //! @param ignoreCase compare without case @return position, or kNPOS
  int index(const char * s, int start=0, bool ignoreCase=false) const;
  //! @brief Position of the first occurrence of s at or after start. @param s string @param start first position searched
  //! @param ignoreCase compare without case @return position, or kNPOS
  int index(const String & s, int start=0, bool ignoreCase=false) const { return index(s.data(),start,ignoreCase); }
  //! @brief Position of the first occurrence of a character. @param c character @return position, or kNPOS
  int first(char c) const;
  //! @brief Position of the first character that belongs to a set (as TString::First(const char*)).
  //! @param set characters looked for @return position, or kNPOS
  int first(const char * set) const;
  //! @brief Position of the last occurrence of a character. @param c character @return position, or kNPOS
  int last(char c) const;
  //! @brief Number of occurrences of a character. @param c character @return count
  int countChar(char c) const;
  //! @brief Three-way comparison. @param s string @param ignoreCase compare without case
  //! @return negative, 0 or positive as this string is before, equal to or after s
  int compareTo(const char * s, bool ignoreCase=false) const;
  //! @}

  //! @name Modification (in place; each returns this string)
  //! @{
  //! @brief Append a C string. @param s C string (nothing if null) @return this string
  String & append(const char * s) { if (s) _s += s; return *this; }
  //! @brief Append a string. @param s string @return this string
  String & append(const String & s) { _s += s._s; return *this; }
  //! @brief Append copies of a character. @param c character @param n number of copies @return this string
  String & append(char c, int n=1) { if (n>0) _s.append(std::size_t(n),c); return *this; }
  //! @brief Prepend a string. @param s string @return this string
  String & prepend(const char * s) { if (s) _s.insert(0,s); return *this; }
  //! @brief Insert a string. @param pos position (clamped to the length) @param s string @return this string
  String & insert(int pos, const char * s);
  //! @brief Remove the characters from pos to the end. @param pos first position removed @return this string
  String & remove(int pos);
  //! @brief Remove n characters from pos. @param pos first position removed @param n number of characters @return this string
  String & remove(int pos, int n);
  //! @brief Replace n characters from pos by s. @param pos position @param n number of characters replaced
  //! @param s replacement @return this string
  String & replace(int pos, int n, const char * s);
  //! @brief Replace every occurrence of s1 by s2. @param s1 string replaced @param s2 replacement @return this string
  String & replaceAll(const char * s1, const char * s2);
  //! @brief Convert to upper case. @return this string
  String & toUpper();
  //! @brief Convert to lower case. @return this string
  String & toLower();
  //! @brief Remove leading and trailing white space. @return this string
  String & strip();
  //! @brief Remove all characters. @return this string
  String & clear() { _s.clear(); return *this; }
  //! @}

  //! @name Numbers (TString rules)
  //! @{
  //! @brief Whether all characters are decimal digits (no sign, no space). @return false for an empty string
  bool isDec() const;
  //! @brief Whether all characters are digits or spaces, with at least one digit. @return true if so
  bool isDigit() const;
  //! @brief Whether the string is an integer: optional sign followed by decimal digits. @return true if so
  bool isInteger() const;
  //! @brief Whether the string is a floating-point number (as TString::IsFloat(): digits with optionally one '.' or ',',
  //! one exponent and a leading sign; spaces allowed). @return true if so
  bool isFloat() const;
  //! @brief Integer value, spaces removed (as TString::Atoi(): the leading integer part, 0 if none). @return value
  int atoi() const;
  //! @brief Integer value, spaces removed (as TString::Atoll()). @return value
  long long atoll() const;
  //! @brief Floating-point value (as TString::Atof(): a ',' is read as '.', spaces are removed, 0 if not a number).
  //! @return value
  double atof() const;
  //! @}

  //! @name Appending (numbers are appended as decimal text)
  //! @{
  //! @brief Append a C string. @param s C string @return this string
  String & operator+=(const char * s) { return append(s); }
  //! @brief Append a string. @param s string @return this string
  String & operator+=(const String & s) { return append(s); }
  //! @brief Append a std::string. @param s string @return this string
  String & operator+=(const std::string & s) { _s += s; return *this; }
  //! @brief Append a character. @param c character @return this string
  String & operator+=(char c) { _s += c; return *this; }
  //! @brief Append the decimal representation of an integer (bool: 1 or 0). @param v number @return this string
  template <class T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T,char>::value, String &>::type
  operator+=(T v)
  {
  if (std::is_signed<T>::value) _s += std::to_string((long long)v);
  else                          _s += std::to_string((unsigned long long)v);
  return *this;
  }
  //! @brief Append a floating-point number with 17 significant digits (format %.17g, as TString). @param v number
  //! @return this string
  template <class T>
  typename std::enable_if<std::is_floating_point<T>::value, String &>::type
  operator+=(T v) { return appendFloat((long double)v); }
  //! @}

  //! @brief Hash code (for unordered containers). @return hash
  std::size_t hash() const { return std::hash<std::string>()(_s); }

protected:

  //! @brief Append a number written with %.17Lg. @param v number @return this string
  String & appendFloat(long double v);

  std::string _s; //!< characters
};

//! @name Concatenation (numbers are appended as decimal text)
//! @{
inline String operator+(const String & a, const String & b)     { String r(a); r += b; return r; }
inline String operator+(const String & a, const char * b)       { String r(a); r += b; return r; }
inline String operator+(const char * a, const String & b)       { String r(a); r += b; return r; }
inline String operator+(const String & a, const std::string & b){ String r(a); r += b; return r; }
inline String operator+(const std::string & a, const String & b){ String r(a); r += b; return r; }
inline String operator+(const String & a, char b)               { String r(a); r += b; return r; }
inline String operator+(char a, const String & b)               { String r(a); r += b; return r; }
template <class T>
typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T,char>::value, String>::type
operator+(const String & a, T b) { String r(a); r += b; return r; }
template <class T>
typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T,char>::value, String>::type
operator+(T a, const String & b) { String r; r += a; r += b; return r; }
//! @}

//! @name Comparison
//! @{
inline bool operator==(const String & a, const String & b) { return a.str()==b.str(); }
inline bool operator==(const String & a, const char * b)   { return b && a.str()==b; }
inline bool operator==(const char * a, const String & b)   { return a && b.str()==a; }
inline bool operator==(const String & a, const std::string & b) { return a.str()==b; }
inline bool operator==(const std::string & a, const String & b) { return a==b.str(); }
inline bool operator!=(const String & a, const String & b) { return !(a==b); }
inline bool operator!=(const String & a, const char * b)   { return !(a==b); }
inline bool operator!=(const char * a, const String & b)   { return !(a==b); }
inline bool operator!=(const String & a, const std::string & b) { return !(a==b); }
inline bool operator!=(const std::string & a, const String & b) { return !(a==b); }
inline bool operator< (const String & a, const String & b) { return a.str()< b.str(); }
inline bool operator< (const String & a, const char * b)   { return a.str()< std::string(b?b:""); }
inline bool operator< (const char * a, const String & b)   { return std::string(a?a:"")< b.str(); }
inline bool operator> (const String & a, const String & b) { return b<a; }
inline bool operator<=(const String & a, const String & b) { return !(b<a); }
inline bool operator>=(const String & a, const String & b) { return !(a<b); }
//! @}

//! @brief Write a string to a stream. @param os stream @param s string @return os
std::ostream & operator<<(std::ostream & os, const String & s);
//! @brief Read a white-space delimited word from a stream. @param is stream @param s string read @return is
std::istream & operator>>(std::istream & is, String & s);

} // namespace CAP

namespace std
{
  //! @brief Hash of a CAP::String, for std::unordered_map and std::unordered_set.
  template <> struct hash<CAP::String>
  {
    std::size_t operator()(const CAP::String & s) const { return s.hash(); }
  };
}

#endif
