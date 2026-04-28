//==============================================================================
/*
	The main structure for Crystal Synthesizer.
*/
//==============================================================================
#pragma once
#include <atomic>
#include <chrono>
#include <thread>
#include <JuceHeader.h>
//==============================================================================
namespace Crystal
{
	using timer_callback_func = std::function<void(GLdouble)>;
	enum class Resource_State
	{
		Invalid = 0,
		Null,
		UnInitialised,
		Updating,
		PendingUpdate,
		Ready
	};
	enum class Logic_State
	{
		Invalid = 0,
		PendingRemoval,
		Null,
		UnInitialised,
		Ready,
		OnProcessing
	};
	/* States that are shared by all Entity instances */
	struct Entity_State
	{
		Resource_State	resource_state{ Resource_State::Null };
		Logic_State		logic_state{ Logic_State::Null };
		bool operator==(const Entity_State& other) const noexcept
		{
			return	resource_state == other.resource_state && logic_state == other.logic_state;
		}
		bool operator>=(const Entity_State& other) const noexcept
		{
			if (resource_state != other.resource_state)
				return static_cast<int>(resource_state) > static_cast<int>(other.resource_state);
			return static_cast<int>(logic_state) >= static_cast<int>(other.logic_state);
		}
		bool operator>(const Entity_State& other) const noexcept
		{
			if (resource_state != other.resource_state)
				return static_cast<int>(resource_state) > static_cast<int>(other.resource_state);
			return static_cast<int>(logic_state) > static_cast<int>(other.logic_state);
		}
		bool operator<(const Entity_State& other) const noexcept
		{
			if (resource_state != other.resource_state)
				return static_cast<int>(resource_state) < static_cast<int>(other.resource_state);
			return static_cast<int>(logic_state) < static_cast<int>(other.logic_state);
		}
		bool operator<=(const Entity_State& other) const noexcept
		{
			if (resource_state != other.resource_state)
				return static_cast<int>(resource_state) < static_cast<int>(other.resource_state);
			return static_cast<int>(logic_state) <= static_cast<int>(other.logic_state);
		}
	};
}
namespace Crystal
{
	static constexpr Entity_State Entity_Constructed{ Resource_State::UnInitialised,	Logic_State::UnInitialised };
	static constexpr Entity_State Entity_Initialising{ Resource_State::Updating,		Logic_State::UnInitialised };
	static constexpr Entity_State Entity_Initialised{ Resource_State::Ready,			Logic_State::Ready };
	static constexpr Entity_State Entity_OnProcessing{ Resource_State::Ready,			Logic_State::OnProcessing };
	static constexpr Entity_State Entity_Deconstructing{ Resource_State::Invalid,		Logic_State::PendingRemoval };
	static constexpr Entity_State Entity_Deconstructed{ Resource_State::Invalid,		Logic_State::Invalid };
}
namespace Crystal
{
	//==============================================================================
	/* Glaobal unique Timer */
	class Timer final
	{
	public:
		//==============================================================================
		static Timer& getTimer()
		{
			static Timer instance;
			return instance;
		}
		void setDT(GLdouble dt) noexcept
		{
			global_dt.store(dt, std::memory_order_release);
		}
		[[nodiscard]] GLdouble getDT() const noexcept
		{
			return global_dt.load(std::memory_order_relaxed);
		}
		[[nodiscard]] GLdouble getSystemTime() const noexcept
		{
			return system_time.load(std::memory_order_relaxed);
		}
		void registerDirectTimerCallback(timer_callback_func func)
		{
			const juce::ScopedLock open(callback_lock);
			callbacks = std::move(func);
		}
	private:
		//==============================================================================
		Timer()
		{
			timer_thread = std::jthread(
				[this](std::stop_token st) { startTimer(st); });
		}
		Timer(const Timer&) = delete;
		Timer& operator=(const Timer&) = delete;
	private:
		//==============================================================================
		/* Global delta time and global accumulator in seconds */
		std::atomic<GLdouble> global_dt{ 1e-3 }, accumulator{ 0 }, system_time{ 0 };
		std::jthread timer_thread;
		timer_callback_func callbacks;
		juce::CriticalSection callback_lock;
	private:
		//==============================================================================
		void timerCallback() noexcept {};
		void startTimer(std::stop_token stop_token) noexcept
		{
			auto last_time_point = std::chrono::steady_clock::now();
			while (!stop_token.stop_requested())
			{
				auto current_time_point = std::chrono::steady_clock::now();
				auto duration_in_sec = std::chrono::duration<GLdouble>(current_time_point - last_time_point);
				last_time_point = current_time_point;

				accumulator += duration_in_sec.count();
				while (accumulator.load() >= global_dt.load(std::memory_order_acquire))
				{
					timerCallback();
					accumulator -= global_dt;
				}
				std::this_thread::yield();
			}
		}
		void registerEventTimerCallback(timer_callback_func func)
		{

		}
	};
	//==============================================================================
	/* Pure virtual base class for all Entities */
	class Entity
	{
	public:
		//==============================================================================
		Entity() = default;
		virtual ~Entity() = default;
	public:
		//==============================================================================
		const Entity_State getState() const noexcept 
		{ 
			const auto state = entity_state.load(std::memory_order_relaxed);
			std::atomic_thread_fence(std::memory_order_acquire);
			return state;
		}
		void debug_forceSetState(const Entity_State& next_state)
		{
			setState(next_state);
		}
	protected:
		void setState(const Entity_State& next_state) noexcept
		{
			/* Wait until all cache write get handled, then setting state */
			std::atomic_thread_fence(std::memory_order_release);
			const auto state = entity_state.load(std::memory_order_relaxed);
			entity_state.store(next_state, std::memory_order_relaxed);
		}
	private:
		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Entity);
		std::atomic<Entity_State> entity_state;
	};
	//==============================================================================
	/* Pure virtual base class for all Processors */
	class Processor
	{
	public:
		//==============================================================================
		Processor(Timer& t = Timer::getTimer()) : timer(t) {}
		virtual ~Processor() = default;
	public:
		//==============================================================================
		virtual bool prepare() = 0;
		virtual void processing() = 0;
		virtual void synchronize() = 0;
		virtual void shutdown() = 0;
	protected:
		Timer& timer;
	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor);
	};
	//==============================================================================
}
namespace Crystal
{
	namespace OpenGL
	{
		struct Vertex_Attrib
		{
			GLuint		location;
			GLint		size;
			GLenum		type;
			GLboolean	normalized;
			GLsizei		stride;
			size_t		offset;
		};
	}
}