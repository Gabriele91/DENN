#pragma once 
#include <cstdio>
#include <string>
#include <zlib.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) 
#endif 

namespace Denn
{
namespace IOFileWrapper
{

class std_file
{
    
    FILE* m_file{ nullptr };
    
public:
    
    bool open(const std::string& pathfile,const std::string& mode)
    {
        m_file = std::fopen(pathfile.c_str(),mode.c_str());
        return is_open();
    }
    
    void close()
    {
        if(m_file) std::fclose(m_file);
        m_file = nullptr;
    }
    
    bool is_open() const
    {
        return m_file != nullptr;
    }
    
    size_t write(const void* data,size_t size,size_t count)
    {
        return std::fwrite(data, size, count, m_file);
    }
    
    size_t read(void* data,size_t size,size_t count)
    {
        return std::fread(data, size, count, m_file);
    }
    
    size_t tell() const
    {
        return std::ftell(m_file);
    }
    
    void rewind()
    {
        std::rewind(m_file);
    }
    
    void seek_set(size_t pos = 0)
    {
        std::fseek(m_file, (int)pos, SEEK_SET);
    }
    
    void seek_end(size_t pos = 0)
    {
        std::fseek(m_file, (int)pos, SEEK_END);
    }
    
    void seek_cur(size_t pos = 0)
    {
        std::fseek(m_file, (int)pos, SEEK_CUR);
    }
    
    bool eof() const
    {
        return std::feof(m_file);
    }
    
    size_t size()
    {
        //get pos
        size_t pos = std::ftell(m_file);
        //go to end
        std::fseek(m_file, 0, SEEK_END);
        //get size
        size_t size = std::ftell(m_file);
        //reset pos
        std::fseek(m_file, (int)pos, SEEK_SET);
        //return size
        return size;
    }
};

template < size_t size_buffer = 0x0 >
class zlib_file
{
    
    gzFile       m_file     { nullptr };
	FILE*        m_file_raw { nullptr };
	unsigned int m_file_size{ 0       };
    
public:
    
    bool open(const std::string& pathfile,const std::string& mode)
    {
        const char* pathfile_cptr = pathfile.c_str();
        const char* mode_cptr = mode.c_str();
        //open
		m_file_raw = std::fopen(pathfile_cptr, mode_cptr);
		//if is open
		if (m_file_raw)
		{
			//compute size
			compute_size();
			//gz file
            #ifdef __APPLE__
            m_file = gzopen(pathfile_cptr, mode_cptr); 
            #else 
			m_file = gzdopen(::fileno(m_file_raw), mode_cptr); 
            #endif
			//test
			if (!m_file)
			{
				close();
				return false;
			}
			//set buffer size
			if (is_open() && size_buffer)
			{
				gzbuffer(m_file, size_buffer);
			}
			//return
			return is_open();
		}
        return false;
    }
    
    void close()
    {
        if (m_file)     gzclose(m_file);
		if (m_file_raw) fclose(m_file_raw);
		m_file      = nullptr;
		m_file_raw  = nullptr;
		m_file_size = 0;
    }
    
    bool is_open() const
    {
        return m_file != nullptr;
    }
    
    size_t write(const void* data,size_t size,size_t count)
    {
        
        return gzwrite(m_file, data,(int)(size * count));
    }
    
    size_t read(void* data,size_t size,size_t count)
    {
        return gzread(m_file, data, (int)(size * count));
    }
    
    size_t tell() const
    {
        return gztell(m_file);
    }
    
    void rewind()
    {
        gzrewind(m_file);
    }
    
    void seek_set(size_t pos = 0)
    {
        gzseek(m_file, (int)pos, SEEK_SET);
    }
    
    void seek_end(size_t pos = 0)
    {
        denn_assert(0);
        gzseek(m_file, (int)pos, SEEK_END);
    }
    
    void seek_cur(size_t pos = 0)
    {
        gzseek(m_file, (int)pos, SEEK_CUR);
    }
    
    bool eof() const
    {
        return gzeof(m_file);
    }
    
    size_t size() const
    {
        return m_file_size;
    }

protected:

	void compute_size()
	{
		m_file_size = 0;
		//test
		if (!m_file_raw) return;
        //return pos
        auto pos = std::ftell(m_file_raw);
		//move
		std::fseek(m_file_raw, -4, SEEK_END);
		//read
		unsigned char buf_size[]{ 0,0,0,0 };
		if (std::fread((unsigned char*)buf_size, sizeof(unsigned char), 4, m_file_raw) == 4)
		{
			//return
			m_file_size = (unsigned int)((buf_size[3] << 24) | (buf_size[2] << 16) | (buf_size[1] << 8) | buf_size[0]);
		}
		//move
		std::fseek(m_file_raw, pos, SEEK_SET);
	}

};

#if 0
template < size_t block_size = 9, size_t work_factor = 30 >
class bzip2_file
{
    
    bool    m_mode_w  { false   };
    BZFILE* m_file    { nullptr };
    FILE*   m_raw_file{ nullptr };
    
public:
    
    bool open(const std::string& pathfile,const std::string& mode)
    {
        const char* pathfile_cptr = pathfile.c_str();
        const char* mode_cptr = mode.c_str();
        //write?
        if(*mode_cptr == 'w')
        {
            //open file
            m_raw_file = fopen(pathfile_cptr, "wb");
            if(!m_raw_file) return is_open();
            //save mode
            m_mode_w = true;
            //write mode
            int bz2_error{0};
            m_file = BZ2_bzWriteOpen(&bz2_error,m_raw_file,(int)block_size,0,work_factor);
        }
        //read?
        if(*mode_cptr == 'r')
        {
            //open file
            m_raw_file = fopen(pathfile_cptr, "rb");
            if(!m_raw_file) return is_open();
            //save mode
            m_mode_w = false;
            //write mode
            int bz2_error{0};
            m_file = BZ2_bzReadOpen(&bz2_error,m_raw_file,0,0,0,0);
        }
        //return
        return is_open();
    }
    
    void close()
    {
        //write mode
        int bz2_error{0};
        if(m_mode_w)    BZ2_bzWriteClose(&bz2_error, m_file, 0, NULL, NULL);
        else            BZ2_bzReadClose(&bz2_error, m_file);
        //to null
        m_file = nullptr;
        m_raw_file = nullptr;
    }
    
    bool is_open() const
    {
        return m_file != nullptr;
    }
    
    size_t write(const void* data,size_t size,size_t count)
    {
        int bz2_error{0};
        BZ2_bzWrite(&bz2_error, m_file, (void*)data,(int)(size * count));
        return bz2_error == BZ_OK ? 1 : 0;
    }
    
    size_t read(void* data,size_t size,size_t count)
    {
        int bz2_error{0};
        return (size_t)BZ2_bzRead(&bz2_error,m_file, data,(int)(size * count));
    }
    
    size_t tell() const
    {
        return std::ftell(m_raw_file);
    }
    
    void rewind()
    {
        std::rewind(m_raw_file);
    }
    
    void seek_set(size_t pos = 0)
    {
        std::fseek(m_raw_file, (int)pos, SEEK_SET);
    }
    
    void seek_end(size_t pos = 0)
    {
        std::fseek(m_raw_file, (int)pos, SEEK_END);
    }
    
    void seek_cur(size_t pos = 0)
    {
        std::fseek(m_raw_file, (int)pos, SEEK_CUR);
    }
    
    bool eof() const
    {
        return std::feof(m_raw_file);
    }
    
    size_t size()
    {
        //get pos
        size_t pos = std::ftell(m_raw_file);
        //go to end
        std::fseek(m_raw_file, 0, SEEK_END);
        //get size
        size_t size = std::ftell(m_raw_file);
        //reset pos
        std::fseek(m_raw_file, (int)pos, SEEK_SET);
        //return size
        return size;
    }
};
#endif

}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif