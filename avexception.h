#ifndef AVEXCEPTION_H
#define AVEXCEPTION_H

#include <exception>
#include <string>

class AVException : public std::exception {
std::string error;

public:
    AVException(std::string s) throw() { error = s; }
    virtual ~AVException() throw() {}
    virtual const char* what() const throw() { return error.c_str(); }
};

#endif // AVEXCEPTION_H
