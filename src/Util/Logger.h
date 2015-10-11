// Logger.h

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <fstream>

#ifdef _DEBUG
#define LOGGING_ENABLED
#endif

#ifdef LOGGING_ENABLED

#  ifdef _WIN32
#  define LOG_INFO(string, ...) Logger::Instance().Write(string , __VA_ARGS__)
#  define LOG_ERROR(string, ...) Logger::Instance().Write(string , __VA_ARGS__)
#  endif

#  ifdef _UNIX
#  define LOG_INFO(string, args...) Logger::Instance().Write(string, ## args)
#  define LOG_ERROR(string, args...) Logger::Instance().Write(string, ## args)
#  endif

#else
#define LOG_INFO(string, ...)
#define LOG_ERROR(string, ...)
#endif

/// Writes log messages to output stream.
class Logger
{
public:
    void Write(const char*, ...);

    static Logger& Instance()
    {
        static Logger theLogger;   // Instantiated when this function is called
        return theLogger;
    }

private:
    Logger();                           ///< disallow default constructor
    Logger(const Logger&);              ///< disallow copy constructor
    Logger& operator = (const Logger&); ///< disallow assignment operator
    virtual ~Logger();

    std::ofstream  m_stream;
};

#endif //_LOGGER_H_
