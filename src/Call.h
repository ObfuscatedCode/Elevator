#pragma once

#include "Floors.h"

#include <utility>
#include <string>

class Call final 
{
public:
  explicit Call(const Floors::FloorNumber startFloor = 0, const Floors::FloorNumber destinationFloor = 0) :
    m_startFloor(startFloor), m_destinationFloor(destinationFloor)
  {
  }

  ~Call() = default;

  Call(const Call&) = delete;
  Call& operator=(const Call&) = delete;

  Call(Call&&) = default;
  Call& operator=(Call&&) = default;

public:
  Floors::FloorNumber GetStartFloor() const { return m_startFloor; }
  Floors::FloorNumber GetDestinationFloor() const { return m_destinationFloor; }

  void SetStartFloor(const Floors::FloorNumber startFloor) { m_startFloor = startFloor; }
  void SetDestinationFloor(const Floors::FloorNumber destinationFloor) { m_destinationFloor = destinationFloor; }

  std::string GetAssignedElevator() const { return m_assignedElevator; }
  void SetAssignedElevator(std::string assignedElevator) { m_assignedElevator = std::move(assignedElevator); }

  Direction GetDirection() const { return m_startFloor < m_destinationFloor ? Direction::Up : Direction::Down; }

  bool IsValid() const
    { return Floors::IsValid(m_startFloor) && Floors::IsValid(m_destinationFloor) && m_startFloor != m_destinationFloor; }

  std::string ToString() const 
    { return "[" + m_assignedElevator + " " + std::to_string(m_startFloor) + ", " + std::to_string(m_destinationFloor) + "]"; }

private:
  Floors::FloorNumber m_startFloor = 0;
  Floors::FloorNumber m_destinationFloor = 0;

  std::string m_assignedElevator = "?";
};

