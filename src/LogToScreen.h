/**********************************************************************************
*        File: LogToScreen.h
* Description: Implements a simple log service that use screen to show messages.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

#include "LogBase.h"

/**
 * \brief Implements a simple log to screen.
 */
class LogToScreen final : public LogBase
{
public:
  explicit LogToScreen(const std::string& traceId = "");
  virtual ~LogToScreen() = default;

  LogToScreen(const LogToScreen&) = default;
  LogToScreen(LogToScreen&&) = default;

  LogToScreen& operator=(const LogToScreen&) = default;
  LogToScreen& operator=(LogToScreen&&) = default;

private:
  void LogFunction(const std::shared_ptr<TraceMessage>& message) override;
};

