/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableStatistics.cpp
//! @brief Implementation of TableStatistics.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>
#include "TableStatistics.hpp"
#include "ContinuousDistributions.hpp"
#include "MathExceptions.hpp"

namespace Datum
{

namespace
{
const double kNaN = std::numeric_limits<double>::quiet_NaN();

//! The pairs where both values are finite.
void finitePairs(const std::vector<double> & x, const std::vector<double> & y, std::vector<double> & a, std::vector<double> & b)
{
  a.clear(); b.clear();
  std::size_t n = std::min(x.size(),y.size());
  a.reserve(n); b.reserve(n);
  for (std::size_t k=0; k<n; k++)
    if (std::isfinite(x[k]) && std::isfinite(y[k])) { a.push_back(x[k]); b.push_back(y[k]); }
}

//! Pearson correlation of finite pairs (NaN if a series is constant or n < 2).
double pearson(const std::vector<double> & a, const std::vector<double> & b)
{
  std::size_t n = a.size();
  if (n<2) return kNaN;
  double ma = std::accumulate(a.begin(),a.end(),0.0)/n;
  double mb = std::accumulate(b.begin(),b.end(),0.0)/n;
  double sab = 0, saa = 0, sbb = 0;
  for (std::size_t k=0; k<n; k++)
    {
    double da = a[k]-ma, db = b[k]-mb;
    sab += da*db; saa += da*da; sbb += db*db;
    }
  if (!(saa>0) || !(sbb>0)) return kNaN;
  double r = sab/std::sqrt(saa*sbb);
  return std::max(-1.0,std::min(1.0,r));
}

//! Kendall tau-b of finite pairs (O(n^2)).
double kendall(const std::vector<double> & a, const std::vector<double> & b)
{
  std::size_t n = a.size();
  if (n<2) return kNaN;
  double concordant = 0, discordant = 0, tiesA = 0, tiesB = 0;
  for (std::size_t i=0; i<n; i++)
    for (std::size_t j=i+1; j<n; j++)
      {
      double da = a[i]-a[j], db = b[i]-b[j];
      if (da==0 && db==0) continue;
      if (da==0) { tiesA++; continue; }
      if (db==0) { tiesB++; continue; }
      if ((da>0) == (db>0)) concordant++; else discordant++;
      }
  double denominator = std::sqrt((concordant+discordant+tiesA)*(concordant+discordant+tiesB));
  if (!(denominator>0)) return kNaN;
  return (concordant-discordant)/denominator;
}

//! Fill the error and p-value of a correlation r of n pairs.
void significance(TableStatistics::Result & result, TableStatistics::Method method)
{
  double r = result.value;
  double n = result.n;
  result.error = kNaN;
  result.pValue = kNaN;
  if (!std::isfinite(r) || n<3) return;
  result.error = std::sqrt(std::max(0.0,1.0-r*r)/(n-2));
  if (method==TableStatistics::kKendall)
    {
    double z = 3.0*r*std::sqrt(n*(n-1))/std::sqrt(2.0*(2.0*n+5.0));
    result.pValue = std::erfc(std::fabs(z)/std::sqrt(2.0));
    return;
    }
  if (std::fabs(r)>=1.0) { result.pValue = 0.0; return; }
  double t = r*std::sqrt((n-2)/(1-r*r));
  MATH::StudentDistribution student(1.0,n-2);
  result.pValue = std::min(1.0,2.0*student.cdfUpper(std::fabs(t)));
}

std::vector<double> columnOf(const MATH::Table2D & table, unsigned int column)
{
  if (column>=table.nColumns()) throw MATH::MathException("Column index out of range","TableStatistics");
  return table.column(column);
}

std::vector<unsigned int> allColumns(const MATH::Table2D & table, const std::vector<unsigned int> & columns)
{
  if (!columns.empty())
    {
    for (unsigned int c : columns) if (c>=table.nColumns()) throw MATH::MathException("Column index out of range","TableStatistics");
    return columns;
    }
  std::vector<unsigned int> all(table.nColumns());
  std::iota(all.begin(),all.end(),0u);
  return all;
}
} // namespace

String TableStatistics::methodName(Method method)
{
  switch (method)
    {
    case kSpearman: return "Spearman";
    case kKendall:  return "Kendall";
    default:        return "Pearson";
    }
}

TableStatistics::Method TableStatistics::methodFromName(const String & name)
{
  std::string n(name.data());
  for (char & c : n) c = char(std::tolower((unsigned char)c));
  if (n=="spearman") return kSpearman;
  if (n=="kendall")  return kKendall;
  return kPearson;
}

std::vector<double> TableStatistics::ranks(const std::vector<double> & values)
{
  std::vector<double> r(values.size(),kNaN);
  std::vector<std::size_t> order;
  for (std::size_t k=0; k<values.size(); k++) if (!std::isnan(values[k])) order.push_back(k);
  std::sort(order.begin(),order.end(),[&](std::size_t a, std::size_t b) { return values[a]<values[b]; });
  for (std::size_t i=0; i<order.size(); )
    {
    std::size_t j = i;
    while (j+1<order.size() && values[order[j+1]]==values[order[i]]) j++;
    double rank = 0.5*(double(i)+double(j)) + 1.0;   // average of the ranks i+1 ... j+1
    for (std::size_t k=i; k<=j; k++) r[order[k]] = rank;
    i = j+1;
    }
  return r;
}

TableStatistics::Summary TableStatistics::summarize(const MATH::Table2D & table, unsigned int column)
{
  std::vector<double> values = columnOf(table,column);
  Summary s;
  s.name = table.columnName(column);
  std::vector<double> finite;
  finite.reserve(values.size());
  for (double v : values) { if (std::isfinite(v)) finite.push_back(v); else s.missing++; }
  s.n = finite.size();
  if (s.n==0)
    {
    s.mean = s.stdDev = s.meanError = s.minimum = s.median = s.maximum = kNaN;
    return s;
    }
  s.sum  = std::accumulate(finite.begin(),finite.end(),0.0);
  s.mean = s.sum/s.n;
  double ss = 0;
  for (double v : finite) ss += (v-s.mean)*(v-s.mean);
  s.stdDev    = s.n>1 ? std::sqrt(ss/(s.n-1)) : kNaN;
  s.meanError = s.n>1 ? s.stdDev/std::sqrt(double(s.n)) : kNaN;
  std::sort(finite.begin(),finite.end());
  s.minimum = finite.front();
  s.maximum = finite.back();
  s.median  = (s.n%2) ? finite[s.n/2] : 0.5*(finite[s.n/2-1]+finite[s.n/2]);
  return s;
}

MATH::Table2D TableStatistics::summaryTable(const MATH::Table2D & table)
{
  MATH::Table2D result(table.name()+String("_summary"));
  for (unsigned int c=0; c<table.nColumns(); c++)
    {
    Summary s = summarize(table,c);
    result.addRow({ double(s.n), double(s.missing), s.mean, s.stdDev, s.meanError, s.minimum, s.median, s.maximum, s.sum });
    }
  if (result.nColumns()>0)
    result.setColumnNames({ "n", "missing", "mean", "stdDev", "meanError", "minimum", "median", "maximum", "sum" });
  return result;
}

TableStatistics::Result TableStatistics::covariance(const std::vector<double> & x, const std::vector<double> & y)
{
  std::vector<double> a, b;
  finitePairs(x,y,a,b);
  Result result;
  result.n = a.size();
  result.error = kNaN;
  result.pValue = kNaN;
  if (result.n<2) { result.value = kNaN; return result; }
  double ma = std::accumulate(a.begin(),a.end(),0.0)/result.n;
  double mb = std::accumulate(b.begin(),b.end(),0.0)/result.n;
  double s = 0;
  for (std::size_t k=0; k<a.size(); k++) s += (a[k]-ma)*(b[k]-mb);
  result.value = s/(result.n-1);
  return result;
}

TableStatistics::Result TableStatistics::correlation(const std::vector<double> & x, const std::vector<double> & y, Method method)
{
  std::vector<double> a, b;
  finitePairs(x,y,a,b);
  Result result;
  result.n = a.size();
  switch (method)
    {
    case kSpearman: result.value = pearson(ranks(a),ranks(b)); break;
    case kKendall:  result.value = kendall(a,b); break;
    default:        result.value = pearson(a,b);
    }
  significance(result,method);
  return result;
}

TableStatistics::Result TableStatistics::correlation(const MATH::Table2D & table, unsigned int a, unsigned int b, Method method)
{
  return correlation(columnOf(table,a),columnOf(table,b),method);
}

MATH::Table2D TableStatistics::covarianceMatrix(const MATH::Table2D & table, const std::vector<unsigned int> & selected)
{
  std::vector<unsigned int> columns = allColumns(table,selected);
  unsigned int n = columns.size();
  MATH::Table2D result(table.name()+String("_covariance"),n,n,kNaN);
  std::vector< std::vector<double> > data;
  for (unsigned int c : columns) data.push_back(table.column(c));
  for (unsigned int i=0; i<n; i++)
    for (unsigned int j=i; j<n; j++)
      result(i,j) = result(j,i) = covariance(data[i],data[j]).value;
  std::vector<String> names;
  for (unsigned int c : columns) names.push_back(table.columnName(c));
  if (n>0) result.setColumnNames(names);
  return result;
}

MATH::Table2D TableStatistics::correlationMatrix(const MATH::Table2D & table, const std::vector<unsigned int> & selected,
                                                 Method method, MATH::Table2D * pValues)
{
  std::vector<unsigned int> columns = allColumns(table,selected);
  unsigned int n = columns.size();
  MATH::Table2D result(table.name()+String("_")+methodName(method),n,n,kNaN);
  MATH::Table2D p(table.name()+String("_")+methodName(method)+String("_pValues"),n,n,kNaN);
  std::vector< std::vector<double> > data;
  for (unsigned int c : columns) data.push_back(table.column(c));
  for (unsigned int i=0; i<n; i++)
    for (unsigned int j=i; j<n; j++)
      {
      Result r = correlation(data[i],data[j],method);
      if (i==j && std::isfinite(r.value)) { r.value = 1.0; r.pValue = 0.0; }
      result(i,j) = result(j,i) = r.value;
      p(i,j) = p(j,i) = r.pValue;
      }
  std::vector<String> names;
  for (unsigned int c : columns) names.push_back(table.columnName(c));
  if (n>0) { result.setColumnNames(names); p.setColumnNames(names); }
  if (pValues) *pValues = p;
  return result;
}

} // namespace Datum
