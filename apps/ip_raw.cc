#include "socket.hh"
#include <iostream>

using namespace std;

class RawSocket : public DatagramSocket
{
public:
  RawSocket() : DatagramSocket( AF_INET, SOCK_RAW, IPPROTO_RAW ) {}
};

auto zeroes( auto n )
{
  return string( n, 0 );
}

void send_internet_datagram( const string& payload )
{
  RawSocket sock;
  string datagram;
  datagram += char( 0b0100'0101 ); // v4, and headerlength 5words
  datagram += zeroes( 7 );
  datagram += char( 64 );  // TTL
  datagram += char( 5 );   // protocol
  datagram += zeroes( 6 ); // checksum + src address
  datagram += char( 34 );
  datagram += char( 93 );
  datagram += char( 94 );
  datagram += char( 131 );
  datagram += payload;
  sock.sendto( Address { "1" }, datagram );
}

void send_icmp_message( const string& payload )
{
  send_internet_datagram( "\x08" + payload );
}

void program_body()
{
  string payload;
  while ( cin.good() ) {
    getline( cin, payload );
    send_icmp_message( payload + "\n" );
  }
}

int main()
{
  try {
    program_body();
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
