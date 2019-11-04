/**********************************************************************************
*        File: Elevator.h
* Description: Implements the elevator logic.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

#include <mutex>
#include <chrono>
#include <string>
#include <condition_variable>

#include "Floors.h"
#include "People.h"
#include "WorkerThread.h"

using namespace std::chrono_literals;

enum class ElevatorStatus
{
  OutOfOrder = -1,
  Idle = 0,
  PeopleEnterAndExit,
  Moving,
};

enum class DoorsStatus
{
  Open,
  Closed
};


class Elevator final 
{
  class ElevatorThread final : public WorkerThread<Elevator>
  {
  public:
    explicit ElevatorThread(Elevator* elevator = nullptr) : WorkerThread<Elevator>(elevator) { }

  protected:
    void CycleFunction(Elevator* elevator) override;
  };

public:
  explicit Elevator(const std::string& id = "");

  Elevator(const Elevator&) = delete;
  Elevator(Elevator&&) = delete;

  ~Elevator();

  Elevator& operator=(const Elevator&) = delete;
  Elevator& operator=(Elevator&& other) noexcept = delete;
  
public:
  bool Available(const std::shared_ptr<Call>& call) const;
  bool AnswerToCall(const std::shared_ptr<Call>& call);

  void ShutDown();

  void SetId(std::string id);
  std::string GetId() const { return m_elevatorId; }

  std::string GetElevatorName() const { return m_name; }

  Floors::FloorNumber GetCurrentFloor() const { return m_currentFloor; }
  ElevatorStatus GetStatus() const { return m_status; }
  Direction GetDirection() const { return m_currentDirection; }

private:
  bool OpenDoors();
  bool CloseDoors();

  void PeopleEnterAndExit();
  void Move(Floors::FloorNumber requestedFloor);
  void Stop();

  void RestoreDestinationStops();

private:
  Floors::FloorNumber m_currentFloor = 0;
  Floors m_floors;

  People m_people;

  ElevatorStatus m_status = ElevatorStatus::Idle;
  Direction m_currentDirection = Direction::None;

  DoorsStatus m_doorsStatus = DoorsStatus::Closed;

  std::condition_variable m_go;
  std::mutex m_goMutex;

  std::string m_elevatorId = "?";
  std::string m_name;

  std::unique_ptr<ElevatorThread> m_thread;

  Log m_log;
};

