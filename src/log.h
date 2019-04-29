#pragma once

#include <sstream>
#include <iostream>
#include <string.h>

enum LogSeverity {
	LOG_SEVERITY_TRACE,
	LOG_SEVERITY_DEBUG,
	LOG_SEVERITY_INFO,
	LOG_SEVERITY_WARN,
	LOG_SEVERITY_ERROR
};

class Logger {
    std::stringstream m_ss;

public:
    Logger(int level, const char* file, int line, const char* func) {
        switch (level) {
        case LOG_SEVERITY_TRACE:
            m_ss << "TRAC";
            break;
        case LOG_SEVERITY_DEBUG:
            m_ss << "DEBG";
            break;
        case LOG_SEVERITY_INFO:
            m_ss << "INFO";
            break;
        case LOG_SEVERITY_WARN:
            m_ss << "WARN";
            break;
        case LOG_SEVERITY_ERROR:
            m_ss << "ERRO";
            break;
        }
        m_ss << "|";

        const char* slashPos = strrchr(file, '/');
        m_ss << (slashPos ? slashPos + 1 : file);
        m_ss << ":" << line << "|" << func << "|";
    }

    ~Logger() {
        std::cout << m_ss.str() << std::endl;
    }

    std::stringstream& stream() {
        return m_ss;
    }
};

#define LOG(LEVEL)                                                            \
    Logger(LEVEL, __FILE__, __LINE__, __FUNCTION__).stream()

#define LOGT LOG(LOG_SEVERITY_TRACE)
#define LOGD LOG(LOG_SEVERITY_DEBUG)
#define LOGI LOG(LOG_SEVERITY_INFO)
#define LOGW LOG(LOG_SEVERITY_WARN)
#define LOGE LOG(LOG_SEVERITY_ERROR)