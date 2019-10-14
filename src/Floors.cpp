﻿#include "Floors.h"

#include <mutex>
#include <sstream>

#include "Call.h"
#include "People.h"
#include "Log.h"

People Floors::m_people("Building");

Floors::Floors()
{
  for (FloorNumber floorNumber = 0; floorNumber < TotalFloors; ++floorNumber)
    m_stops.emplace_back(false, Direction::None);

  m_log.SetTraceId("Building");
}

bool Floors::SetStop(const std::shared_ptr<Call>& call, const bool destinationOnly)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!call->IsValid())
    return false;

  if (!destinationOnly)
  {
    m_stops[call->GetStartFloor()].first = true;

    m_stops[call->GetStartFloor()].second =
      m_stops[call->GetStartFloor()].second != Direction::None &&
      m_stops[call->GetStartFloor()].second != call->GetDirection()
      ? Direction::Both
      : call->GetDirection();
  }

  m_stops[call->GetDestinationFloor()].first = true;

  m_stops[call->GetDestinationFloor()].second =
    m_stops[call->GetDestinationFloor()].second != Direction::None &&
    m_stops[call->GetDestinationFloor()].second != call->GetDirection()
      ? Direction::Both
      : call->GetDirection();

  return true;
}

Floors::FloorNumber Floors::GetNextStop(const FloorNumber currentFloor, Direction& currentDirection)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!IsValid(currentFloor))
    return InvalidFloor;

  if (currentDirection == Direction::None)
    currentDirection = currentFloor < TotalFloors / 2U ? Direction::Down : Direction::Up;

  FloorNumber searchStartFloor = InvalidFloor;

  // If the elevator reached the top or the bottom floor we must invert the direction
  if (currentDirection == Direction::Up)
  {
    searchStartFloor = currentFloor != TopFloor ? currentFloor : BottomFloor;
  }
  else
  {
    searchStartFloor = currentFloor != BottomFloor ? currentFloor : TopFloor;
  }

  FloorNumber nextStop = Search(searchStartFloor, currentDirection);

  // One more search attempt for every direction, changing the search start floor:
  // top floor for down and bottom floor for up
  for (auto attempt = 0U; attempt < 2U; ++attempt) 
  {
    if (!IsValid(nextStop))
    {
      // No stops found in current direction, search stops in the other direction
      currentDirection = (currentDirection == Direction::Up ? Direction::Down : Direction::Up);
      searchStartFloor = (currentDirection == Direction::Up ? BottomFloor : TopFloor);

      nextStop = Search(searchStartFloor, currentDirection);
    }
  }

  m_log.Trace(std::string("GetNextStop ") + std::to_string(currentFloor), Log::TraceLevel::Debug);
  
  return nextStop;
}

Floors::FloorNumber Floors::Search(const FloorNumber startFloor, const Direction direction)
{
  if (!IsValid(startFloor) || direction != Direction::Up && direction != Direction::Down)
    return InvalidFloor;

  if(direction == Direction::Up)
  {
  for (FloorNumber floor = startFloor; IsValid(floor); ++floor)
    if (m_stops[floor].first && m_stops[floor].second == Direction::Up || m_stops[floor].second == Direction::Both)
      return floor;
  }
  else if (direction == Direction::Down)
  {
    for (FloorNumber floor = startFloor; IsValid(floor); --floor)
      if (m_stops[floor].first && m_stops[floor].second == Direction::Down || m_stops[floor].second == Direction::Both)
        return floor;
  }

  return InvalidFloor;
}

void Floors::ClearStop(const FloorNumber floor, const Direction direction)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!IsValid(floor))
    return;

  if (m_stops[floor].second == Direction::Up || m_stops[floor].second == Direction::Down)
  {
    m_stops[floor].first = false;
    m_stops[floor].second = Direction::None;

    m_log.Trace("Cleared floor " + std::to_string(floor), Log::TraceLevel::Debug);
  }
  else if (m_stops[floor].second == Direction::Both)
  {
    m_stops[floor].second = direction == Direction::Up ? Direction::Down : Direction::Up;

    std::stringstream message;
    message << "Cleared floor " << floor << "direction " << (direction == Direction::Up ? "UP" : "DOWN");
    m_log.Trace(message, Log::TraceLevel::Debug);
  }
}

void Floors::Trace(const FloorNumber currentFloor)
{
  std::stringstream message;
  message << "Floors stops: ";

  for (FloorNumber floor = BottomFloor; IsValid(floor); ++floor)
  {
    std::string direction;

    switch (m_stops[floor].second)
    {
    case Direction::Up: 
      direction = "U";
      break;
    case Direction::Down: 
      direction = "D";
      break;
    case Direction::Both: 
      direction = "B";
      break;

    default:
    case Direction::None:
      direction = "None";
      break;
    }

    const auto currentFloorIndicator = IsValid(currentFloor) && currentFloor == floor ? "*" : "";

    if (m_stops[floor].first)
      message << "[" << currentFloorIndicator << std::to_string(floor) << direction << "]";
  }

  m_log.Trace(message, Log::TraceLevel::Verbose);
}
