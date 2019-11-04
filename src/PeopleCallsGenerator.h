/**********************************************************************************
*        File: PeopleCallsGenerator.h
* Description: Implements a random and a fixes calls generator.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

#include "Log.h"
#include "WorkerThread.h"

class PeopleCallsGenerator final 
{
private:
  class RandomGeneratorThread final : public WorkerThread<PeopleCallsGenerator>
  {
  public:
    explicit RandomGeneratorThread(PeopleCallsGenerator* peopleCallsGenerator, const unsigned int numberOfCalls) :
      WorkerThread<PeopleCallsGenerator>(peopleCallsGenerator),
      m_numberOfCalls(numberOfCalls) {}

  protected:
    void CycleFunction(PeopleCallsGenerator* peopleCallsGenerator) override;

  private:
    unsigned int m_numberOfCalls = 0;
  };

  class FixedGeneratorThread final : public WorkerThread<PeopleCallsGenerator>
  {
  public:
    explicit FixedGeneratorThread(PeopleCallsGenerator* peopleCallsGenerator = nullptr) :
      WorkerThread<PeopleCallsGenerator>(peopleCallsGenerator) {}

  protected:
    void CycleFunction(PeopleCallsGenerator* peopleCallsGenerator) override;
  };

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
  class Management& m_management;

  Log m_log;

  std::unique_ptr<WorkerThread<PeopleCallsGenerator>> m_generatorThread;
};


