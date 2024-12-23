#ifndef _lili_LOG_H_
#define _lili_LOG_H_

#include <stdio.h>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include <thread>
#include <sstream>
#include <fstream>

#define THEFOOL_LOG(logger,levell) \
    if(logger->get_level() <= levell) \
        thefool::LogEventWrapper(logger, std::make_shared<thefool::LogEvent>(logger, __FILE__, __LINE__,levell, std::this_thread::get_id(),"threadname", 12,time(0))).getSS()

#define THEFOOL_LOG_DEBUG(logger) THEFOOL_LOG(logger,thefool::LogLevel::debug)

#define THEFOOL_LOG_VERBOSE(logger) THEFOOL_LOG(logger,thefool::LogLevel::verbose)

#define THEFOOL_LOG_NOTICE(logger) THEFOOL_LOG(logger,thefool::LogLevel::notice)

#define THEFOOL_LOG_WARNING(logger) THEFOOL_LOG(logger,thefool::LogLevel::warning)

#define THEFOOL_LOG_ERROR(logger) THEFOOL_LOG(logger,thefool::LogLevel::error)

namespace thefool{
class LogManager;

class LogEvent;
class LogFormatter;
class LogAppender;
class LogLevel;
class Logger;
//log 等级
class LogLevel{
    public:
        enum Level{
            //调试信息,最详细
            debug , 

            //详细信息
            verbose,
            
            //生产环境信息
            notice,

            //警告信息
            warning,

            //错误信息
            error
        };
    
    static std::string level_to_string(Level level);
    static Level string_to_level(const std::string& level_str);
};



class LogEvent{
    public:
        using ptr = std::shared_ptr<LogEvent>;
        LogEvent(
                const std::shared_ptr<Logger>& logger,
                const std::string& file, 
                int32_t line, 
                LogLevel::Level level,
                std::thread::id threadId,
                const std::string& threadName,
                uint32_t fibreId,
                uint64_t time);
      //  ~LogEvent();
        //getters
        const std::string& get_file() const { return file_; };
        
        int32_t get_line() const { return line_; };
       
       // const std::string& get_function() const { return function_; };
       
        LogLevel::Level get_level() const { return level_; };
      
        const std::string get_message() const {return std::move(message_.str());   };
      
        uint64_t get_time() const { return time_; };
      
        std::thread::id get_thread_id() const { return threadId_; };
      
        const std::string& get_thread_name() const { return threadName_; };
       
        uint32_t get_fibre_id() const { return fibreId_; };
       
        std::stringstream& getSS() { return message_; };

        std::shared_ptr<Logger> get_logger() const { return logger_; };
    private:
        //文件名
        std::string file_;
        
        //行号
        int32_t line_;

        //日志信息
        std::stringstream message_;

        //日志等级
        LogLevel::Level level_;

        //日志时间
        uint64_t time_;
        
        //线程ID
        std::thread::id threadId_;

        //线程名称
        std::string threadName_;

        //协程ID
        uint32_t fibreId_;

        //日志器
        std::shared_ptr<Logger> logger_;

        
};


//日志输出目标
class LogAppender{
    friend class Logger;
    public:
        using ptr = std::shared_ptr<LogAppender>;

        virtual void log(LogEvent::ptr event, LogLevel::Level level){};

        virtual ~LogAppender(){};

        void setFormatter(const std::shared_ptr<LogFormatter>& formatter);

        std::shared_ptr<LogFormatter> getFormatter() const { return formatter_; };
    protected:
        //日志输出目标名称
        std::string name_;

        std::shared_ptr<LogFormatter> formatter_ = nullptr;

      //  LogLevel::Level level_;

        std::mutex mutex_;

        bool hasFormatter_ = false;

};


//日志格式化器
class LogFormatter{
    public:
        using ptr = std::shared_ptr<LogFormatter>;
    /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *  %k 空格
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    LogFormatter(const std::string& pattern);
    
     /**
     * @brief 返回格式化日志文本
     * @param[in] logger 日志器
     * @param[in] level 日志级别
     * @param[in] event 日志事件
     */
    std::string format( LogLevel::Level level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, LogLevel::Level level, LogEvent::ptr event);

     /**
     * @brief 初始化,解析日志模板
     */
    void init();
    
    /**
     * @brief 是否有错误
     */
    bool is_error() const{ return error_; };

    const std::string getPattern() const { return pattern_;}
    public:
        /**
        * @brief 日志内容项格式化
        */
        class FormatItem{
        public:
            using ptr = std::shared_ptr<FormatItem>;

            // FormatItem();

            virtual void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) = 0;
            
           
        };

    private:


        // 日志格式模板
        std::string pattern_;

        //日志格式解析后格式
        std::vector<FormatItem::ptr> items_;

        //是否有错误
        bool error_;

};

//日志记录器
class Logger{
    public:
        //typedef std::shared_ptr<Logger> ptr;
        using ptr = std::shared_ptr<Logger>;
        
        Logger(const std::string& name = "root");
       
      //  ~Logger();
      
        void add_appender(const LogAppender::ptr& appender);
    
        void del_appender(LogAppender::ptr appender);
     
        void clear_appenders();
        
        //setters
     
        void set_level(LogLevel::Level level){ level_= level_; };
     
        void set_formatter(const LogFormatter::ptr& formatter){ formatter_ = formatter_; };
      
        void set_formatter(const std::string& formatter_str);
        //getters
        LogLevel::Level get_level() const { return level_; };
      
        std::string get_name() const {return name_;};
      
        LogFormatter::ptr get_formatter() const { return formatter_; };

        void log(LogLevel::Level level, LogEvent::ptr event);
      
        void debug(LogEvent::ptr __event);
       
        void verbose(LogEvent::ptr __event);
      
        void notice(LogEvent::ptr __event);
      
        void warning(LogEvent::ptr __event);
       
        void error(LogEvent::ptr __event);

    private:
        //日志器名称
        std::string name_;
        //日志级别
        LogLevel::Level level_;
        //互斥锁
        std::mutex mutex_;
        //日志输出目标
        std::list<LogAppender::ptr> appenders_;
        //日志格式化器
        LogFormatter::ptr formatter_;

};


//文件日志输出目标
class FileLogAppender: public LogAppender{
    public:
        using ptr = std::shared_ptr<FileLogAppender>;
        FileLogAppender();
        FileLogAppender(const std::string& filename);
        void log(LogEvent::ptr event, LogLevel::Level level) ;

        bool reopen();
    private:

        //文件名
        std::string filename_;

        std::ofstream ofs_;

        uint64_t last_flush_time_ = 0;


};

//标准输出日志输出目标
class StdoutLogAppender: public LogAppender{
    public:
        using ptr = std::shared_ptr<StdoutLogAppender>;
       // void log(LogEvent::ptr event, LogLevel::Level level) override;
        void log(LogEvent::ptr event, LogLevel::Level level) ;

};


//日志管理器
class LogManager{
    public:
        using ptr = std::shared_ptr<LogManager>;

    private:
};


class LogEventWrapper{
    public:
        using ptr = std::shared_ptr<LogEventWrapper>;
        LogEventWrapper(Logger::ptr logger, LogEvent::ptr event):logger_(logger), event_(event){

        };
        ~LogEventWrapper(){
            logger_->log(event_->get_level(), event_);
        };
        std::stringstream& getSS(){ return event_->getSS(); };

        Logger::ptr logger_;
        LogEvent::ptr event_;
};
}

#endif /* _lili_LOG_H_ */