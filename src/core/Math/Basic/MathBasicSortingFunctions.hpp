#ifndef DATUM_MathBasicSortingFunctions
#define DATUM_MathBasicSortingFunctions
#include "MathExceptions.hpp"

#include <vector>
using std::vector;

//!
//! Compendium of sorting  functions
//! By convention, all function names  begin with a lower case letter.
//!

namespace Datum
{
namespace MATH
{
void bubbleHigh(int  Narr, double  *valuesArray, int  *orderArray);
void bubbleLow(int  Narr, double  *valuesArray, int  *orderArray);
void bubbleHigh(vector<double> valuesArray, vector<int> orderArray);
void bubbleLow (vector<double> valuesArray, vector<int> orderArray);
} // namespace MATH

} // namespace Datum

#endif

