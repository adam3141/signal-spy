#pragma once

#include "ui/ui_types.hpp"

#include <cstdint>
#include <deque>
#include <mutex>
#include <variant>

namespace spy::signal
{

struct SetCenterFrequencyCommand
{
    double center_frequency_hz{0.0};
};

struct SetSampleRateCommand
{
    double sample_rate_hz{0.0};
};

struct SetGainCommand
{
    double gain_db{0.0};
};

struct SetFFTSizeCommand
{
    uint32_t fft_size{1024};
};

struct SetWindowFunctionCommand
{
    spy::ui::WindowFunction window_function{spy::ui::WindowFunction::Hann};
};

using CommandVariant = std::variant<SetCenterFrequencyCommand, SetSampleRateCommand, SetGainCommand, SetFFTSizeCommand,
                                    SetWindowFunctionCommand>;

/**
 * @brief Thread-safe Multi-Producer Single-Consumer (MPSC) Command Queue.
 *
 * Transmits non-blocking parameter mutation instructions from UI control widgets to the DSP worker engine.
 */
class CommandQueue
{
  public:
    CommandQueue() = default;
    ~CommandQueue() = default;

    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;

    CommandQueue(CommandQueue&&) = delete;
    CommandQueue& operator=(CommandQueue&&) = delete;

    /**
     * @brief Enqueues a command (Thread-safe).
     * @param command Command variant to enqueue.
     */
    void Push(CommandVariant command)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push_back(std::move(command));
    }

    /**
     * @brief Dequeues a command if available (Thread-safe).
     * @param command Output parameter where popped command will be stored.
     * @return True if a command was popped, false if queue was empty.
     */
    bool Pop(CommandVariant& command)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty())
        {
            return false;
        }

        command = std::move(_queue.front());
        _queue.pop_front();
        return true;
    }

    /**
     * @brief Returns true if the queue is empty.
     */
    [[nodiscard]] bool IsEmpty() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    /**
     * @brief Returns current number of pending commands.
     */
    [[nodiscard]] std::size_t Size() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }

  private:
    mutable std::mutex _mutex;
    std::deque<CommandVariant> _queue;
};

} // namespace spy::signal
