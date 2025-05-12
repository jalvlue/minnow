#include "reassembler.hh"
#include "debug.hh"

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring )
{
  // found last index at the stream, record it for further checking.
  if ( is_last_substring ) {
    this->final_end_index = first_index + data.size();
  }

  auto buffer_bound = this->buffer_bound();
  this->trim_data( buffer_bound.first, buffer_bound.second, first_index, data );

  if ( !data.empty() ) {
    // if data is overlapping, buffer the longer one
    auto [it, ok] = this->buffer_.emplace( first_index, std::move( data ) );
    if ( not ok and it->second.size() < data.size() ) {
      it->second = std::move( data );
    }

    while ( not this->buffer_.empty() ) {
      auto top = this->buffer_.begin();
      buffer_bound = this->buffer_bound();

      // no available capacity for writer
      if ( buffer_bound.first == buffer_bound.second ) {
        break;
      }

      // hit buffer_begin, try to push to writer
      if ( top->first <= buffer_bound.first ) {
        debug( "first_index: {}, orig_str: {}", top->first, top->second );
        auto curr_index = top->first;
        auto curr_str = std::move( top->second );
        this->buffer_.erase( top );
        if ( curr_index + curr_str.size() <= buffer_bound.first ) {
          // out-of-dated
          continue;
        }

        this->trim_data( buffer_bound.first, buffer_bound.second, curr_index, curr_str );
        debug( "write string: {}", curr_str );
        this->output_.writer().push( std::move( curr_str ) );
      } else {
        break;
      }
    }
  }

  // check if finish writing(recorded final_end_index and no more bytes to write)
  buffer_bound = this->buffer_bound();
  if ( this->final_end_index != UINT64_MAX and this->final_end_index == buffer_bound.first ) {
    this->output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t res { 0 };
  uint64_t prev_end { 0 };
  auto buffer_bound = this->buffer_bound();
  for ( const auto& node : this->buffer_ ) {
    auto begin = std::max( node.first, std::max( prev_end, buffer_bound.first ) );
    auto end = std::min( buffer_bound.second, node.first + node.second.size() );

    if ( end <= begin ) {
      continue;
    }

    res += end - begin;
    prev_end = end;
  }

  return std::min( res, buffer_bound.second - buffer_bound.first );
}

std::pair<uint64_t, uint64_t> Reassembler::buffer_bound() const
{
  auto begin = this->writer().bytes_pushed();
  auto end = this->writer().available_capacity() + begin;
  return { begin, end };
}

// trim_data trims data to fit into buffer
// remove leading bytes that has been buffered
// remove trailing bytes that out-of-bound of buffer capacity
void Reassembler::trim_data( uint64_t buffer_begin,
                             uint64_t buffer_end,
                             uint64_t& first_index,
                             std::string& data ) const
{
  auto end_index = first_index + data.size();

  // within buffer, ok
  if ( first_index >= buffer_begin and end_index <= buffer_end ) {
    return;
  }

  // out-of buffer, trim to empty
  if ( first_index >= buffer_end or end_index <= buffer_begin ) {
    data = {};
    return;
  }

  // do trim
  auto trim_begin = std::max( first_index, buffer_begin );
  data = data.substr( trim_begin - first_index, std::min( buffer_end - buffer_begin, data.size() ) );
  first_index = trim_begin;
  return;
}
