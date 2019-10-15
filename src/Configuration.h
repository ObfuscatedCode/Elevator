/**********************************************************************************
*        File: Configuration.h
* Description: Simulation parameters.
*      Author: Emanuele Merlo (emanuele.merlo@gmail.com)
*       Notes:
**********************************************************************************/

#pragma once

#include <chrono>
#include "ILog.h"

using namespace std::chrono_literals;

/**
 * \brief Simulation parameters.
 */
namespace Configuration
{
  namespace Building
  {
    /** 
     * \brief Number of elevators to simulate.
     */
    constexpr unsigned int NumberOfElevators = 1;

    /**
     * \brief Total number of floors.
     */
    constexpr unsigned int NumberOfFloors = 5;
  }

  namespace CallsGenerator
  {
    /**
     * \brief Types of calls generators.
     */
    enum class Type { Fixed, Random };

    /**
     * \brief Generator Type.
     */
    constexpr auto GeneratorType = Type::Random; // Type::Fixed;

    /**
     * \brief Assigned to NumberOfCalls specify that the generator must generate continuous calls.
     */
    constexpr unsigned int EndlessCalls = static_cast<unsigned int>(-1);

    /**
     * \brief [For random generator] Number of random calls to generate.
     */    
    constexpr unsigned int NumberOfCalls = 5; // EndlessCalls;

    /**
     * \brief Minimum delay (ms) between random calls.
     */
    constexpr auto MinDelayBetweenCalls = std::chrono::milliseconds(3s).count();

    /**
     * \brief Maximum delay (ms) between random calls.
     */
    constexpr auto MaxDelayBetweenCalls = std::chrono::milliseconds(10s).count();
  }

  namespace Elevator
  {
    /**
     * \brief Time to reach the next floor.
     */
    constexpr std::chrono::milliseconds TimeToReachTheNextFloor = 2s;

    /**
     * \brief Time for people enter and exit in the elevator
     */
    constexpr std::chrono::milliseconds EnterAndExitTime = 2s;
  }

  namespace Log
  {
    constexpr ILog::TraceLevel TraceLevel = ILog::TraceLevel::Verbose;

    constexpr ILog::LogType DefaultLogType = ILog::LogType::Screen;
  }

}