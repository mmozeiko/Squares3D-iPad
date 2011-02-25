#include <stddef.h>
#include <algorithm>
#include "file.h"
#include "glue.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// PRIVATE part

namespace File
{  

    Reader::Reader(const string& filename, bool inWriteFolder) : m_pointer(NULL)
    {
        int fd = open(((inWriteFolder ? file_get_writePath() : file_get_readPath()) + filename).c_str(), O_RDONLY, 0);
        if (fd >= 0)
        {
            struct stat statInfo;
            if (fstat(fd, &statInfo) == 0)
            {
                m_pointer = (unsigned char*)mmap(NULL, statInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
                if (m_pointer == MAP_FAILED)
                {
                    m_pointer = NULL;
                }
                else
                {
                    m_size = statInfo.st_size;
                }
            }
            
            close(fd);
        }
    }
    
    Reader::~Reader()
    {
        if (m_pointer != NULL)
        {
            munmap(m_pointer, m_size);
        }
    }
               
    bool Reader::is_open() const
    {
        return m_pointer != NULL;
    }
               
    size_t Reader::size() const
    {
        return m_size;
    }
               
    unsigned char* Reader::pointer() const
    {
        return m_pointer;
    }

    Writer::Writer(const string& filename, bool append)
        : m_handle(fopen((file_get_writePath() + filename).c_str(), append ? "ab" : "wb"))
    {
    }
    
    Writer::~Writer()
    {
        if (m_handle != NULL)
        {
            fclose(m_handle);
            m_handle = NULL;
        }
    }
    
    bool Writer::is_open() const
    {
        return m_handle != NULL;
    }

    size_t Writer::write(const void* buffer, size_t size)
    {
        return fwrite(buffer, 1, size, m_handle);
    }

    bool exists(const string& filename)
    {
        FILE* f = fopen((file_get_readPath() + filename).c_str(), "rb");
        if (f == NULL)
        {
            f = fopen((file_get_writePath() + filename).c_str(), "rb");
        }
        if (f == NULL)
        {
            return false;
        }
        fclose(f);

        return true;
    }

}
