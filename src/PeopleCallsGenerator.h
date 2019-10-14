#pragma once

#include "Log.h"

#include <atomic>
#include <thread>
#include <memory>

class PeopleCallsGenerator final 
{
public:
  explicit PeopleCallsGenerator(class Management& management);
  PeopleCallsGenerator() = delete;

  ~PeopleCallsGenerator();

  PeopleCallsGenerator(const PeopleCallsGenerator&) = delete;
  PeopleCallsGenerator& operator=(const PeopleCallsGenerator&) = delete;

  PeopleCallsGenerator(PeopleCallsGenerator&&) = delete;
  PeopleCallsGenerator operator=(PeopleCallsGenerator&&) = delete;

public:
  void StartRandom(const unsigned int numberOfCalls = static_cast<unsigned int>(-1));
  void StartFixed();

  void Shutdown();

private:

  /**
   * \brief Generate random calls with random frequency within the configuration bounds.  
   * \param numberOfCalls Number of calls to generate, if set to 'Endless' (see Configuration) generate infinite calls.
   */
  void Random(const unsigned int numberOfCalls);

  /**
   * \brief Generate a fixed list of calls useful for test and debug purposes. 
   */
  void Fixed();

private:
  class Management& m_management;

  std::unique_ptr<std::thread> m_thread;
  std::atomic_bool m_shutdownRequested{false};
  std::atomic_bool m_working{false};

  Log m_log;
};


