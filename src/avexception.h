#ifndef AVEXCEPTION_H
#define AVEXCEPTION_H

#include <exception>
#include <string>
#include <string_view>

class AVException : public std::exception {
    std::string error;

public:
    explicit AVException(std::string_view s) noexcept
        : error(s) {
    }
    virtual ~AVException() noexcept override {
    }
    virtual const char* what() const noexcept override {
        return error.c_str();
    }
};

#endif // AVEXCEPTION_H
