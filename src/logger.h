#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>

// ANSI escape codes for colored text
#define ANSI_COLOR_RED "\x1b[91m"
#define ANSI_COLOR_YELLOW "\x1b[93m"
#define ANSI_COLOR_BLUE "\x1b[94m"
#define ANSI_COLOR_GREEN "\x1b[92m"
#define ANSI_COLOR_RESET "\x1b[0m"

// Log levels
enum LogLevel
{
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN
};

class Logger
{
public:
        static void setLogLevel(LogLevel level)
        {
                get().logLevel = level;
        }

        // Log with printf-style formatting
        template <typename... Args>
        static void log(LogLevel level, const char *format, Args... args)
        {
                if (level >= get().logLevel)
                {
                        std::time_t now = std::time(0);
                        std::tm *timeinfo = std::localtime(&now);
                        std::cout << "[" << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S") << "] ";

                        switch (level)
                        {
                        case LOG_DEBUG:
                                std::cout << ANSI_COLOR_GREEN << "DEBUG: ";
                                break;
                        case LOG_INFO:
                                std::cout << ANSI_COLOR_BLUE << "INFO: ";
                                break;
                        case LOG_WARN:
                                std::cout << ANSI_COLOR_RED << "WARN: ";
                                break;
                        }

                        std::cout << ANSI_COLOR_RESET;
                        printf(format, args...);
                }
        }

        // Convenience functions for different log levels
        template <typename... Args>
        static void debug(const char *format, Args... args)
        {
                log(LOG_DEBUG, format, args...);
        }

        template <typename... Args>
        static void info(const char *format, Args... args)
        {
                log(LOG_INFO, format, args...);
        }

        template <typename... Args>
        static void warn(const char *format, Args... args)
        {
                log(LOG_WARN, format, args...);
        }

private:
        Logger() : logLevel(LOG_INFO) {} // Private constructor to prevent instantiation

        static Logger &get()
        {
                static Logger instance;
                return instance;
        }

        LogLevel logLevel;
};
