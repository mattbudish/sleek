#pragma once
#include <istream>
#include <ostream>
#include <streambuf>

namespace sleek
{

namespace util
{

class fdstreambuf : public std::streambuf
{
private:
    int fd;
    char buf;
    
public:
    fdstreambuf(int fd);

protected:
    // Write methods

    virtual int_type overflow(int_type c) override;
    virtual std::streamsize xsputn(const char* s, std::streamsize num) override;

    // Read methods

    virtual int underflow() override;
    virtual std::streamsize xsgetn(char_type* s, std::streamsize count) override;
};

class ofdstream : public std::ostream
{
private:
    fdstreambuf buf;

public:
    ofdstream(int fd);
};

class ifdstream : public std::istream
{
private:
    fdstreambuf buf;

public:
    ifdstream(int fd);
};

}

}