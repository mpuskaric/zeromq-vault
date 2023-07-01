// hexDump source https://gist.github.com/drbobbeaty/722469

#ifndef UTILS_H
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <msgpack.hpp>

struct Content {
	char header[9]; 
	char body[10];
	std::vector<int> vec; 
	MSGPACK_DEFINE(header, body, vec);
};

std::string hexDump( const zmq::message_t & aMessage ) {
  // I'm going to build a hex/ascii dump like you've seen so many times before
  std::string  msg;
  std::string  ascii;
  // get the pointer to the start of the message's payload - and it's size
  const char *buff = (const char *)aMessage.data();
  int         size = ((zmq::message_t &)aMessage).size();
  const char *end  = buff + size - 1;
  // see if it's the trivial case
  if (buff == NULL) {
    msg.append("NULL");
  } else {
    // get a place to hold the conversion of each byte
    char   hex[3];
    bzero(hex, 3);
    // run through the valid data in the buffer
    for (const char *p = buff; p <= end; ++p) {
      // generate the hex code for the byte and add it
      snprintf(hex, 3, "%02x", (uint8_t)(*p));
      msg.append(hex).append(" ");
      // if it's printable, add it to the ascii part, otherwise, put a '.'
      if (isprint(*p)) {
        ascii.append(p, 1);
      } else {
        ascii.append(".");
      }
      // see if we have a complete line
      if (ascii.size() >= 19) {
        msg.append(" ").append(ascii).append("\n");
        ascii.clear();
      }
    }
    // if we have anything left, put it on the line as well
    if (ascii.size() > 0) {
      msg.append((19 - ascii.length())*3 + 1, ' ').append(ascii);
    }
  }

  return msg;
}


#endif