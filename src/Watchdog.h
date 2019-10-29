/**********************************************************************************
*        File: Watchdog.h
* Description: Implements a simple timer which call the passed function if expires.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

#include "Log.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

/**
 * \brief Implements a simple timer which call the passed function if expires.
 */
class Watchdog final
{
public: 
  /**
   * \brief Default constructor.
   * \param id Id of the watchdog.
   */
  explicit Watchdog(const std::string& id): m_id(id), m_log(id)
  {
  }

  /**
   * \brief Construct and start the watchdog.
   * \param id Id of the watchdog.
   * \param timeout Milliseconds before the timer expires.
   * \param timeoutCallback Function called if the timer expires.
   */
  Watchdog(
    const std::string& id, 
    const std::chrono::milliseconds timeout,
    const std::function<void(const std::string&)>& timeoutCallback): m_id(id), m_log(id)
  {
    Start(timeout, timeoutCallback);
  }

  ~Watchdog()
  {
    Stop();
  }

  Watchdog(const Watchdog&) = delete;
  Watchdog& operator=(const Watchdog&) = delete;

  Watchdog(Watchdog&&) = delete;
  Watchdog& operator=(Watchdog&&) = delete;

public: 
  /**
   * \brief Start the watchdog. 
   * \param timeout Milliseconds before the timer expires.
   * \param timeoutCallback Function called if the timer expires.
   * \return True if the watchdog has been correctly started. If it was already started it will be restarted.
   */
  bool Start(
    const std::chrono::milliseconds timeout,
    const std::function<void(const std::string&)>& timeoutCallback)
  {
    if (m_started)
      Stop();

    m_guardFunctionResult = std::async(
      std::launch::async,
      bind(&Watchdog::GuardFunction, this, timeout, timeoutCallback));

      m_log.Trace("Watchog started", ILog::TraceLevel::Debug);

    return true;
  }
    
  /**
   * \brief Stop the watchdog.
   * \return True if the watchdog has been correctly stopped. 
   */
  bool Stop()
  {
    m_log.Trace("Watchog stop requested...", ILog::TraceLevel::Debug);

    m_stopped = true;
    m_condition.notify_one();
    m_guardFunctionResult.wait();

    m_log.Trace("Watchog stopped", ILog::TraceLevel::Debug);

    return true;
  }

  bool IsStopped() { return m_stopped; }

private: 
  bool GuardFunction(
    const std::chrono::milliseconds timeout,
    const std::function<void(const std::string&)>& timeoutCallback)
    {
      TrueInScope toggle(m_started);

      m_log.Trace("Watchog waiting...", ILog::TraceLevel::Debug);

      std::mutex mutex;
      std::unique_lock<std::mutex> lock(mutex);

      while(!IsStopped())
      {
        if (!m_condition.wait_for(lock, timeout, [this](){ return IsStopped();} ))
        {
          m_log.Trace("Watchog timeout", ILog::TraceLevel::Debug);
          timeoutCallback(m_id);
        }
      }
      
      m_log.Trace("Watchog exit", ILog::TraceLevel::Debug);

      return m_stopped;
    }

private: 
  std::string m_id;

  std::condition_variable m_condition;

  std::atomic_bool m_started{ false };
  std::atomic_bool m_stopped{ false };

  std::future<bool> m_guardFunctionResult;

  Log m_log;

private:
  class TrueInScope
  {
    public:
      TrueInScope(std::atomic_bool& variable) : m_variable(variable) { m_variable = true; }   
      ~TrueInScope() { m_variable = false; }  

    private:
      std::atomic_bool& m_variable;
  };

};


