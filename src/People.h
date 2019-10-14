#pragma once

#include <list>

#include "Call.h"

class People final : std::list<std::shared_ptr<Call>> 
{
public:
  /*
  // To lock the list and avoid invalidation during external iterations
  class LockInScope 
  {
  public:
    LockInScope(People& people) : m_mutex(people.m_mutex) { m_mutex.lock(); }
    ~LockInScope() { m_mutex.unlock(); }

  private:
    std::mutex& m_mutex;
  };
  */

public:
  explicit People(const std::string& id = "") { SetId(id); }
  ~People() noexcept = default;

  People(const People&) = delete;
  People& operator=(const People&) = delete;

  People(const People&&) = delete;
  People& operator=(People&&) = delete;

public:
  iterator Insert(const std::shared_ptr<Call>& call);

  void EnterAndExit(
    People& waitingPeople, 
    const Floors::FloorNumber currentFloor, 
    const Direction currentDirection, 
    const std::string& elevatorId);

  void Trace(const Floors::FloorNumber currentFloor = Floors::InvalidFloor);

  bool Empty();

  void SetId(const std::string& id) { m_log.SetTraceId(id); }
  std::string GetId() const { return m_log.GetTraceId(); }

  // Functions for range based loops support, use LockInScope before start iteration to avoid iterator invalidation
  const std::list<std::shared_ptr<Call>>& GetList() const { return *this; }
  //auto begin() { return std::list<std::shared_ptr<Call>>::begin();  }
  //auto end() { return std::list<std::shared_ptr<Call>>::end(); }

private:
  void Enter(
    People& waitingPeople, 
    const Floors::FloorNumber currentFloor, 
    const Direction currentDirection, 
    const std::string& elevatorId);

  void Exit(const Floors::FloorNumber currentFloor);

private:
  std::mutex m_mutex;

  Log m_log;
};



