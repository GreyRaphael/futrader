// https://github.com/GreyRaphael/lockfree
#pragma once
#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <optional>
#include <utility>

namespace lockfree {

template <typename T, size_t BufSize>
    requires(BufSize >= 2) && ((BufSize & (BufSize - 1)) == 0)
class SPSC {
    static constexpr size_t MASK = BufSize - 1;

    // The raw ring buffer storage
    std::array<T, BufSize> _buffer{};

    // Align each atomic on its own cache line to avoid false sharing
    alignas(64) std::atomic<size_t> _write_pos{0};
    alignas(64) std::atomic<size_t> _read_pos{0};

   public:
    SPSC() noexcept = default;
    ~SPSC() noexcept = default;

    // no copy / move
    SPSC(const SPSC&) = delete;
    SPSC& operator=(const SPSC&) = delete;
    SPSC(SPSC&&) = delete;
    SPSC& operator=(SPSC&&) = delete;

    // —————————————————————————————————————————————————————————————
    //  push(): single producer
    //  Returns false if the queue is “full” at the time of call.
    //
    //  Memory‐ordering rationale:
    //    - We do read_pos.load(acquire) to pair with the consumer’s release
    //      of read_pos.  That guarantees we see an up-to-date “slots freed.”
    //    - We write into _buffer[...] first (plain store), then do write_pos.store(..., release).
    //    - The consumer does write_pos.load(acquire) before reading _buffer.  That sequencing
    //      guarantees the consumer never sees a “written index” until after the producer’s
    //      real data write into the buffer.
    //
    template <typename U>
        requires std::constructible_from<T, U&&>
    bool push(U&& u) noexcept {
        // 1) load local copies of the two indices
        size_t w = _write_pos.load(std::memory_order_relaxed);
        size_t r = _read_pos.load(std::memory_order_acquire);

        // 2) “full” check must be (w - r) >= BufSize, not w >= r + BufSize
        if ((w - r) >= BufSize) {
            return false;  // queue is full
        }

        // 3) write data into the ring slot
        _buffer[w & MASK] = std::forward<U>(u);

        // 4) publish the new write_pos (release)
        _write_pos.store(w + 1, std::memory_order_release);
        return true;
    }

    // —————————————————————————————————————————————————————————————
    //  pop(): single consumer
    //  Returns std::nullopt if the queue is empty.
    //
    //  Memory‐ordering rationale:
    //    - We do write_pos.load(acquire) so that we see the producer’s
    //      release store to write_pos—which in turn pairs with the producer’s
    //      actual write into _buffer.  That guarantees “slot is ready.”
    //    - We read _buffer[...] (after seeing write_pos ≥ read_pos + 1), so we know the data is valid.
    //    - Finally we do read_pos.store(relaxed or release).  Using release is fine;
    //      producers read read_pos with acquire.  (We could even do relaxed here if we only
    //      ever read read_pos with acquire, but release is more standard.)
    //
    std::optional<T> pop() noexcept {
        size_t r = _read_pos.load(std::memory_order_relaxed);
        size_t w = _write_pos.load(std::memory_order_acquire);

        // “empty” check must be (w - r) == 0, not (r >= w)
        if ((w - r) == 0) {
            return std::nullopt;  // queue is empty
        }

        // 1) pull the item out
        T value = std::move(_buffer[r & MASK]);

        // 2) advance read_pos (release)
        _read_pos.store(r + 1, std::memory_order_release);

        return value;
    }

    // —————————————————————————————————————————————————————————————
    //  pop(T& out): non-allocating pop
    //  Returns false if empty, true + writes into ‘out’ otherwise.
    bool pop(T& out) noexcept {
        size_t r = _read_pos.load(std::memory_order_relaxed);
        size_t w = _write_pos.load(std::memory_order_acquire);

        if ((w - r) == 0) {
            return false;  // queue empty
        }

        // move‐into user’s storage
        out = std::move(_buffer[r & MASK]);

        // advance read_pos (release)
        _read_pos.store(r + 1, std::memory_order_release);
        return true;
    }
};

}  // namespace lockfree
