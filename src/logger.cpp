//
// Created by WeiChuandong on 2025/2/27.
//

#include "logger.h"

std::shared_ptr<spdlog::logger> Logger::logger = nullptr;

// 生成带时间戳的日志文件名
std::string Logger::generateLogFileName(const std::string& logDir) {
    // 创建日志目录（如果不存在）
    std::filesystem::create_directories(logDir);

    // 获取当前时间并格式化
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&nowTime);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y%m%d_%H%M%S");
    return logDir + "/video_player" + oss.str() + ".log";
}

void Logger::init(bool logToFile, const std::string &logDir) {
    std::vector<spdlog::sink_ptr> sinks;

    // 控制台输出（带颜色）
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%Y-%m-%d %T.%e] [%^%l%$] [thread %t] %v");
    sinks.push_back(consoleSink);

    // 文件输出
    if (logToFile) {
        auto logFile = generateLogFileName(logDir);
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
        fileSink->set_pattern("[%Y-%m-%d %T.%e] [%l] [thread %t] %v");
        sinks.push_back(fileSink);
    }

    // 创建日志器
    logger = std::make_shared<spdlog::logger>("VideoPlayer", begin(sinks), end(sinks));
    spdlog::register_logger(logger);

    // 默认日志级别为 DEBUG
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::warn); // 遇到 WARN 及以上级别时立即刷新
}

std::shared_ptr<spdlog::logger>& Logger::getLogger() {
    if (!logger) {
        throw std::runtime_error("Logger 未初始化！");
    }
    return logger;
}