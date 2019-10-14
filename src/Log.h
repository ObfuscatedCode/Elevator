#pragma once

#include "ILog.h"
#include "Configuration.h"

#include <memory>
#include <sstream>

/**
 * \brief Implements the log service factory
 */
class Log final : public ILog
{
public:
  explicit Log(const std::string& traceId = "", const LogType logType = Configuration::Log::DefaultLogType);
  virtual ~Log() = default;

  Log(const Log&) = default;
  Log(Log&&) = default;

  Log& operator=(const Log&) = default;
  Log& operator=(Log&&) = default;

public:
  void Trace(
    const std::stringstream& message, 
    const TraceLevel level = TraceLevel::Info, 
    const std::string& messageSpecificId = "") const override;

  void Trace(
    const std::string& message, 
    const TraceLevel level = TraceLevel::Info, 
    const std::string& messageSpecificId = "") const override;

  void SetTraceId(const std::string& traceId) override;
  const std::string& GetTraceId() const override;

  void SetTraceLevelFilter(const TraceLevel startLevel) override;

private:
  std::shared_ptr<ILog>& GetLog(const std::string& traceId);

private:
  std::shared_ptr<ILog> m_implementation{ nullptr };
  LogType m_logType{ LogType::Default };
};

