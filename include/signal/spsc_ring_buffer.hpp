#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace spy::signal
{

/**
 * @brief Lock-Free Single-Producer Single-Consumer (SPSC) Ring Buffer.
 *
 * Provides high-throughput, zero-allocation data transfers between a single producer thread
 * (e.g., DSP ingestion thread) and a single consumer thread (e.g., UI rendering loop).
 * Uses cache line padding (64 bytes) to prevent false sharing between CPU cores.
 *
 * @tparam T Element type stored in the buffer.
 * @tparam Capacity Maximum capacity of the ring buffer (must be > 0).
 */
template <typename T, std::size_t Capacity> class SPSCRingBuffer
{
    static_assert(Capacity > 0, "Capacity must be greater than zero");

  public:
    SPSCRingBuffer() noexcept
        : _head(0)
        , _tail(0)
    {
    }

    ~SPSCRingBuffer() = default;

    SPSCRingBuffer(const SPSCRingBuffer&) = delete;
    SPSCRingBuffer& operator=(const SPSCRingBuffer&) = delete;

    SPSCRingBuffer(SPSCRingBuffer&&) = delete;
    SPSCRingBuffer& operator=(SPSCRingBuffer&&) = delete;

    /**
     * @brief Pushes an element into the buffer (Producer only).
     * @param value Element to push (copied).
     * @return True if successfully pushed, false if buffer is full.
     */
    bool push(const T& value)
    {
        const auto current_head = _head.load(std::memory_order_relaxed);
        const auto current_tail = _tail.load(std::memory_order_acquire);

        if (current_head - current_tail >= Capacity)
        {
            return false; // Buffer full
        }

        _buffer[current_head % Capacity] = value;
        _head.store(current_head + 1, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pushes an element into the buffer by moving (Producer only).
     * @param value Element to push (moved).
     * @return True if successfully pushed, false if buffer is full.
     */
    bool push(T&& value)
    {
        const auto current_head = _head.load(std::memory_order_relaxed);
        const auto current_tail = _tail.load(std::memory_order_acquire);

        if (current_head - current_tail >= Capacity)
        {
            return false; // Buffer full
        }

        _buffer[current_head % Capacity] = std::move(value);
        _head.store(current_head + 1, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pops an element from the buffer (Consumer only).
     * @param value Reference where popped element will be stored.
     * @return True if successfully popped, false if buffer is empty.
     */
    bool pop(T& value)
    {
        const auto current_tail = _tail.load(std::memory_order_relaxed);
        const auto current_head = _head.load(std::memory_order_acquire);

        if (current_tail == current_head)
        {
            return false; // Buffer empty
        }

        value = std::move(_buffer[current_tail % Capacity]);
        _tail.store(current_tail + 1, std::memory_order_release);
        return true;
    }

    /**
     * @brief Returns the current number of elements in the buffer.
     */
    [[nodiscard]] std::size_t size() const noexcept
    {
        const auto head = _head.load(std::memory_order_relaxed);
        const auto tail = _tail.load(std::memory_order_relaxed);
        return (head >= tail) ? (head - tail) : 0;
    }

    /**
     * @brief Returns true if the buffer is empty.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    /**
     * @brief Returns maximum capacity of the buffer.
     */
    [[nodiscard]] static constexpr std::size_t capacity() noexcept
    {
        return Capacity;
    }

  private:
    alignas(64) std::atomic<std::size_t> _head{0};
    alignas(64) std::atomic<std::size_t> _tail{0};
    std::array<T, Capacity> _buffer{};
};

} // namespace spy::signal
