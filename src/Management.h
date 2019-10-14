#pragma once

#include "Log.h"

#include <vector>
#include <memory>

class Management final 
{
public:
  explicit Management(const unsigned int numberOfElevators);

  Management(const Management&) = delete;
  Management(Management&&) = delete;

  ~Management();
  
  Management& operator=(const Management&) = delete;
  Management& operator=(Management&&) = delete;

public:
  bool AssignCall(class std::shared_ptr<class Call>& call);

  void Shutdown();

private:
  std::vector<std::unique_ptr<class Elevator>> m_elevators;

  Log m_log;
};
