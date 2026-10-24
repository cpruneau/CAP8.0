/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "String.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <istream>
#include <ostream>

namespace CAP
{

  namespace
  {
  char lower(char c) { return char(std::tolower((unsigned char)c)); }
  bool sameChars(const char * a, const char * b, std::size_t n, bool ignoreCase)
  {
  if (!ignoreCase) return std::memcmp(a,b,n)==0;
  for (std::size_t i=0; i<n; i++) if (lower(a[i])!=lower(b[i])) return false;
  return true;
  }
  // characters of s with the spaces removed (TString::Atoi/Atoll/Atof)
  std::string withoutSpaces(const std::string & s)
  {
  std::string r;
  r.reserve(s.size());
  for (char c : s) if (c!=' ') r += c;
  return r;
  }
  }

  String String::subString(int start, int n) const
  {
  if (start<0) start = 0;
  if (std::size_t(start)>=_s.size()) return String();
  return String(_s.substr(std::size_t(start), n<0 ? std::string::npos : std::size_t(n)));
  }

  bool String::equalTo(const char * s, bool ignoreCase) const
  {
  if (!s) s = "";
  std::size_t n = std::strlen(s);
  return n==_s.size() && sameChars(_s.data(),s,n,ignoreCase);
  }

  bool String::beginsWith(const char * s, bool ignoreCase) const
  {
  if (!s) return false;
  std::size_t n = std::strlen(s);
  return n<=_s.size() && sameChars(_s.data(),s,n,ignoreCase);
  }

  bool String::endsWith(const char * s, bool ignoreCase) const
  {
  if (!s) return false;
  std::size_t n = std::strlen(s);
  return n<=_s.size() && sameChars(_s.data()+_s.size()-n,s,n,ignoreCase);
  }

  int String::index(const char * s, int start, bool ignoreCase) const
  {
  if (!s) return kNPOS;
  if (start<0) start = 0;
  std::size_t n = std::strlen(s);
  if (std::size_t(start)+n>_s.size()) return kNPOS;
  if (!ignoreCase)
    {
    std::size_t p = _s.find(s,std::size_t(start));
    return p==std::string::npos ? kNPOS : int(p);
    }
  for (std::size_t p=std::size_t(start); p+n<=_s.size(); p++)
    if (sameChars(_s.data()+p,s,n,true)) return int(p);
  return kNPOS;
  }

  int String::first(char c) const
  {
  std::size_t p = _s.find(c);
  return p==std::string::npos ? kNPOS : int(p);
  }

  int String::first(const char * set) const
  {
  if (!set) return kNPOS;
  std::size_t p = _s.find_first_of(set);
  return p==std::string::npos ? kNPOS : int(p);
  }

  int String::last(char c) const
  {
  std::size_t p = _s.rfind(c);
  return p==std::string::npos ? kNPOS : int(p);
  }

  int String::countChar(char c) const
  {
  return int(std::count(_s.begin(),_s.end(),c));
  }

  int String::compareTo(const char * s, bool ignoreCase) const
  {
  if (!s) s = "";
  if (!ignoreCase) return _s.compare(s);
  std::string a(_s), b(s);
  for (auto & c : a) c = lower(c);
  for (auto & c : b) c = lower(c);
  return a.compare(b);
  }

  String & String::insert(int pos, const char * s)
  {
  if (!s) return *this;
  if (pos<0) pos = 0;
  _s.insert(std::min(std::size_t(pos),_s.size()),s);
  return *this;
  }

  String & String::remove(int pos)
  {
  if (pos<0) pos = 0;
  if (std::size_t(pos)<_s.size()) _s.erase(std::size_t(pos));
  return *this;
  }

  String & String::remove(int pos, int n)
  {
  if (pos<0) pos = 0;
  if (n>0 && std::size_t(pos)<_s.size()) _s.erase(std::size_t(pos),std::size_t(n));
  return *this;
  }

  String & String::replace(int pos, int n, const char * s)
  {
  if (pos<0) pos = 0;
  if (std::size_t(pos)>_s.size()) pos = int(_s.size());
  _s.replace(std::size_t(pos),n<0 ? 0 : std::size_t(n),s ? s : "");
  return *this;
  }

  String & String::replaceAll(const char * s1, const char * s2)
  {
  if (!s1 || !*s1) return *this;
  std::string from(s1), to(s2 ? s2 : "");
  std::size_t p = 0;
  while ((p=_s.find(from,p))!=std::string::npos)
    {
    _s.replace(p,from.size(),to);
    p += to.size();
    }
  return *this;
  }

  String & String::toUpper()
  {
  for (auto & c : _s) c = char(std::toupper((unsigned char)c));
  return *this;
  }

  String & String::toLower()
  {
  for (auto & c : _s) c = lower(c);
  return *this;
  }

  String & String::strip()
  {
  std::size_t b = _s.find_first_not_of(" \t\r\n");
  if (b==std::string::npos) { _s.clear(); return *this; }
  std::size_t e = _s.find_last_not_of(" \t\r\n");
  _s = _s.substr(b,e-b+1);
  return *this;
  }

  bool String::isDec() const
  {
  if (_s.empty()) return false;
  for (char c : _s) if (!std::isdigit((unsigned char)c)) return false;
  return true;
  }

  bool String::isDigit() const
  {
  bool digit = false;
  for (char c : _s)
    {
    if (std::isdigit((unsigned char)c)) digit = true;
    else if (c!=' ') return false;
    }
  return digit;
  }

  bool String::isInteger() const
  {
  std::size_t start = (!_s.empty() && (_s[0]=='+' || _s[0]=='-')) ? 1 : 0;
  if (start>=_s.size()) return false;
  for (std::size_t i=start; i<_s.size(); i++) if (!std::isdigit((unsigned char)_s[i])) return false;
  return true;
  }

  // TString::IsFloat(): blank out one '.', one ',', one exponent (not first) and a leading sign, then require
  // digits and spaces only.
  bool String::isFloat() const
  {
  if (isDigit()) return true;
  String tmp(*this);
  tmp.toLower();
  int pos = tmp.first('.');
  if (pos!=kNPOS) tmp.replace(pos,1," ");
  pos = tmp.first(',');
  if (pos!=kNPOS) tmp.replace(pos,1," ");
  pos = tmp.index("e-");
  if (pos>=1) tmp.replace(pos,2," ");
  pos = tmp.index("e+");
  if (pos>=1) tmp.replace(pos,2," ");
  pos = tmp.index("e");
  if (pos>=1) tmp.replace(pos,1," ");
  pos = tmp.first('-');
  if (pos==0) tmp.replace(pos,1," ");
  pos = tmp.first('+');
  if (pos==0) tmp.replace(pos,1," ");
  return tmp.isDigit();
  }

  int String::atoi() const
  {
  return std::atoi(withoutSpaces(_s).c_str());
  }

  long long String::atoll() const
  {
  return std::atoll(withoutSpaces(_s).c_str());
  }

  double String::atof() const
  {
  std::string t(_s);
  std::size_t comma = t.find(',');
  if (comma!=std::string::npos) t[comma] = '.';
  return std::atof(withoutSpaces(t).c_str());
  }

  String & String::appendFloat(long double v)
  {
  char buffer[64];
  std::snprintf(buffer,sizeof(buffer),"%.17Lg",v);
  _s += buffer;
  return *this;
  }

  std::ostream & operator<<(std::ostream & os, const String & s)
  {
  return os << s.str();
  }

  std::istream & operator>>(std::istream & is, String & s)
  {
  return is >> s.str();
  }

} // namespace CAP
