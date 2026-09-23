#include <ring-buffer.hpp>
#include <optional>

template <typename T, size_t Capacity>
void RingBuffer<T, Capacity>::push(T item) {
    size_t current_tail = tail.load(std::memory_order_relaxed);

    if (current_tail - cached_head == Capacity) {
        cached_head = head.load(std::memory_order_acquire);

        if (current_tail - cached_head == Capacity) {
            head.store(cached_head + 1, std::memory_order_release);
            cached_head++;
        }
    }

    buffer[mask(current_tail)] = std::move(item);

    tail.store(current_tail + 1, std::memory_order_release);
}

template <typename T, size_t Capacity>
std::optional<T> RingBuffer<T, Capacity>::pop() {
    size_t current_head = head.load(std::memory_order_relaxed);

    if (current_head == cached_tail) {
        cached_tail = tail.load(std::memory_order_acquire);

        if (current_head == cached_tail) {
            return std::nullopt;
        }
    }

    T item = std::move(buffer[mask(current_head)]);

    head.store(current_head + 1, std::memory_order_release);

    return item;
}

template <typename T, size_t Capacity>
void RingBuffer<T, Capacity>::clear() {
    cached_tail = tail.load(std::memory_order_acquire);
    head.store(cached_tail, std::memory_order_release);
}