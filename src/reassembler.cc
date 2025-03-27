#include "reassembler.hh"
#include "debug.hh"
#include <cstdint>
#include <iterator>
#include <utility>

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring )
{
  auto next_expected = this->byte_stream_next_index();
  auto end_index = first_index + data.size(); // index of last byte in data

  // duplicated
  // or totally out of capacity
  if ( end_index < next_expected or first_index >= next_expected + this->writer().available_capacity() ) {
    return;
  }

  // partially out of capacity, trim
  if ( first_index + data.size() > next_expected + this->writer().available_capacity() ) {
    data.resize( ( first_index + data.size() ) - ( next_expected + this->writer().available_capacity() ) );
  }

  auto next = this->buffer_.upper_bound( first_index );
  auto prev = ( next == this->buffer_.begin() ) ? this->buffer_.end() : std::prev( next );
  if ( prev != this->buffer_.end() && prev->first + prev->second.size() == next->first ) {
    // duplicated
    return;
  }

  // TODO: maybe directly push if first_index <= next_expected < end_index instead of adding to buffer_
  if ( next != this->buffer_.end() and end_index > next->first ) {
    data.resize( end_index - next->first );
  }
  if ( prev != this->buffer_.end() and prev->first + prev->second.size() > first_index ) {
    prev->second.resize( prev->first + prev->second.size() - first_index );
  }
  debug( "emplace: {}", data );
  this->buffer_.emplace( first_index, std::move( data ) );
  if ( is_last_substring ) {
    this->with_last_substring_ = true;
  }

  auto it = this->buffer_.begin();
  while ( !this->buffer_.empty() and next_expected >= it->first ) {
    auto tmp = std::move( it->second );
    tmp.erase( 0, next_expected - it->first );
    debug( "push: {}", tmp );
    this->output_.writer().push( std::move( tmp ) );
    this->buffer_.erase( it );

    it = this->buffer_.begin();
    next_expected = this->byte_stream_next_index();
  }

  // everything is finished
  if ( this->with_last_substring_ and this->buffer_.empty() ) {
    this->output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  auto next_expected = this->byte_stream_next_index();
  debug( "next_expected: {}", next_expected );
  uint64_t bytes_pending {};
  for ( const auto& kv : this->buffer_ ) {
    debug( "kv.first: {}, kv.second: {}", kv.first, kv.second );
    if ( kv.first < next_expected ) {
      bytes_pending -= next_expected - kv.first;
    }
    bytes_pending += kv.second.size();
  }
  return bytes_pending;
}
