/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <atomic>
#include <chrono>
#include <random>
#include "MersenneTwister.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  MersenneTwister::MersenneTwister(std::uint64_t seed)
  :
  RandomGenerator(),
  _mt(),
  _index(kN),
  _seed(0)
  {
  setSeed(seed);
  }

  void MersenneTwister::setSeed(std::uint64_t seed)
  {
  std::uint32_t s = std::uint32_t(seed & 0xffffffffu);
  if (s == 0)
    {
    // a seed of its own: the system's random device, the clock and a counter (two generators seeded at once differ)
    static std::atomic<std::uint32_t> counter{0};
    std::random_device device;
    auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    s = device() ^ std::uint32_t(ticks) ^ std::uint32_t(std::uint64_t(ticks) >> 32) ^ (0x9e3779b9u*++counter);
    if (s == 0) s = 5489;
    }
  _seed = s;
  _mt[0] = s;
  for (int k=1; k<kN; k++) _mt[k] = 1812433253u*(_mt[k-1] ^ (_mt[k-1] >> 30)) + std::uint32_t(k);
  _index = kN;
  resetDistributions();
  }

  void MersenneTwister::setSeeds(const std::vector<std::uint32_t> & keys)
  {
  setSeed(19650218u);
  _seed = 0;
  if (keys.empty()) return;
  int i = 1, j = 0;
  int n = int(keys.size());
  for (int k = (kN > n ? kN : n); k > 0; k--)
    {
    _mt[i] = (_mt[i] ^ ((_mt[i-1] ^ (_mt[i-1] >> 30))*1664525u)) + keys[size_t(j)] + std::uint32_t(j);
    i++; j++;
    if (i >= kN) { _mt[0] = _mt[kN-1]; i = 1; }
    if (j >= n) j = 0;
    }
  for (int k = kN - 1; k > 0; k--)
    {
    _mt[i] = (_mt[i] ^ ((_mt[i-1] ^ (_mt[i-1] >> 30))*1566083941u)) - std::uint32_t(i);
    i++;
    if (i >= kN) { _mt[0] = _mt[kN-1]; i = 1; }
    }
  _mt[0] = 0x80000000u;   // a non-zero initial state
  _index = kN;
  }

  void MersenneTwister::twist()
  {
  static const std::uint32_t mag01[2] = { 0u, 0x9908b0dfu };
  const std::uint32_t upper = 0x80000000u, lower = 0x7fffffffu;
  int k = 0;
  for (; k < kN - kM; k++)
    {
    std::uint32_t y = (_mt[k] & upper) | (_mt[k+1] & lower);
    _mt[k] = _mt[k+kM] ^ (y >> 1) ^ mag01[y & 1u];
    }
  for (; k < kN - 1; k++)
    {
    std::uint32_t y = (_mt[k] & upper) | (_mt[k+1] & lower);
    _mt[k] = _mt[k+(kM-kN)] ^ (y >> 1) ^ mag01[y & 1u];
    }
  std::uint32_t y = (_mt[kN-1] & upper) | (_mt[0] & lower);
  _mt[kN-1] = _mt[kM-1] ^ (y >> 1) ^ mag01[y & 1u];
  _index = 0;
  }

  std::uint32_t MersenneTwister::next32()
  {
  if (_index >= kN) twist();
  std::uint32_t y = _mt[size_t(_index++)];
  // tempering
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680u;
  y ^= (y << 15) & 0xefc60000u;
  y ^= (y >> 18);
  return y;
  }

  double MersenneTwister::random()
  {
  // 27 + 26 bits: a double uniformly distributed in [0,1) (genrand_res53)
  std::uint32_t a = next32() >> 5, b = next32() >> 6;
  return (double(a)*67108864.0 + double(b))*(1.0/9007199254740992.0);
  }

  std::vector<std::uint32_t> MersenneTwister::state() const
  {
  std::vector<std::uint32_t> s(_mt.begin(), _mt.end());
  s.push_back(std::uint32_t(_index));
  return s;
  }

  void MersenneTwister::setState(const std::vector<std::uint32_t> & s)
  {
  if (s.size() != size_t(kN) + 1 || s[kN] > std::uint32_t(kN))
    throw MathException("Not a state of a MersenneTwister", "MersenneTwister::setState");
  for (int k=0; k<kN; k++) _mt[size_t(k)] = s[size_t(k)];
  _index = int(s[kN]);
  resetDistributions();
  }

  } // namespace MATH
} // namespace CAP
