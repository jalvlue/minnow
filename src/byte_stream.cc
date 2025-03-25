#include "byte_stream.hh"
#include <vector>

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : buf( capacity )
  , capacity_( capacity )
  , size_( 0 )
  , head_( 0 )
  , tail_( 0 )
  , bytes_pushed_( 0 )
  , bytes_popped_( 0 )
  , error_( false )
  , close_( false ) {};

void Writer::push( string data )
{
  // return directly after close
  if ( this->close_ ) {
    return;
  }

  auto num_pushed = min( this->available_capacity(), data.size() );
  for ( uint64_t i = 0; i < num_pushed; ++i ) {
    this->buf[this->tail_] = data[i];
    this->tail_ = ( this->tail_ + 1 ) % this->capacity_;
  }
  this->size_ += num_pushed;
  this->bytes_pushed_ += num_pushed;
}

void Writer::close()
{
  if ( !this->close_ ) {
    this->close_ = true;
  }
}

bool Writer::is_closed() const
{
  return this->close_;
}

uint64_t Writer::available_capacity() const
{
  return this->capacity_ - this->size_;
}

uint64_t Writer::bytes_pushed() const
{
  return this->bytes_pushed_;
}

string_view Reader::peek() const
{
  if ( this->size_ == 0 ) {
    return "";
  }
  return string_view { &this->buf[this->head_], 1 };
}

void Reader::pop( uint64_t len )
{
  auto num_popped = min( len, this->size_ );
  this->head_ = ( this->head_ + num_popped ) % this->capacity_;
  this->size_ -= num_popped;
  this->bytes_popped_ += num_popped;
}

bool Reader::is_finished() const
{
  return this->close_ && this->size_ == 0;
}

uint64_t Reader::bytes_buffered() const
{
  return this->size_;
}

uint64_t Reader::bytes_popped() const
{
  return this->bytes_popped_;
}
