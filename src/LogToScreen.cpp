#include "LogToScreen.h"

#include <iostream>

LogToScreen::LogToScreen(const std::string& traceId): LogBase(traceId)
{
}

void LogToScreen::LogFunction(const std::shared_ptr<TraceMessage>& message)
{
  if (message->m_level >= m_traceLevelFilter)
  {
    if (message->m_traceId.empty())
    {
      std::cout << message->m_string.c_str() << std::endl;
    }
    else
    {
      std::cout << message->m_traceId << " | " << message->m_string.c_str() << std::endl;
    }
  }
}

