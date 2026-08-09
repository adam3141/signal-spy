#include "signal/command_queue.hpp"
#include "signal/spsc_ring_buffer.hpp"
#include "ui/ui_types.hpp"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

TEST(SPSCRingBufferTest, BasicPushPop)
{
    spy::signal::SPSCRingBuffer<int, 4> buffer;

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer.capacity(), 4);

    EXPECT_TRUE(buffer.push(10));
    EXPECT_TRUE(buffer.push(20));
    EXPECT_EQ(buffer.size(), 2);

    int val = 0;
    EXPECT_TRUE(buffer.pop(val));
    EXPECT_EQ(val, 10);

    EXPECT_TRUE(buffer.pop(val));
    EXPECT_EQ(val, 20);

    EXPECT_FALSE(buffer.pop(val));
    EXPECT_TRUE(buffer.empty());
}

TEST(SPSCRingBufferTest, FullCapacityOverflow)
{
    spy::signal::SPSCRingBuffer<int, 3> buffer;

    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));
    EXPECT_FALSE(buffer.push(4)); // Overflow

    EXPECT_EQ(buffer.size(), 3);

    int val = 0;
    EXPECT_TRUE(buffer.pop(val));
    EXPECT_EQ(val, 1);

    EXPECT_TRUE(buffer.push(4)); // Now space available
    EXPECT_EQ(buffer.size(), 3);
}

TEST(SPSCRingBufferTest, SpectrumFrameDTOPushing)
{
    spy::signal::SPSCRingBuffer<spy::ui::SpectrumFrameDTO, 16> buffer;

    spy::ui::SpectrumFrameDTO frame;
    frame.timestamp_ns = 123456789;
    frame.center_frequency_hz = 100.0e6;
    frame.sample_rate_hz = 2.0e6;
    frame.magnitudes_db = {-80.0f, -45.0f, -10.0f, -60.0f};

    EXPECT_TRUE(buffer.push(std::move(frame)));

    spy::ui::SpectrumFrameDTO popped_frame;
    EXPECT_TRUE(buffer.pop(popped_frame));

    EXPECT_EQ(popped_frame.timestamp_ns, 123456789);
    EXPECT_DOUBLE_EQ(popped_frame.center_frequency_hz, 100.0e6);
    EXPECT_DOUBLE_EQ(popped_frame.sample_rate_hz, 2.0e6);
    ASSERT_EQ(popped_frame.magnitudes_db.size(), 4);
    EXPECT_FLOAT_EQ(popped_frame.magnitudes_db[2], -10.0f);
}

TEST(SPSCRingBufferTest, MultithreadedProducerConsumer)
{
    constexpr std::size_t ITEM_COUNT = 50000;
    spy::signal::SPSCRingBuffer<std::size_t, 1024> buffer;

    std::atomic<bool> producer_done{false};
    std::size_t items_received = 0;
    std::size_t checksum = 0;

    std::thread producer([&]() {
        for (std::size_t i = 1; i <= ITEM_COUNT; ++i)
        {
            while (!buffer.push(i))
            {
                std::this_thread::yield();
            }
        }
        producer_done.store(true, std::memory_order_release);
    });

    std::thread consumer([&]() {
        std::size_t val = 0;
        while (!producer_done.load(std::memory_order_acquire) || !buffer.empty())
        {
            if (buffer.pop(val))
            {
                items_received++;
                checksum += val;
            }
            else
            {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(items_received, ITEM_COUNT);
    std::size_t expected_checksum = (ITEM_COUNT * (ITEM_COUNT + 1)) / 2;
    EXPECT_EQ(checksum, expected_checksum);
}

TEST(CommandQueueTest, EnqueueDequeueVariants)
{
    spy::signal::CommandQueue queue;
    EXPECT_TRUE(queue.empty());

    queue.push(spy::signal::SetCenterFrequencyCommand{101.5e6});
    queue.push(spy::signal::SetFFTSizeCommand{4096});
    EXPECT_EQ(queue.size(), 2);

    spy::signal::CommandVariant cmd;
    EXPECT_TRUE(queue.pop(cmd));
    ASSERT_TRUE(std::holds_alternative<spy::signal::SetCenterFrequencyCommand>(cmd));
    EXPECT_DOUBLE_EQ(std::get<spy::signal::SetCenterFrequencyCommand>(cmd).center_frequency_hz, 101.5e6);

    EXPECT_TRUE(queue.pop(cmd));
    ASSERT_TRUE(std::holds_alternative<spy::signal::SetFFTSizeCommand>(cmd));
    EXPECT_EQ(std::get<spy::signal::SetFFTSizeCommand>(cmd).fft_size, 4096);

    EXPECT_FALSE(queue.pop(cmd));
    EXPECT_TRUE(queue.empty());
}
