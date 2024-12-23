#include "log.h"
#include <iostream>
#include <mutex>
#include <map>
#include <vector>
#include <functional>
namespace thefool{



    //LogEvent
    LogEvent::LogEvent(
                const std::shared_ptr<Logger>& logger,
                const std::string& file, 
                int32_t line, 
                LogLevel::Level level,
                std::thread::id threadId,
                const std::string& threadName,
                uint32_t fibreId,
                uint64_t time
            ): 
            logger_(logger), 
            threadId_(threadId), 
            threadName_(threadName), 
            fibreId_(fibreId), 
            time_(time), 
            file_(file), 
            line_(line), 
            level_(level)
        {

        }
     std::string LogLevel::level_to_string(Level level){
            switch (level)
            {
            case Level::debug:
                return "debug";
            case Level::notice:
                return "notice";
            case Level::warning:    
                return "warning";
            case Level::error:
                return "error";
            case Level::verbose:
                return "verbose";
            default:
                return "debug";
            }
    }
    LogLevel::Level LogLevel::string_to_level(const std::string& level_str){
            return LogLevel::Level::debug;
    }
    // LogEvent::~LogEvent() {
    //     std::cout<<"LogEvent::~LogEvent()"<<std::endl;
    // }


    //LogFormatter

    class LevelFormatter : public LogFormatter{

    };

    class ThreadIdFormatter : public LogFormatter{
        
    };


    
class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->get_message();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,  LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::level_to_string(level);
    }
};

// class ElapseFormatItem : public LogFormatter::FormatItem {
// public:
//     ElapseFormatItem(const std::string& str = "") {}
//     void format(std::ostream& os,LogLevel::Level level, LogEvent::ptr event) override {
//         os << event->getElapse();
//     }
// };

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,  LogLevel::Level level, LogEvent::ptr event) override {
        os << event->get_logger()->get_name();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->get_thread_id();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,  LogLevel::Level level, LogEvent::ptr event) override {
        os << event->get_fibre_id();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,  LogLevel::Level level, LogEvent::ptr event) override {
        os << event->get_thread_name();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os,LogLevel::Level level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->get_time();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->get_file();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->get_line();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class SpaceFormatItem : public LogFormatter::FormatItem {
    public:
    SpaceFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << " ";
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,  LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
private:
    std::string m_string;
};

    LogFormatter::LogFormatter(const std::string& format_str) : pattern_(format_str){
        init();
    }
 
    
//%xxx %xxx{xxx} %%
void LogFormatter::init() {
    //str, format, type
    std::vector<std::tuple<std::string, std::string, int> > vec;
    std::string nstr;
    for(size_t i = 0; i < pattern_.size(); ++i) {
        if(pattern_[i] != '%') {
            nstr.append(1, pattern_[i]);
            continue;
        }

        if((i + 1) < pattern_.size()) {
            if(pattern_[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while(n < pattern_.size()) {
            if(!fmt_status && (!isalpha(pattern_[n]) && pattern_[n] != '{'
                    && pattern_[n] != '}')) {
                str = pattern_.substr(i + 1, n - i - 1);
                break;
            }
            if(fmt_status == 0) {
                if(pattern_[n] == '{') {
                    str = pattern_.substr(i + 1, n - i - 1);
                    //std::cout << "*" << str << std::endl;
                    fmt_status = 1; //解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if(fmt_status == 1) {
                if(pattern_[n] == '}') {
                    fmt = pattern_.substr(fmt_begin + 1, n - fmt_begin - 1);
                    //std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == pattern_.size()) {
                if(str.empty()) {
                    str = pattern_.substr(i + 1);
                }
            }
        }

        if(fmt_status == 0) {
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if(fmt_status == 1) {
            std::cout << "pattern parse error: " << pattern_ << " - " << pattern_.substr(i) << std::endl;
            error_ = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}

        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
       // XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FilenameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };

    for(auto& i : vec) {
        if(std::get<2>(i) == 0) {
            items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            //std::cout << "items push_back string1   "<<items_.size() << std::endl;

        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()) {
                items_.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                //  std::cout << "items push_back string2   "<<items_.size() << std::endl;
                error_ = true;
            } else {
                items_.push_back(it->second(std::get<1>(i)));
                //  std::cout << "items push_back string3   "<<items_.size() << std::endl;
            }
        }
        //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
   // std::cout <<" LogFormatter::init "<< items_.size() << std::endl;
}
    std::string LogFormatter::format( LogLevel::Level level, LogEvent::ptr event){
        std::ostringstream oss;
        for(auto& item : items_ ){
            item->format(oss, level, event);
        }
        return oss.str();
    }
    std::ostream& LogFormatter::format(std::ostream& ofs, LogLevel::Level level, LogEvent::ptr event){
        // std::cout << "LogFormatter::format size="<< std::endl;
        // if(items_.empty()){
        //     std::cout << "LogFormatter::format empty" << std::endl;
        // }
        // else{
        //     std::cout << "LogFormatter::format size="<< std::endl;
        // }
        for(auto& item : items_ ){
            item->format(ofs, level, event);
        }
        return ofs;
    }






    //Logger


    Logger::Logger(const std::string& name )
    :name_(name), 
    level_(LogLevel::debug){
        formatter_.reset(new LogFormatter("%c:%d{%Y-%m-%d %H:%M:%S} %f:%l%T %p:%m%n"));

    }

    void Logger::add_appender(const LogAppender::ptr& appender){
        std::lock_guard<std::mutex> lock(mutex_);
        if(!appender->getFormatter()){
            //std::lock_guard<std::mutex> lock(appender->mutex_);
            appender->setFormatter(formatter_);
        }
        appenders_.push_back(appender);
    }
    void Logger::del_appender(LogAppender::ptr appender){
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto it = appenders_.begin(); it!= appenders_.end(); ++it){
            if(*it == appender){
                appenders_.erase(it);
                return;
            }
        }
        
    }
    void Logger::clear_appenders(){
        std::lock_guard<std::mutex> lock(mutex_);
        appenders_.clear();
    }
    
    //setters
    void Logger::set_formatter(const std::string& formatter_str){
        auto formatter = std::make_shared<LogFormatter>(formatter_str);
        if(formatter->is_error()){
            std::cout << "Logger setFormatter name=" << name_
                  << " value=" << formatter_str << " invalid formatter"
                  << std::endl;
            return;
        }
        set_formatter(formatter);
    };

    void Logger::log(LogLevel::Level level, LogEvent::ptr event){
        if(level < level_){
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        if(!appenders_.empty()){
            for(auto appender : appenders_){
                appender->log(event,level);
            }
        }
            
    }

    void Logger::debug(LogEvent::ptr event){
        log(LogLevel::debug, event);
    }
    void Logger::verbose(LogEvent::ptr event){
        log(LogLevel::verbose, event);
    }
    void Logger::notice(LogEvent::ptr event){
        log(LogLevel::notice , event);
    }
    void Logger::warning(LogEvent::ptr event){
        log(LogLevel::warning , event);
    }
    void Logger::error(LogEvent::ptr event){
        log(LogLevel::error , event);
    }


    //LogAppender

    void LogAppender::setFormatter(const std::shared_ptr<LogFormatter>& formatter){
        std::lock_guard<std::mutex> lock(mutex_);

        formatter_ = formatter;
        if(formatter){
            hasFormatter_ = true;
        }else{
            hasFormatter_ = false;
        }
    }


    FileLogAppender::FileLogAppender(){
        time_t t;
        time(&t);
        struct tm tm;
        localtime_r(&t, &tm);
        std::ostringstream oss;
        oss << (tm.tm_year + 1900)<< "-" << tm.tm_mon << "-" << tm.tm_mday << "server.log";

        filename_  = oss.str();
        reopen();
    }

    FileLogAppender::FileLogAppender(const std::string& filename){
        filename_ = filename;
        reopen();
    }
    bool FileLogAppender::reopen(){
        std::lock_guard<std::mutex> lock(mutex_);
        if(ofs_){
            ofs_.close();
        } 
        ofs_.open(filename_, std::ios::app);
        if(ofs_.is_open())
            return true;
        else
            return false;
    }
    
        
    void FileLogAppender::log(LogEvent::ptr event, LogLevel::Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        // if(level < level_)
        //     return;
        formatter_->format(ofs_,level,event);
    }

    void StdoutLogAppender::log(LogEvent::ptr event, LogLevel::Level level){
        std::lock_guard<std::mutex> lock(mutex_);
        // if(level < level_)
        //     return;
        formatter_->format(std::cout,level, event) << std::flush;
    }
}