/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "SelectionGenerator.hpp"
#include "Exceptions.hpp"


Datum::SelectionGenerator::SelectionGenerator()
:
cumulativeProbability()
{    }

Datum::SelectionGenerator::SelectionGenerator(const std::vector<double> & probabilities)
:
cumulativeProbability()
{
  initializeWith(probabilities);
}

Datum::SelectionGenerator & Datum::SelectionGenerator::operator=(const Datum::SelectionGenerator & source)
{
  if (this!=&source)
    {
    cumulativeProbability = source.cumulativeProbability;
    }
  return *this;
}

void Datum::SelectionGenerator::initializeWith(const std::vector<double> & probabilities)
{
  cumulativeProbability.clear();
  int n = probabilities.size();
  if (n==0) throw Datum::Exception("Empty probability vector","SelectionGenerator::initializeWith()");
  double sum = 0.0;
  for (int k=0; k<n; k++)
    {
    if (probabilities[k]<0.0) throw Datum::Exception("Negative probability","SelectionGenerator::initializeWith()");
    sum += probabilities[k];
    }
  if (!(sum>0.0)) throw Datum::Exception("Probabilities sum to zero","SelectionGenerator::initializeWith()");
  double prob = 0.0;
  for (int kk=0; kk<n; kk++)
    {
    prob += probabilities[kk]/sum;
    cumulativeProbability.push_back(prob);
    }
}


int Datum::SelectionGenerator::generate()
{
  double v = Datum::MATH::randomGenerator().random();
  int n = cumulativeProbability.size();
  if (n==0) throw Datum::Exception("Generator not initialized","SelectionGenerator::generate()");
  for (int k=0; k<n; k++)
  {
  if (v <= cumulativeProbability[k]) return k;
  }
  return n-1;
}

