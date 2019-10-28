/**********************************************************************************
*        File: Watchdog.h
* Description: Implements a simple timer which call the passed function if expires.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

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
  explicit Watchdog(const unsigned int id = 0U);

  /**
   * \brief Construct and start the watchdog.
   * \param id Id of the watchdog.
   * \param timeout Milliseconds before the timer expires.
   * \param timeoutCallback Function called if the timer expires.
   */
  Watchdog(
    const unsigned int id,
    const std::chrono::milliseconds timeout,
    const std::function<void(const unsigned int)>& timeoutCallback);

  ~Watchdog();

  Watchdog(const Watchdog&) = delete;
  Watchdog& operator=(const Watchdog&) = delete;

  Watchdog(Watchdog&&) = delete;
  Watchdog& operator=(Watchdog&&) = delete;

public: 
  /**
   * \brief Start the watchdog. 
   * \param timeout Milliseconds before the timer expires.
   * \param timeoutCallback Function called if the timer expires.
   * \return True if the watchdog has been correctly started. False if it was already started.
   */
  bool Start(
    const std::chrono::milliseconds timeout,
    const std::function<void(const unsigned int)>& timeoutCallback);

  /**
   * \brief Stop the watchdog.
   * \return True if the watchdog has been correctly stopped. False if it was not started.
   */
  bool Stop();

private: 
  bool GuardFunction(
    const std::chrono::milliseconds timeout,
    const std::function<void(const unsigned int)>& timeoutCallback);

private: // Variables
  unsigned int m_id = 0U;

  std::condition_variable m_condition;

  std::atomic_bool m_started{ false };
  std::atomic_bool m_stopped{ false };

  std::future<bool> m_guardFunctionResult;
};


inline Watchdog::Watchdog(const unsigned id): m_id(id)
{
}

inline Watchdog::Watchdog(
  const unsigned id, const std::chrono::milliseconds timeout,
  const std::function<void(const unsigned)>& timeoutCallback) : m_id(id)
{
  Start(timeout, timeoutCallback);
}

inline Watchdog::~Watchdog()
{
  Stop();
}

inline bool Watchdog::Start(
  const std::chrono::milliseconds timeout,
  const std::function<void(const unsigned)>& timeoutCallback)
{
  if (m_started)
    return false;

  m_guardFunctionResult = std::async(
    std::launch::async,
    bind(&Watchdog::GuardFunction, this, timeout, timeoutCallback));

  return true;
}

inline bool Watchdog::Stop()
{
  if (!m_started)
    return false;

  m_stopped = true;
  m_condition.notify_all();
  m_guardFunctionResult.wait();

  return true;
}

inline bool Watchdog::GuardFunction(
  const std::chrono::milliseconds timeout,
  const std::function<void(const unsigned)>& timeoutCallback)
{
  m_started = true;

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  if (!m_condition.wait_for(lock, timeout, [this]() -> bool { return m_stopped == true; }))
    timeoutCallback(m_id);

  m_started = false;

  return m_stopped;
}

