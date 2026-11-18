/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TablePlots.cpp
//! @brief Implementation of TablePlots.
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include "TablePlots.hpp"
#include "DataPointsVector.hpp"
#include "Histogram.hpp"
#include "MathExceptions.hpp"

namespace Datum
{

namespace
{
void check(const MATH::Table2D & table, int column, const char * source)
{
  if (column>=0 && unsigned(column)>=table.nColumns())
    throw MATH::MathException("Column index out of range",source);
}
} // namespace

MATH::DataPointsVector * TablePlots::xy(const MATH::Table2D & table, int xColumn, unsigned int yColumn,
                                        int yErrorColumn, int xErrorColumn)
{
  check(table,xColumn,"TablePlots::xy()");
  check(table,int(yColumn),"TablePlots::xy()");
  check(table,yErrorColumn,"TablePlots::xy()");
  check(table,xErrorColumn,"TablePlots::xy()");
  std::vector<double> x, xe, y, ye, ys;
  for (unsigned int i=0; i<table.nRows(); i++)
    {
    double vx = xColumn>=0 ? table(i,unsigned(xColumn)) : double(i), vy = table(i,yColumn);
    if (!std::isfinite(vx) || !std::isfinite(vy)) continue;
    double ex = xErrorColumn>=0 ? table(i,unsigned(xErrorColumn)) : 0.0;
    double ey = yErrorColumn>=0 ? table(i,unsigned(yErrorColumn)) : 0.0;
    x.push_back(vx);
    y.push_back(vy);
    xe.push_back(std::isfinite(ex) ? std::fabs(ex) : 0.0);
    ye.push_back(std::isfinite(ey) ? std::fabs(ey) : 0.0);
    ys.push_back(0.0);
    }
  auto * points = new MATH::DataPointsVector(x,xe,y,ye,ys);
  String xName = xColumn>=0 ? table.columnName(unsigned(xColumn)) : String("row"), yName = table.columnName(yColumn);
  points->setName(yName+String("_vs_")+xName);
  points->setTitle(yName+String(" vs ")+xName);
  return points;
}

MATH::DataPointsVector * TablePlots::bars(const MATH::Table2D & table, unsigned int valueColumn, int xColumn, double width,
                                          double offset)
{
  check(table,int(valueColumn),"TablePlots::bars()");
  check(table,xColumn,"TablePlots::bars()");
  if (!(width>0)) width = 0.8;
  if (width>1) width = 1;
  std::vector<double> x, y;
  for (unsigned int i=0; i<table.nRows(); i++)
    {
    double vx = xColumn>=0 ? table(i,unsigned(xColumn)) : double(i);
    double vy = table(i,valueColumn);
    if (!std::isfinite(vx) || !std::isfinite(vy)) continue;
    x.push_back(vx);
    y.push_back(vy);
    }
  // smallest distance between two positions sets the width of the bars
  std::vector<double> sorted(x);
  std::sort(sorted.begin(),sorted.end());
  double spacing = std::numeric_limits<double>::infinity();
  for (std::size_t k=1; k<sorted.size(); k++)
    if (sorted[k]>sorted[k-1]) spacing = std::min(spacing,sorted[k]-sorted[k-1]);
  if (!std::isfinite(spacing)) spacing = 1.0;
  if (offset!=0) for (double & v : x) v += offset*spacing;
  std::vector<double> xe(x.size(),0.5*width*spacing), zero(x.size(),0.0);
  auto * points = new MATH::DataPointsVector(x,xe,y,zero,zero);
  String name = table.columnName(valueColumn);
  points->setName(name+String("_bars"));
  points->setTitle(xColumn>=0 ? name+String(" vs ")+table.columnName(unsigned(xColumn)) : name);
  return points;
}

MATH::Histogram * TablePlots::distribution(const MATH::Table2D & table, unsigned int column, unsigned int nBins,
                                           double minimum, double maximum)
{
  check(table,int(column),"TablePlots::distribution()");
  std::vector<double> values;
  for (unsigned int i=0; i<table.nRows(); i++) if (std::isfinite(table(i,column))) values.push_back(table(i,column));
  if (values.empty()) throw MATH::MathException("The column holds no finite value","TablePlots::distribution()");
  if (!(maximum>minimum))
    {
    minimum = *std::min_element(values.begin(),values.end());
    maximum = *std::max_element(values.begin(),values.end());
    if (!(maximum>minimum)) { minimum -= 0.5; maximum += 0.5; }
    else
      {
      double margin = 1e-9*(maximum-minimum);   // the largest value falls in the last bin
      maximum += margin;
      }
    }
  if (nBins==0) nBins = std::max(5u,unsigned(std::ceil(std::log2(double(values.size()))+1)));   // Sturges
  String name = table.columnName(column);
  auto * histogram = new MATH::Histogram(name+String("_distribution"),name,nBins,minimum,maximum);
  histogram->setDimensionTitle(0,name);
  for (double v : values) histogram->fill(v,1.0);
  return histogram;
}

MATH::Histogram * TablePlots::matrixMap(const MATH::Table2D & matrix, const String & name)
{
  unsigned int n = matrix.nRows();
  if (n==0 || matrix.nColumns()!=n) throw MATH::MathException("The matrix must be square and not empty","TablePlots::matrixMap()");
  String histogramName = name.isNull() ? matrix.name() : name;
  auto * map = new MATH::Histogram(histogramName,histogramName,n,0.0,double(n),n,0.0,double(n));
  for (unsigned int i=0; i<n; i++)
    for (unsigned int j=0; j<n; j++)
      {
      double v = matrix(i,j);
      map->setBinValueAt(i,j,std::isfinite(v) ? v : 0.0);
      map->setBinUncertaintyAt(i,j,0.0);
      }
  map->setDimensionTitle(0,"variable");
  map->setDimensionTitle(1,"variable");
  return map;
}

} // namespace Datum
