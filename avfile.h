#ifndef AVFILE_H
#define AVFILE_H

#include <string>

class AVFile
{
public:
    AVFile();
    virtual ~AVFile();

    void open(std::string url);

private:

};

#endif // AVFILE_H
