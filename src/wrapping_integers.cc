#include "wrapping_integers.hh"
#include "debug.hh"
#include <cstdint>

using namespace std;

// absolute seqno -> seqno
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point + n;
}

// seqno -> absolute seqno
uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  const uint32_t k = this->raw_value_ - zero_point.raw_value_;
  const uint64_t k64 = k, interval = 1ULL << 32;
  const int64_t m = ( static_cast<int64_t>( checkpoint ) - static_cast<int64_t>( k64 ) ) / interval;
  array<int64_t, 3> candidates { m - 1, m, m + 1 };
  uint64_t best_n = 0, min_distance = UINT64_MAX;

  for ( const auto& candidate_m : candidates ) {
    const uint64_t candidate_n = k64 + static_cast<uint64_t>( candidate_m ) * interval;
    const uint64_t distance
      = ( candidate_n > checkpoint ) ? ( candidate_n - checkpoint ) : ( checkpoint - candidate_n );
    if ( distance < min_distance || ( distance == min_distance && candidate_n < best_n ) ) {
      min_distance = distance;
      best_n = candidate_n;
    }
  }
  return best_n;
}
