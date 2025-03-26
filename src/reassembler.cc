#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // out of capacity
  // or duplicated datagram (had been written to byte_stream)
  // or duplicated datagram (had been written to reassembler storage)
  auto buf_begin = this->reader().bytes_popped();
  if ( buf_begin + this->output_.capacity() < first_index
       or buf_begin + this->reader().bytes_buffered() > first_index
       or this->first_index_2_data_excerpt_.contains( first_index ) ) {
    return;
  }

  // TODO: directly write to output_
  data.resize( min( data.size(), buf_begin + this->output_.capacity() ) );
  this->first_index_2_data_excerpt_.insert( { first_index, std::move( data ) } );
  if ( is_last_substring ) {
    this->with_last_substring_ = true;
  }

  while ( 1 ) {
    auto bytestream_next_index = this->reader().bytes_popped() + this->reader().bytes_buffered();
    if ( this->first_index_2_data_excerpt_.contains( bytestream_next_index ) ) {
      auto tmp = std::move( first_index_2_data_excerpt_.at( bytestream_next_index ) );
      this->first_index_2_data_excerpt_.erase( bytestream_next_index );
      this->output_.writer().push( std::move( tmp ) );
    } else {
      break;
    }
  }

  // everything is finished
  if ( this->with_last_substring_ and this->first_index_2_data_excerpt_.empty() ) {
    this->output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t bytes_pending {};
  for ( const auto& kv : this->first_index_2_data_excerpt_ ) {
    bytes_pending += kv.second.size();
  }
  return bytes_pending;
}
