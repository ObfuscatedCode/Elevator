#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>


class Watchdog final
{
public: // Constructors and destructors
  Watchdog(
    const unsigned int id,
    const std::chrono::milliseconds timeout,
    const std::function<void(const unsigned int)>& timeoutCallback) : m_id(id)
  {
    Start(timeout, timeoutCallback);
  }

  ~Watchdog()
  {
    Stop();
  }

public: // Deleted constructors and operators
  Watchdog() = delete;

  Watchdog(const Watchdog&) = delete;
  Watchdog& operator=(const Watchdog&) = delete;

  Watchdog(Watchdog&&) = delete;
  Watchdog& operator=(Watchdog&&) = delete;

public: // Methods
  void Stop()
  {
    m_stop = true;
    m_condition.notify_all();
    m_guardFunctionResult.wait();    
  }

private: // Methods
  void Start( 
    const std::chrono::milliseconds timeout,
    const std::function<void(const unsigned int)>& timeoutCallback)
  {
    m_guardFunctionResult = std::async(
      std::launch::async, 
      bind(&Watchdog::GuardFunction, this, timeout, timeoutCallback));
  }

  bool GuardFunction(
    const std::chrono::milliseconds timeout,
    const std::function<void(const unsigned int)>& timeoutCallback)
  {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);

    if (!m_condition.wait_for(lock, timeout, [this]() -> bool { return m_stop; }))
    {
      timeoutCallback(m_id);
      return false;
    }

    return true;
  }

private: // Variables
  unsigned int m_id = 0;

  std::condition_variable m_condition;
  std::atomic_bool m_stop = false;

  std::future<bool> m_guardFunctionResult;
};

