/**********************************************************************************
*        File: LogBase.h
* Description: Implements the log service basic logic.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

#include <mutex>
#include <deque>
#include <atomic>
#include <condition_variable>
#include <thread>

#include "ILog.h"

/**
 * \brief Implements the log basic producer/consumer logic with a message queue and a trace thread. 
 */
class LogBase : public ILog
{
protected:
  struct TraceMessage
  {
    typedef std::chrono::steady_clock Clock;
    typedef std::chrono::time_point<Clock> TimeStamp;

    TraceMessage(
      const std::string& message, 
      const std::string& traceId, 
      const TraceLevel level, 
      const TimeStamp& timeStamp = Clock::now())
    {
      m_string = message;
      m_traceId = traceId;
      m_level = level;
      m_timeStamp = timeStamp;
    }

    std::string m_string;
    std::string m_traceId;
    TraceLevel m_level = TraceLevel::Info;
    TimeStamp m_timeStamp;
  };

public:
  explicit LogBase(const std::string& traceId = "");
  virtual ~LogBase();

  LogBase(const LogBase&) = default;
  LogBase(LogBase&&) = default;

  LogBase& operator=(const LogBase&) = default;
  LogBase& operator=(LogBase&&) = default;

public: // ILog 
  void Trace(const std::stringstream& message, const TraceLevel level = TraceLevel::Info, const std::string& messageSpecificId = "") const override;
  void Trace(const std::string& message, const TraceLevel level = TraceLevel::Info, const std::string& messageSpecificId = "") const override;

  void SetTraceId(const std::string& traceId) override { m_traceId = traceId; }
  const std::string& GetTraceId() const override { return m_traceId; }

  void SetTraceLevelFilter(const TraceLevel traceLevelThreshold) override { m_traceLevelFilter = traceLevelThreshold; }

private:
  static void Enqueue(const std::shared_ptr<TraceMessage>& traceMessage);

  void Start();
  void Shutdown();

  static void TraceThreadFunction(LogBase* _this);

  virtual void LogFunction(const std::shared_ptr<TraceMessage>& message) = 0;

private: 
  static std::deque<std::shared_ptr<TraceMessage>> m_messageQueue;
  static std::unique_ptr<std::mutex> m_messageQueueMutex;

  static std::unique_ptr <std::condition_variable> m_go;
  static std::unique_ptr<std::mutex> m_goMutex;

  static std::atomic_bool m_working;
  static std::atomic_bool m_shutdownRequested;

  static std::unique_ptr<std::thread> m_traceThread;

protected:
  std::string m_traceId;

  static TraceLevel m_traceLevelFilter;
};

