#include "PeopleCallsGenerator.h"

#include "Call.h"
#include "Management.h"
#include "Floors.h"
#include "People.h"
#include "Watchdog.h"
#include "Configuration.h"

#include <random>
#include <chrono>
#include <future>
#include <memory>
#include <functional>

using namespace std::chrono_literals;
using namespace Configuration::CallsGenerator;

PeopleCallsGenerator::PeopleCallsGenerator(Management& management) : m_management(management)
{
  m_log.SetTraceId("Generator");
}

PeopleCallsGenerator::~PeopleCallsGenerator()
{
  Shutdown();
}

void PeopleCallsGenerator::StartRandom(const unsigned int numberOfCalls)
{
  if (m_working)
    Shutdown();

  m_thread = std::make_unique<std::thread>(&PeopleCallsGenerator::Random, this, numberOfCalls);
}

void PeopleCallsGenerator::StartFixed()
{
  if (m_working)
    Shutdown();

  m_thread = std::make_unique<std::thread>(&PeopleCallsGenerator::Fixed, this);
}

void PeopleCallsGenerator::Shutdown()
{
  if (m_shutdownRequested || m_thread == nullptr)
    return;
  
  m_log.Trace("Shutdown requested...", Log::TraceLevel::Verbose);

  const auto callback = std::bind(
    [this](const unsigned int) -> void { m_log.Trace("** SHUTDOWN IS TAKING TOO LONG **", ILog::TraceLevel::Warning); },
    std::placeholders::_1);

  Watchdog watchdog(0, 60s, callback);

  m_shutdownRequested = true;

  if (m_thread->joinable())
    m_thread->join();

  watchdog.Stop();

  m_thread.reset();  

  m_log.Trace("Shutdown completed", Log::TraceLevel::Verbose);
}

void PeopleCallsGenerator::Random(const unsigned int numberOfCalls)
{
  m_working = true;

  std::this_thread::sleep_for(5s); // arbitrary delay before start

  unsigned int numberOfGeneratedCalls = 0;

  const auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
  std::default_random_engine generator(seed);

  const std::uniform_int_distribution<Floors::FloorNumber> randomFloor(Floors::BottomFloor, Floors::TopFloor);
  const std::uniform_int_distribution<long long> randomDelay(MinDelayBetweenCalls, MaxDelayBetweenCalls);

  do
  {
    std::shared_ptr<Call> call;

    do
    {
      auto getStartFloor = std::bind(randomFloor, std::ref(generator));
      auto getDestinationFloor = std::bind(randomFloor, std::ref(generator));

      call = std::make_shared<Call>(getStartFloor(), getDestinationFloor());
    } while (!call->IsValid()); // only valid calls

    m_log.Trace("Generated call " + call->ToString());

    const auto it = Floors::GetPeople().Insert(call);

    // Async assign request
    auto handle = std::async(std::launch::async, [this, &it]() {m_management.AssignCall(*it); });

    // Synch assign request
    //m_management.AssignCall(*it);

    ++numberOfGeneratedCalls;
    if (numberOfCalls != EndlessCalls && numberOfGeneratedCalls >= numberOfCalls)
      break;

    auto getDelay = std::bind(randomDelay, std::ref(generator));
    std::this_thread::sleep_for(std::chrono::milliseconds(getDelay()));

  } while (!m_shutdownRequested);

  m_working = false;
  m_log.Trace("Thread exit", ILog::TraceLevel::Debug);
}

void PeopleCallsGenerator::Fixed()
{
  m_working = true;

  const auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
  std::default_random_engine generator(seed);

  const std::uniform_int_distribution<long long> randomDelay(MinDelayBetweenCalls, MaxDelayBetweenCalls);

  std::this_thread::sleep_for(5s); // arbitrary delay before start

  static constexpr auto topFloor = Floors::TopFloor; // workaround to avoid an obscure linking problem with g++
  static constexpr auto bottomFloor = Floors::BottomFloor;

  std::list<std::shared_ptr<Call>> calls = {
    std::make_shared<Call>(2,1),
    std::make_shared<Call>(5,1),
    std::make_shared<Call>(6,1),
    std::make_shared<Call>(7,1),
    std::make_shared<Call>(8,1),

    std::make_shared<Call>(0,8),
    std::make_shared<Call>(2,8),
    std::make_shared<Call>(3,8),
    std::make_shared<Call>(4,8),
    std::make_shared<Call>(6,8),

    std::make_shared<Call>(1, topFloor),
    std::make_shared<Call>(3, topFloor),
    std::make_shared<Call>(5, topFloor),
    std::make_shared<Call>(7, topFloor),
    std::make_shared<Call>(8, topFloor),

    std::make_shared<Call>(bottomFloor, 2),
    std::make_shared<Call>(bottomFloor, 4),
    std::make_shared<Call>(bottomFloor, 6),
    std::make_shared<Call>(bottomFloor, 8),
    std::make_shared<Call>(bottomFloor, 9)
  };

  for (const auto& call : calls)
  {
    if (!call->IsValid())
      continue;

    m_log.Trace("Asking call assignment " + call->ToString());

    const auto it = Floors::GetPeople().Insert(call);

    // Async assign request
    auto handle = std::async(std::launch::async, [this, &it]() {m_management.AssignCall(*it); });

    // Synch assign request
    //m_management.AssignCall(*it);

    auto getDelay = std::bind(randomDelay, std::ref(generator));
    std::this_thread::sleep_for(std::chrono::milliseconds(getDelay()));
  }

  m_working = false;
}