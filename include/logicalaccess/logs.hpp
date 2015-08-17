/**
 * \file logs.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Logging header
 */

#ifndef LOGICALACCESS_LOGS_HPP
#define LOGICALACCESS_LOGS_HPP

/**
 * \brief Convenient alias to create an exception.
 */
#undef EXCEPTION
#define EXCEPTION(type, ...)                                                   \
    type(/*__FUNCTION__ "," __FILE__ "," __LINE__ " : "*/ __VA_ARGS__)

/**
 * \brief Assertion which raises an exception on failure.
 */
#define EXCEPTION_ASSERT(condition, type, msg)                                 \
    if (!(condition))                                                          \
    {                                                                          \
        throw EXCEPTION(type, msg);                                            \
    }

#include "logicalaccess_api.hpp"
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

namespace logicalaccess
{
    enum LogLevel
    {
        NONE = 0,
        INFOS,
        WARNINGS,
        NOTICES,
        ERRORS,
        EMERGENSYS,
        CRITICALS,
        ALERTS,
        DEBUGS,
        COMS,
        PLUGINS,
        PLUGINS_ERROR
    };

    /**
     * A class that push a string into the current logger's context at
     * construction, and pop it at deletion.
     *
     * The direct use of this class is discouraged and the macro
     * LLA_LOG_CTX(...) should be used to push some context to the logger.
     */
    class LIBLOGICALACCESS_API LogContext
    {
      public:
        LogContext(const std::string &);
        ~LogContext();
    };

    class LIBLOGICALACCESS_API Logs
    {
      public:
        Logs(const char *file, const char *func, int line, enum LogLevel level);

        ~Logs();

        template <class T>
        Logs &operator<<(const T &arg)
        {
            _stream << arg;
            return (*this);
        }

        static std::ofstream logfile;

        /**
        * Do we duplicate the log to stderr?
        * Defaults to false.
        */
        static bool logToStderr;

      private:
        /**
         * Build a string containing some contextual information.
         */
        std::string pretty_context_infos();

        enum LogLevel d_level;
        std::stringstream _stream;
        static std::map<LogLevel, std::string> logLevelMsg;

        /**
         * A queue of "context string" to help make sense of the log message.
         */
        static thread_local std::vector<std::string> context_;

        friend class LogContext;
    };

    /**
     * A RAII object that disable logging in its constructor, and restore
     * the old value in its destructor.
     *
     * This is used where we need to temporarily disable logging. This is
     * exception safe.
     */
    struct LIBLOGICALACCESS_API LogDisabler
    {
        LogDisabler();
        ~LogDisabler();

      private:
        bool old_;
    };

    /**
     * An overload to pretty-print a byte vector to an ostream.
     */
    LIBLOGICALACCESS_API std::ostream &
    operator<<(std::ostream &ss, const std::vector<unsigned char> &bytebuff);

    /**
     * An overload to pretty-print a boolean vector to an ostream.
     */
    LIBLOGICALACCESS_API std::ostream &
    operator<<(std::ostream &ss, const std::vector<bool> &bytebuff);

#ifdef LOGICALACCESS_LOGS

#define LOG(x) logicalaccess::Logs(__FILE__, __FUNCTION__, __LINE__, x)

#define LLA_LOG_CTX(param)                                                     \
    LogContext lla_log_ctx([&](void)                                           \
                           {                                                   \
                               std::stringstream logger_macro_ss__;            \
                               logger_macro_ss__ << param;                     \
                               return logger_macro_ss__.str();                 \
                           }())

/**
* Convenient alias to throw an exception with logs.
*/
#define THROW_EXCEPTION_WITH_LOG(type, msg, ...)                               \
    {                                                                          \
        LOG(logicalaccess::LogLevel::ERRORS) << msg;                           \
        throw EXCEPTION(type, msg, ##__VA_ARGS__);                             \
    }

/**
* Assertion which raises an exception on failure with logs.
*/
#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg, ...)                   \
    if (!(condition))                                                          \
    {                                                                          \
        THROW_EXCEPTION_WITH_LOG(type, msg, ##__VA_ARGS__);                    \
    }

#else

#define LOG(x)                                                                 \
    logicalaccess::Logs(__FILE__, __FUNCTION__, __LINE__,                      \
                        logicalaccess::LogLevel::NONE)

#define THROW_EXCEPTION_WITH_LOG(type, msg)                                    \
    {                                                                          \
        throw EXCEPTION(type, msg);                                            \
    }
#define EXCEPTION_ASSERT_WITH_LOG(condition, type, msg)                        \
    if (!(condition))                                                          \
    {                                                                          \
        THROW_EXCEPTION_WITH_LOG(type, msg);                                   \
    }

#endif
}

#endif
