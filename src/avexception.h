#ifndef AVEXCEPTION_H
#define AVEXCEPTION_H

#include <exception>
#include <string>

class AVException : public std::exception {
std::string error;

public:
    AVException(std::string s) noexcept : error(std::move(s)) {}
    virtual ~AVException() noexcept {}
    virtual const char* what() const noexcept override { return error.c_str(); }
};

#endif // AVEXCEPTION_H
