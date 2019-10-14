#include "People.h"
#include "Log.h"

#include <sstream>

void People::EnterAndExit(
  People& waitingPeople, 
  const Floors::FloorNumber currentFloor, 
  const Direction currentDirection, 
  const std::string& elevatorId)
{
  waitingPeople.Trace(currentFloor);
  Enter(waitingPeople, currentFloor, currentDirection, elevatorId);
  Exit(currentFloor);
}

std::list<std::shared_ptr<Call>>::iterator People::Insert(const std::shared_ptr<Call>& call)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  push_front(call);
  return begin();
}

void People::Trace(const Floors::FloorNumber currentFloor)
{
  std::stringstream message;

  if (currentFloor != Floors::InvalidFloor)
  {
    message << "People waiting on floor " << std::to_string(currentFloor) << ": ";

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& person : *this)
      if ((*person).GetStartFloor() == currentFloor)
        message << (*person).ToString();
  }
  else
  {
    message << "People waiting on floors: ";

    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& person : *this)
        message << (*person).ToString();
  }

  m_log.Trace(message);
}

bool People::Empty() 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return empty();
}

void People::Enter(
  People& waitingPeople, 
  const Floors::FloorNumber currentFloor, 
  const Direction currentDirection, 
  const std::string& elevatorId)
{
  std::stringstream message;
  message << "People enter from floor " << std::to_string(currentFloor) << ": ";

  std::lock_guard<std::mutex> waitingPeopleLock(waitingPeople.m_mutex);
  auto person = waitingPeople.begin();

  while (person != waitingPeople.end())
  {
    if ((*person)->GetStartFloor() == currentFloor && (*person)->GetDirection() == currentDirection && (*person)->GetAssignedElevator() == elevatorId)
    {
      message << (*person)->ToString();

      std::lock_guard<std::mutex> lock(m_mutex);
      push_front(*person);

      person = waitingPeople.erase(person);
      continue;
    }
      
    ++person;
  }

  m_log.Trace(message);
}

void People::Exit(const Floors::FloorNumber currentFloor)
{
  std::stringstream message;
  message << "People exit to floor " << std::to_string(currentFloor) << ": ";

  std::lock_guard<std::mutex> lock(m_mutex);
  auto person = begin();

  while(person != end())
  {
    if ((*person)->GetDestinationFloor() == currentFloor)
    {
      message << (*person)->ToString();
      person = erase(person);
      continue;
    }

    ++person;
  }

  m_log.Trace(message);
}
