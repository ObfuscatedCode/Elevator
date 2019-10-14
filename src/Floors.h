#pragma once

#include "Configuration.h"
#include "Log.h"

#include <vector>
#include <mutex>

enum class Direction
{
  None = -1,
  Up,
  Down,
  Both,
};

typedef std::pair<bool, Direction> Stop;
typedef std::vector<Stop> FloorStops;

class Floors final
{
public: // Types, constants and static functions
    typedef unsigned int FloorNumber;

    static constexpr FloorNumber TotalFloors = Configuration::Building::NumberOfFloors;
    static constexpr FloorNumber BottomFloor = 0U;
    static constexpr FloorNumber TopFloor = (TotalFloors - 1);
    static constexpr FloorNumber InvalidFloor = static_cast<FloorNumber>(-1);

    static bool IsValid(const FloorNumber floorNumber) { return floorNumber >= BottomFloor && floorNumber <= TopFloor; }

public:
  Floors();

public:
  bool SetStop(const class std::shared_ptr<class Call>& call, bool destinationOnly = false);
  void ClearStop(const FloorNumber floor, const Direction direction);

  FloorNumber GetNextStop(const FloorNumber currentFloor, Direction& currentDirection);

  static class People& GetPeople() { return m_people; }

  void Trace(const FloorNumber currentFloor);

  void SetId(const std::string& id) { m_log.SetTraceId(id); }
  std::string GetId() const { return m_log.GetTraceId(); }

private:
  FloorNumber Search(const FloorNumber startFloor, const Direction direction);

private:
  static class People m_people;
  FloorStops m_stops;

  std::mutex m_mutex;

  Log m_log;
};

