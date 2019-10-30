#include "Elevator.h"
#include "Log.h"
#include "Watchdog.h"
#include "Configuration.h"

using namespace Configuration::Elevator;

Elevator::Elevator(const std::string& id)
{
  SetId(id);

  m_thread = std::make_unique<std::thread>(&Elevator::ElevatorThreadFunction, this);
}

Elevator::~Elevator()
{
  ShutDown();
}

void Elevator::Stop()
{
  m_status = ElevatorStatus::Idle;
  m_log.Trace("Stopped");

  OpenDoors();
}

bool Elevator::AnswerToCall(const std::shared_ptr<Call>& call)
{
  m_log.Trace("Call received");
  m_log.Trace("Current floor: " + std::to_string(m_currentFloor));

  if (!call->IsValid())
  {
    m_log.Trace("*** Invalid call ***", Log::TraceLevel::Warning);
    return false;
  }

  m_floors.SetStop(call);
  m_floors.Trace(m_currentFloor);

  if (m_currentDirection == Direction::None)
    m_currentDirection = call->GetDirection();

  m_go.notify_all();
  return true;
}

void Elevator::ElevatorThreadFunction()
{
  m_log.Trace("Working", Log::TraceLevel::Verbose);
  m_working = true;

  do
  {
    CloseDoors();
    m_log.Trace("Waiting...");

    if (!m_people.Empty())
      m_log.Trace("** ERROR ** The elevator is in idle but there are still people inside", Log::TraceLevel::Error);

    if (m_shutdownRequested)
      break;

    std::unique_lock<std::mutex> lock(m_goMutex);
    m_go.wait(lock);

    auto nextFloor = m_floors.GetNextStop(m_currentFloor, m_currentDirection);

    while (Floors::IsValid(nextFloor) && !m_shutdownRequested) // continue until there are stops in current direction and shutdown is not requested
    {
      m_log.Trace(m_currentDirection == Direction::Up ? "Current direction: UP" : "Current direction: DOWN");

      if(nextFloor != m_currentFloor)
      {
        Move(nextFloor);
      }
      else
      {
        m_floors.ClearStop(m_currentFloor, m_currentDirection);
      }
        
      PeopleEnterAndExit();
      
      nextFloor = m_floors.GetNextStop(m_currentFloor, m_currentDirection);
    }

    m_currentDirection = Direction::None;
  }
  while (!m_shutdownRequested);

  m_working = false;
  m_log.Trace("Thread exit", ILog::TraceLevel::Debug);
}

bool Elevator::OpenDoors()
{
  if (m_status != ElevatorStatus::Idle)
  {
    m_log.Trace("OpenDoors error: elevator is not idle", Log::TraceLevel::Error);
    return false;
  }

  if (m_doorsStatus == DoorsStatus::Closed)
  {
    std::this_thread::sleep_for(1s);
    m_doorsStatus = DoorsStatus::Open;
    m_log.Trace("Doors open", Log::TraceLevel::Verbose);
  }

  return true;
}

bool Elevator::CloseDoors()
{
  if (m_status != ElevatorStatus::Idle)
  {
    m_log.Trace("CloseDoors error: elevator is not idle", Log::TraceLevel::Error);
    return false;
  }

  if (m_doorsStatus == DoorsStatus::Open)
  {
    std::this_thread::sleep_for(1s);
    m_doorsStatus = DoorsStatus::Closed;
    m_log.Trace("Doors closed", Log::TraceLevel::Verbose);
  }

  return true;
}

void Elevator::PeopleEnterAndExit()
{
  const auto previousStatus = m_status;
  m_status = ElevatorStatus::PeopleEnterAndExit;

  m_log.Trace("People Enter and Exit...", Log::TraceLevel::Verbose);

  m_people.EnterAndExit(Floors::GetPeople(), m_currentFloor, m_currentDirection, m_elevatorId);

  RestoreDestinationStops();

  std::this_thread::sleep_for(EnterAndExitTime);

  m_status = previousStatus;
}

/**
 * \brief This function ensures that every time a person (call) enters the elevator, 
 * the destination is correctly (re)set in the floors stops array.
 */
void Elevator::RestoreDestinationStops()
{
  // This is a workaround.
  // When a call is assigned the stops are set in the floor array and 
  // cleared once reached the start or destination floor; this mechanism can fail in some cases:
  // imagine that the elevator is managing a first call from floor 6 to 1
  // and while is moving from 6 to 1, another call arrives from floor 8 to 1;
  // once reached the floor 1 the stop is cleared, but the second call is still to be managed,
  // so in the next step the elevator go to floor 8, but the destination has been lost and 
  // people remain in the elevator.
  // An alternative solution could be to implement a sort of reference counting of the floors stops
  // but this is the simplest solution.

  // Refresh the stops based on the people inside
  for (auto& person : m_people.GetList()) 
  {
    if (person->GetDestinationFloor() != m_currentFloor)
    {
      m_log.Trace("Restored stop " + person->ToString(), Log::TraceLevel::Debug);
      m_floors.SetStop(person, true);
    }
  }
}

void Elevator::Move(const Floors::FloorNumber requestedFloor)
{
  if (requestedFloor != m_currentFloor)
  {
    CloseDoors();

    do
    {
      m_status = ElevatorStatus::Moving;

      if (requestedFloor > m_currentFloor && m_currentFloor < Floors::TopFloor)
      {
        m_log.Trace("Moving Up [" + std::to_string(m_currentFloor) + "]", Log::TraceLevel::Verbose);
        std::this_thread::sleep_for(TimeToReachTheNextFloor); 
        ++m_currentFloor;        
      }
      else if (requestedFloor < m_currentFloor && m_currentFloor > 0)
      {
        m_log.Trace("Moving Down [" + std::to_string(m_currentFloor) + "]", Log::TraceLevel::Verbose);
        std::this_thread::sleep_for(TimeToReachTheNextFloor); 
        --m_currentFloor;
      }

    } while (m_currentFloor != requestedFloor && !m_shutdownRequested);

    const std::string message = "Arrived on the floor " + std::to_string(m_currentFloor);
    m_log.Trace(message);
  }

  m_floors.ClearStop(m_currentFloor, m_currentDirection);
  Stop();
}

void Elevator::ShutDown()
{
  if (m_shutdownRequested || m_thread == nullptr)
    return;

  m_log.Trace("Shutdown in progress...", Log::TraceLevel::Verbose);

  const auto callback = std::bind(
    [this](const std::string&) -> void { m_log.Trace("** SHUTDOWN IS TAKING TOO LONG **", ILog::TraceLevel::Warning); }, 
    std::placeholders::_1);

  Watchdog watchdog(m_name, 20s, callback);

  m_shutdownRequested = true;
  m_go.notify_one();

  if (m_thread->joinable())
    m_thread->join();

  watchdog.Stop();

  m_thread.reset();

  m_log.Trace("Shutdown completed", Log::TraceLevel::Verbose);
 }

void Elevator::SetId(std::string id)
{
  m_elevatorId = std::move(id);
  m_name = "Elevator " + m_elevatorId;

  m_people.SetId(m_name);
  m_floors.SetId(m_name);
  m_log.SetTraceId(m_name);
}

bool Elevator::Available(const std::shared_ptr<Call>& call)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!call->IsValid())
    return false;

  if (m_status == ElevatorStatus::OutOfOrder)
    return false;

  if (m_status == ElevatorStatus::Idle || m_currentDirection == Direction::None)
    return true;

  if (call->GetDirection() == m_currentDirection  && m_currentDirection == Direction::Up && call->GetStartFloor() > m_currentFloor)
    return true;
  
  if (call->GetDirection() == m_currentDirection && m_currentDirection == Direction::Down && call->GetStartFloor() < m_currentFloor)
    return true;

  return false;
}


