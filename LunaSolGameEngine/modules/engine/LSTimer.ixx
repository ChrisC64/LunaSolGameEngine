module;
#include "LSEFramework.h"

export module Engine.LSTimer;

using namespace std::chrono;
using namespace std::chrono_literals;

export namespace LS
{
	template<class Rep, int64_t Numerator, int64_t Denominator>
	class Timer
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
			m_deltaTime = std::chrono::duration_cast<TimerDuration>(m_currentTP - m_previousTP);
			m_previousTP = m_currentTP;
		}

		void Reset()
		{
			m_bIsStopped = false;
			m_currentTP = steady_clock::now();
			m_previousTP = m_currentTP;
			m_startTP = m_currentTP;
			m_deltaTime = std::chrono::duration_cast<TimerDuration>(0s);
		}

		void Stop()
		{
			if (m_bIsStopped)
				return;
			m_stopTP = steady_clock::now();
			m_bIsStopped = true;
		}

		/**
		 * @brief Returns the total time that has passed since this timer had been started.
		*/
		[[nodiscard]] TimerDuration GetTotalTime() const
		{
			return std::chrono::duration_cast<TimerDuration>(steady_clock::now() - m_startTP);
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
			return std::chrono::duration_cast<std::chrono::duration<R, Ratio>>(m_deltaTime).count();
		}

	private:
		SteadyPoint m_startTP;// Time this timer was started
		SteadyPoint m_currentTP;// The current Time Point pointed by this timer (used to make time calculations)
		SteadyPoint m_previousTP;// The previous Time Point of this timer
		SteadyPoint m_stopTP;// The time this timer was stopped

		TimerDuration m_deltaTime;// The time between ticks
		bool m_bIsStopped = true;
	};
}