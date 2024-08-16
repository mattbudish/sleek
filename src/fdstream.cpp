// oftstream.cpp

#include <iostream>
#include <streambuf>
#include <cstdio>
#include <unistd.h>

#include "fdstream.hpp"

using namespace std;

using namespace sleek::util;

fdstreambuf::fdstreambuf(int fd) : fd(fd)
{}

streambuf::int_type fdstreambuf::overflow(int_type c)
{
    if (c != EOF) 
    {
        char z = c;
        if (write(fd, &z, 1) != 1) 
        {
            return EOF;
        }
    }

    return c;
}

streamsize fdstreambuf::xsputn(const char* s, streamsize num)
{
    return write(fd,s,num);
}

int fdstreambuf::underflow()
{
    traits_type::int_type i;

    if (read(fd, &i, 1) != 1)
    {
        return traits_type::eof();
    }
    
    buf = traits_type::to_char_type(i);
    setg(&buf, &buf, &buf+1);

    return i;
}

streamsize fdstreambuf::xsgetn(char_type* s, streamsize count)
{
    return read(fd, s, count);
}

ofdstream::ofdstream(int fd) : ostream(0), buf(fd)
{
    rdbuf(&buf);
}

ifdstream::ifdstream(int fd) : istream(0), buf(fd)
{
    rdbuf(&buf);
}