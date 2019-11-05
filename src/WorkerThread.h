/**********************************************************************************
*        File: WorkerThread.h
* Description: Implements a thread which call a user defined function inside a cycle 
*              and the methods to control it.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

#include <thread>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <cassert>

/**
 * \brief Implements a thread which call a user defined function inside a cycle. 
 * \tparam T Type of the owner.
 */
template<class T>
class WorkerThread
{
public:
  /**
   * \brief Default constructor.
   * \param owner Owner of the thread, will be passed to the user-defined cycle function. 
   */
  explicit WorkerThread(T* owner = nullptr) : m_owner{ owner } {}

  virtual ~WorkerThread() { Stop(); }

  WorkerThread(const WorkerThread&) = delete;
  WorkerThread(WorkerThread&&) = delete;

  WorkerThread& operator=(const WorkerThread&) = delete;
  WorkerThread& operator=(WorkerThread&&) = delete;

public:
  /**
   * \brief Start the thread in wait state. 
   */
  void Start()
  {
    if (m_thread == nullptr)
      m_thread = std::make_unique<std::thread>([&]() {ThreadFunction(); });

    while (!IsActive())
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  /**
   * \brief Stop the thread.
   */
  void Stop()
  {
    if (m_stopRequested)
      return; // already requested

    m_stopRequested = true;
    Go();

    if (m_thread != nullptr && m_thread->joinable())
      m_thread->join();

    m_thread.reset();
  }

  /**
   * \brief Signal the thread that is there something to do: exit the thread from the wait state.
   */
  void Go()
  {
    assert(IsActive() && "** THREAD NOT STARTED **"); // Thread started?

    m_stopWait = true;
    m_go.notify_one();
  }

  /**
   * \brief Indicates if the thread is started.
   * \return 'true' if the thread is started, 'false' otherwise. 
   */
  bool IsActive() const { return m_isActive; }

  /**
   * \brief Indicates if the thread stop is requested
   * \return 'true' if the thread stop is requested, 'false' otherwise.
   */
  bool StopRequested() const { return m_stopRequested; }

  /**
   * \brief Set the owner.
   * \param owner Owner of the thread, will be passed to the user-defined cycle function. 
   */
  void SetOwner(T* owner) { m_owner = owner; }

protected:
  /**
   * \brief User-defined function called inside the thread loop.
   * \param owner Owner of the thread. Can be nullptr. 
   */
  virtual void CycleFunction(T* owner) = 0;

private:
  void ThreadFunction()
  {
    TrueInScope toggle(m_isActive);

    do
    {
      if (!StopRequested())
      {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_go.wait(lock, [this]() {return m_stopWait == true; });
        m_stopWait = false;
      }

      if (!StopRequested())
        CycleFunction(m_owner);

    } while (!StopRequested());
  }

private:
  std::atomic_bool m_isActive{ false };
  std::atomic_bool m_stopRequested{ false };

  std::unique_ptr<std::thread> m_thread;

  std::mutex m_mutex;
  std::condition_variable m_go;
  std::atomic_bool m_stopWait{ false };

  T* m_owner{ nullptr };

protected:
  class TrueInScope final
  {
  public:
    explicit TrueInScope(std::atomic_bool& variable) : m_variable{ variable } { m_variable = true; }
    ~TrueInScope() { m_variable = false; }

    TrueInScope() = delete;
    TrueInScope(const TrueInScope&) = default;
    TrueInScope(TrueInScope&&) = default;

    TrueInScope& operator=(const TrueInScope&) = delete;
    TrueInScope& operator=(TrueInScope&&) = delete;

  private:
    std::atomic_bool& m_variable;
  };

};


