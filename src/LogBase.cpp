#include "LogBase.h"

#include "Configuration.h"

#include <sstream>

LogBase::TraceLevel LogBase::m_traceLevelFilter = Configuration::Log::TraceLevel;

std::deque<std::shared_ptr<LogBase::TraceMessage>> LogBase::m_messageQueue;
std::unique_ptr<std::mutex> LogBase::m_messageQueueMutex;

std::unique_ptr<std::condition_variable> LogBase::m_go;
std::unique_ptr<std::mutex> LogBase::m_goMutex;

std::atomic_bool LogBase::m_working{false};
std::atomic_bool LogBase::m_shutdownRequested{false};

std::unique_ptr<std::thread> LogBase::m_traceThread;

LogBase::LogBase(const std::string& traceId)
{
  m_traceId = traceId;

  Start();
}

LogBase::~LogBase()
{
  Shutdown();
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
  // NOTE: Order of creation is important
  if (m_goMutex == nullptr)
    m_goMutex = std::make_unique<std::mutex>();

  if (m_go == nullptr)
    m_go = std::make_unique<std::condition_variable>();

  if (m_messageQueueMutex == nullptr)
    m_messageQueueMutex = std::make_unique<std::mutex>();

  if (m_traceThread == nullptr)
    m_traceThread = std::make_unique<std::thread>(&LogBase::TraceThreadFunction, this);
}

void LogBase::Shutdown()
{
  if (m_traceThread == nullptr || m_shutdownRequested)
    return;

  m_shutdownRequested = true;

  if (m_traceThread->joinable())
    m_traceThread->join();

  m_traceThread.reset();
}

void LogBase::Enqueue(const std::shared_ptr<TraceMessage>& traceMessage)
{
  std::lock_guard<std::mutex> lockMessageQueue(*m_messageQueueMutex);
  m_messageQueue.push_back(traceMessage);

  m_go->notify_all();
}

void LogBase::TraceThreadFunction(LogBase* _this)
{
  m_working = true;

  do
  {
    std::unique_lock<std::mutex> lock(*m_goMutex);
    m_go->wait(lock);

    if (m_shutdownRequested)
      break;

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
