#pragma once
namespace Config::Settings {struct LogLevel {enum Value {INFO,DEBUG,TRACE};};}
namespace Compat {
struct Log {inline static Config::Settings::LogLevel::Value level=Config::Settings::LogLevel::INFO;
 static auto getLogLevel(){return level;}};
struct NullStream {template<class T> NullStream& operator<<(const T&){return *this;}};
}
#define LOG_DEBUG Compat::NullStream()
#define LOG_FUNC(...) ((void)0)
#define LOG_RESULT(value) (value)
