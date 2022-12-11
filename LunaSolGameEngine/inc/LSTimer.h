#pragma once
#include "LSEFramework.h"

using namespace std::chrono;
using namespace std::chrono_literals;

namespace LS
{
	template<class Rep, int64_t Numerator, int64_t Denominator>
	class LSTimer
	{
		using Ratio = std::ratio<Numerator, Denominator>;
		using TimerDuration = duration<Rep, Ratio>;
		using SteadyPoint = steady_clock::time_point;
	public:
		void Start()
		{
			if (!m_bIsStopped)
				return;
			m_bIsStopped = false;
			m_startTP = steady_clock::now();
		}

		void Tick()
		{
			if (m_bIsStopped)
				return;
			m_currentTP = steady_clock::now();
			m_deltaTime = duration_cast<TimerDuration>(m_currentTP - m_previousTP);
			m_previousTP = m_currentTP;
		}

		void Reset()
		{
			m_bIsStopped = false;
			m_currentTP = steady_clock::now();
			m_previousTP = m_currentTP;
			m_startTP = m_currentTP;
			m_deltaTime = duration_cast<TimerDuration>(0s);
			m_totalTime = duration_cast<TimerDuration>(0s);
		}

		void Stop()
		{
			if (m_bIsStopped)
				return;
			m_stopTP = steady_clock::now();
			m_totalTime = m_stopTP - m_startTP;
			m_bIsStopped = true;
		}

		/**
		 * @brief Returns the total time that has passed since this timer had been started.
		*/
		[[nodiscard]] TimerDuration GetTotalTime() const
		{
			return duration_cast<TimerDuration>(steady_clock::now() - m_startTP);
		}

		template<class DurationCast>
		[[nodiscard]] DurationCast GetTotalTimeIn() const
		{
			return duration_cast<DurationCast>(steady_clock::now() - m_startTP);
		}

		/**
		 * @brief Returns the total time that has accumulated through ticks.
		 *
		 * @link Timer::Tick() must be called frequently to get an accurate time. This
		 * usually is called during every "frame" or "update" cycle, and represents the total
		 * amount of time interval in fixed steps. @link Timer::GetTotalTime() returns the current value
		 * of passed time at the point of the call, which means every call is a new time value, and
		 * can be inconsistent for aligning timing requirements.
		 * @return
		*/
		[[nodiscard]] TimerDuration GetTotalTimeTicked() const
		{
			return duration_cast<TimerDuration>(m_currentTP - m_startTP);
		}

		template<class DurationCast>
		[[nodiscard]] DurationCast GetTotalTimeTickedIn() const
		{
			return duration_cast<DurationCast>(m_currentTP - m_startTP);
		}

		[[nodiscard]] TimerDuration GetTotalTimeSinceStopped() const
		{
			return m_stopTP - m_startTP;
		}

		/**
		 * @brief The time between ticks
		 * @return The time duration between ticks
		*/
		[[nodiscard]] TimerDuration GetDeltaTime() const
		{
			return m_deltaTime;
		}

		/**
		 * @brief Returns Delta Time as another format representation (i.e. current Rep = int64_t, request to
		 * return current rep as float)
		 * @tparam R The representation value you wish to duration cast to
		 * @return The duration cast time from current Rep to another rep.
		*/
		template<class R>
		R DeltaTimeAs() const
		{
			return duration_cast<duration<R, Ratio>>(m_deltaTime).count();
		}

		/**
		 * @brief Returns the value of DeltaTime in a duration specified. For example if the timer
		 * is set to represent 1000ms per tick, you could return the time as seconds by
		 * supplying std::chrono::seconds.
		 * @tparam D The duration type to return the time as
		 * @return the delta time as a duration casted into another time
		*/
		template<class D>
		D DeltaTimeIn() const
		{
			return duration_cast<D>(m_deltaTime);
		}


	private:
		SteadyPoint m_startTP;// Time this timer was started
		SteadyPoint m_currentTP;// The current Time Point pointed by this timer (used to make time calculations)
		SteadyPoint m_previousTP;// The previous Time Point of this timer
		SteadyPoint m_stopTP;// The time this timer was stopped

		TimerDuration m_deltaTime;// The time between ticks
		TimerDuration m_totalTime;// The time between ticks
		bool m_bIsStopped = true;
	};

	using LSTimerLLMS = LSTimer<uint64_t, 1, 10000>;
}