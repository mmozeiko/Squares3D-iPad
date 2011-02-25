#ifndef __FILE_H__
#define __FILE_H__

#include <cstdio>

#include "common.h"

namespace File
{
    bool exists(const string& filename);

    class Reader : NoCopy
    {
    public:
        Reader(const string& filename, bool inWriteFolder = false);
        ~Reader();
        
        bool is_open() const;

        size_t size() const;
        
        unsigned char* pointer() const;

    private:
        unsigned char* m_pointer;
        size_t m_size;
    };

    class Writer : NoCopy
    {
    public:
        Writer(const string& filename, bool append = false);
        ~Writer();
        
        bool is_open() const;
        size_t write(const void* buffer, size_t size);

    private:
        FILE* m_handle;
    };

};

#endif
