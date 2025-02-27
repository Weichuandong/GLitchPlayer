//
// Created by WeiChuandong on 2025/2/27.
//

#ifndef VIDEOPLAYER_LOGGER_H
#define VIDEOPLAYER_LOGGER_H

#pragma once
#include <memory>
#include <filesystem> // C++17 文件系统库
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

class Logger {
public:
    static void init(bool logToFile = false, const std::string& logDir = "logs");
    static std::shared_ptr<spdlog::logger>& getLogger();

private:
    static std::string generateLogFileName(const std::string& logDir);
    static std::shared_ptr<spdlog::logger> logger;
};

// 简化日志调用宏（自动包含文件名和行号）
#define LOG_TRACE(...)    Logger::getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Logger::getLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     Logger::getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     Logger::getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    Logger::getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getLogger()->critical(__VA_ARGS__)

#endif //VIDEOPLAYER_LOGGER_H
