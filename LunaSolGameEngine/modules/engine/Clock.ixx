export module Clock;
import <chrono>;
import <cstdint>;

using namespace std::chrono;
using namespace std::chrono_literals;

namespace LS
{
    template<class T>
    concept is_allowed_time_type = requires(T t)
    {
        std::is_same_v<decltype(t), float> ||
            std::is_same_v<decltype(t), double> ||
            std::is_same_v<decltype(t), uint64_t> ||
            std::is_same_v<decltype(t), int64_t> ||
            std::is_same_v<decltype(t), uint32_t> ||
            std::is_same_v<decltype(t), int32_t>;
    };
}

export namespace LS
{
    /**
     * @brief A simple timer class that counts down with high precision. Must be manually started and can only move forward.
     * Timer can be running or stopped. A manual reset is provided to start the timer over from 0.
     */
    class Clock
    {
        using Steady = std::chrono::steady_clock;
        using TimePoint = std::chrono::steady_clock::time_point;
        // Microseconds represented as UInt64 (1s = 1'000'000us)
        using Duration = std::chrono::duration<uint64_t, std::ratio<1, 1'000'000>>;

    public:
        Clock() = default;
        ~Clock() = default;

        void Tick()
        {
            if (!m_isRunning)
            {
                return;
            }
            m_currentPoint = Steady::now();
            m_deltaTime = std::chrono::duration_cast<Duration>(m_currentPoint - m_prevPoint);
            m_prevPoint = m_currentPoint;
        }

        void Start()
        {
            m_startPoint = Steady::now();
            m_currentPoint = m_startPoint;
            m_prevPoint = m_startPoint;
            m_isRunning = true;
        }

        /**
         * @brief Returns the total number of ticks that passed in nanoseconds
         * @return Number of accumulated ticks as nanoseconds
         */
        constexpr uint64_t GetTotalTimeUs() const
        {
            return (m_currentPoint - m_startPoint).count();
        }
        
        /**
         * @brief Returns the total number of ticks that passed in milliseconds
         * @return Number of accumulated ticks as milliseconds
         */
        constexpr uint64_t GetTotalTimeMs() const
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(m_currentPoint - m_startPoint).count();
        }

        /**
         * @brief Returns the total number of ticks that passed in seconds
         * @return Number of accumulated ticks as seconds
         */
        constexpr uint64_t GetTotalTimeSecs() const
        {
            return std::chrono::duration_cast<std::chrono::seconds>(m_currentPoint - m_startPoint).count();
        }

        /**
         * @brief Returns the total time between ticks
         * @return Number of time between ticks as microseconds
         */
        constexpr uint64_t GetDeltaTimeUs() const
        {
            return m_deltaTime.count();
        }

        /**
         * @brief The total time passed since the start of the clock as milliseconds (ms). Does not require ticking
         * @return Time passed since the epoch (clock start)
         */
        constexpr uint64_t GetEpochMs() const
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(Steady::now() - m_startPoint).count();
        }

        /**
         * @brief Total time passed since the start of the clock in microseconds (us). Does not require ticking
         * @return Time passed since the epoch (clock start)
         */
        constexpr uint64_t GetEpochUs() const
        {
            return (Steady::now() - m_startPoint).count();
        }

        /**
         * @brief Returns the time between ticks as your desired format
         * @tparam Type The data type to return as (float, uint)
         * @tparam Ratio 
         * @return 
         */
        template<class Type, class Ratio> requires is_allowed_time_type<Type>
        constexpr Type DeltaTimeAs() const
        {
            return std::chrono::duration_cast<std::chrono::duration<Type, Ratio>>(m_deltaTime).count();
        }

        /**
         * @brief Helper function to return time as a specified duration
         * @tparam D the duration to cast to 
         * @return The internal microseconds timer cast to a specified duration
         */
        template<class D>
        constexpr uint64_t DeltaTimeIn() const
        {
            return std::chrono::duration_cast<D>(m_deltaTime).count();
        }

        /**
         * @brief Return the clock's total time as the specified type 
         * @tparam Type The specified type to convert to
         * @tparam Ratio The ratio of which to represent the timer as
         * @return Total time accumulated by each tick
         */
        template<class Type, class Ratio> requires is_allowed_time_type<Type>
        constexpr Type TotalTimeAs() const
        {
            return std::chrono::duration_cast<std::chrono::duration<Type, Ratio>>(m_currentPoint - m_startPoint).count();
        }

        /**
         * @brief Helper function to return total time accumulated by ticks 
         * @tparam D 
         * @return The conversion of the total time pass between ticks 
         */
        template<class D>
        constexpr uint64_t TotalTimeIn() const
        {
            return std::chrono::duration_cast<D>(m_currentPoint - m_startPoint).count();
        }

        /**
         * @brief Returns the epoch since clock start as specified duration and type
         * @tparam Type The type to return as
         * @tparam Ratio A measurement of the epoch to represent as
         * @return The type requested in the duration specified
         */
        template<class Type, class Ratio> requires is_allowed_time_type<Type>
        constexpr Type EpochAs() const
        {
            return std::chrono::duration_cast<std::chrono::duration<Type, Ratio>>(Steady::now() - m_startPoint).count();
        }

    private:
        bool m_isRunning = false;
        TimePoint m_startPoint;
        TimePoint m_prevPoint;
        TimePoint m_currentPoint;
        Duration m_deltaTime;
    };
}
