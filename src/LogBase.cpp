#include "LogBase.h"

#include "Watchdog.h"
#include "Configuration.h"

#include <sstream>
#include <utility>

std::atomic_bool LogBase::m_started{ false };

std::deque<std::shared_ptr<LogBase::TraceMessage>> LogBase::m_messageQueue;
std::unique_ptr<std::mutex> LogBase::m_messageQueueMutex{ std::make_unique<std::mutex>() };

std::unique_ptr<std::condition_variable> LogBase::m_go{ std::make_unique<std::condition_variable>() };
std::unique_ptr<std::mutex> LogBase::m_goMutex{ std::make_unique<std::mutex>() };

std::atomic_bool LogBase::m_working{ false };
std::atomic_bool LogBase::m_shutdownRequested{ false };

LogBase::TraceLevel LogBase::m_traceLevelFilter{ Configuration::Log::TraceLevel };

std::unique_ptr<std::thread> LogBase::m_traceThread;

LogBase::LogBase(std::string traceId) : m_traceId(std::move(traceId))
{
  Start();
}

LogBase::~LogBase()
{
  Stop();
}

void LogBase::Trace(const std::stringstream& message, const TraceLevel level, const std::string& messageSpecificId) const
{
  Enqueue(std::make_shared<TraceMessage>(message.str(), messageSpecificId.empty() ? m_traceId : messageSpecificId, level));
}

void LogBase::Trace(const std::string& message, const TraceLevel level, const std::string& messageSpecificId) const
{
  Enqueue(std::make_shared<TraceMessage>(message, messageSpecificId.empty() ? m_traceId : messageSpecificId, level));
}

void LogBase::Start()
{
  if (m_started)
    return; // already started

  if (m_traceThread == nullptr)
    m_traceThread = std::make_unique<std::thread>(&LogBase::TraceThreadFunction, this);

  m_started = true;
}

void LogBase::Stop()
{
  if (!m_started)
    return; // not started

  if (m_traceThread == nullptr || m_shutdownRequested)
    return;

  m_shutdownRequested = true;
  m_go->notify_one();

  if (m_traceThread->joinable())
    m_traceThread->join();

  m_traceThread.reset();

  m_started = false;
}

void LogBase::Enqueue(const std::shared_ptr<TraceMessage>& traceMessage)
{
  if (m_messageQueueMutex == nullptr || m_go == nullptr) // Preconditions
    return;
    
  std::lock_guard<std::mutex> lockMessageQueue(*m_messageQueueMutex);
  m_messageQueue.push_back(traceMessage);

  m_go->notify_all();
}

void LogBase::TraceThreadFunction(LogBase* _this)
{
  if (m_messageQueueMutex == nullptr || m_go == nullptr) // Preconditions
    return;

  m_working = true;

  do
  {
    if (m_shutdownRequested)
      break;

    std::unique_lock<std::mutex> lock(*m_goMutex);
    m_go->wait(lock);

    while (!m_messageQueue.empty() && !m_shutdownRequested)
    {
      std::lock_guard<std::mutex> lockMessageQueue(*m_messageQueueMutex);

      const auto message = std::move(m_messageQueue.front());
      m_messageQueue.pop_front();

      _this->LogFunction(message);
    }
  } while (!m_shutdownRequested);

  m_working = false;
}
