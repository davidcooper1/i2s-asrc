#include <stddef.h>
#include <atomic>
#include <optional>

template <typename T, size_t Capacity>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two.");

    private:
        alignas(64) std::atomic<size_t> head{0};
        alignas(64) std::atomic<size_t> tail{0};
        alignas(64) T buffer[Capacity];

        size_t cached_head{0};
        size_t cached_tail{0};

        size_t mask(size_t index) const {
            return index & (Capacity - 1);
        }

    public:
        void push(T item);
        std::optional<T> pop();
};