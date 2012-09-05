#ifndef AVEXCEPTION_H
#define AVEXCEPTION_H

#include <exception>

class AVException : public std::exception
{
const char* error;

public:
    AVException(const char* s) throw() { error = s; }
    virtual ~AVException() throw() {}
    virtual const char* what() const throw() { return error; }
};

#endif // AVEXCEPTION_H
