#pragma once
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <optional>
#include <utility>

// C++23 lockfree::MPSC<T, BufSize>
// ————————————————————————————————————————————————————————
// Bounded multiple‐producer / single‐consumer ring buffer.
// Uses a per‐slot “sequence” counter (Dmitry Vyukov’s algorithm), but
// optimizes for performance by (1) aligning each slot to 64 bytes (cache line)
// and (2) using C++20’s atomic.wait/notify_one to avoid pure spin‐loops.
//
// Requirements on BufSize: BufSize >= 2, power of two.
// ————————————————————————————————————————————————————————
namespace lockfree {

template <typename T, size_t BufSize>
    requires(BufSize >= 2) && ((BufSize & (BufSize - 1)) == 0)
class MPSC {
    static constexpr std::size_t MASK = BufSize - 1;

    // Each slot is cache‐line aligned so that seq/data for slot i does not
    // share a cache line with any other slot j≠i.  This avoids false sharing
    // when multiple producers spin on different slots.
    struct alignas(64) Slot {
        std::atomic<std::size_t> seq;
        T data;
        // No need for extra padding: alignas(64) already forces each Slot to
        // occupy a multiple of 64 bytes.
    };

    // head_ is the “ticket dispenser” for producers.
    // tail_ is the single‐consumer’s read index.
    alignas(64) std::atomic<std::size_t> _head{0};
    alignas(64) std::atomic<std::size_t> _tail{0};

    // The ring of slots
    std::array<Slot, BufSize> _buffer{};

   public:
    MPSC() noexcept {
        // At construction, initialize each slot’s seq = its index:
        //   That means “slot i is free, next producer to use it will have ticket i.”
        for (std::size_t i = 0; i < BufSize; ++i) {
            _buffer[i].seq.store(i, std::memory_order_relaxed);
        }
    }

    ~MPSC() noexcept = default;

    // no copy / move
    MPSC(const MPSC&) = delete;
    MPSC& operator=(const MPSC&) = delete;
    MPSC(MPSC&&) = delete;
    MPSC& operator=(MPSC&&) = delete;

    // ———————————————————————————————————————————
    //  push(): multiple producers may call concurrently.
    //  Returns false immediately if the queue is (definitely) full.
    //
    //  Steps:
    //    1) Quick “full” check: load head (relaxed) and tail (acquire).
    //       If head – tail >= BufSize ⇒ full ⇒ return false.
    //    2) pos = head_.fetch_add(1, relaxed);  // grab a unique ticket
    //    3) Slot& slot = buffer_[pos & MASK];
    //    4) Wait until slot.seq == pos (Acquire).  If not equal, do:
    //          old = slot.seq.load(Acquire);
    //          slot.seq.wait(old, Relaxed);        // C++20 atomic.wait
    //       …and retry load(Acquire) until it becomes pos.
    //    5) Construct/move‐store the data into slot.data (non‐atomic).
    //    6) Publish by slot.seq.store(pos+1, Release).
    //    7) slot.seq.notify_one() is *not needed* here because only the consumer
    //       ever waits for “pos+1” on the other side.  (If we were waking a “reader,”
    //       we’d notify here.  Instead, the consumer simply polls or expires.)
    //
    template <typename U>
        requires std::constructible_from<T, U&&>
    bool push(U&& u) noexcept {
        // 1) Fast full‐check
        std::size_t cur_head = _head.load(std::memory_order_relaxed);
        std::size_t cur_tail = _tail.load(std::memory_order_acquire);
        // If the queue is full, we can return false immediately.
        if (cur_head - cur_tail >= BufSize) return false;

        // 2) Grab a ticket
        std::size_t pos = _head.fetch_add(1, std::memory_order_relaxed);
        Slot& slot = _buffer[pos & MASK];

        // 3) Wait until slot.seq == pos
        std::size_t seq_val = slot.seq.load(std::memory_order_acquire);
        if (seq_val != pos) {
            // Slow‐path: park ourselves until seq changes, then re‐check.
            do {
                // We capture the current value; wait until it changes:
                slot.seq.wait(seq_val, std::memory_order_relaxed);
                seq_val = slot.seq.load(std::memory_order_acquire);
            } while (seq_val != pos);
        }

        // 4) Now we own that slot—write the data
        slot.data = std::forward<U>(u);

        // 5) Publish it by bumping seq → pos+1
        slot.seq.store(pos + 1, std::memory_order_release);

        // (No notify_one needed here; the consumer will poll or wait on seq==pos+1.)

        return true;
    }

    // ———————————————————————————————————————————
    //  pop(): single consumer only.  If empty, returns std::nullopt.
    //
    //  Steps:
    //    1) pos = tail_.load(relaxed)
    //    2) Slot& slot = buffer_[pos & MASK];
    //    3) seq_val = slot.seq.load(Acquire)
    //       If seq_val != pos+1, that means “no data ready” ⇒ empty ⇒ return nullopt.
    //    4) We see seq == pos+1 ⇒ the producer fully wrote data & did Release → pos+1.
    //    5) Take T value = std::move(slot.data);
    //    6) Mark slot free for the next wrap: slot.seq.store(pos + BufSize, Release);
    //    7) slot.seq.notify_one();  // wake exactly one producer waiting for this slot
    //    8) tail_.store(pos+1, relaxed);
    //    9) return value.
    //
    std::optional<T> pop() noexcept {
        std::size_t pos = _tail.load(std::memory_order_relaxed);
        Slot& slot = _buffer[pos & MASK];

        std::size_t seq_val = slot.seq.load(std::memory_order_acquire);
        if (seq_val != pos + 1) {
            // either queue empty or producer hasn’t yet published this slot
            return std::nullopt;
        }

        // 5) Read out the data (the Release→Acquire pairing on seq
        //    guarantees `slot.data` is fully written).
        T value = std::move(slot.data);

        // 6) “Free” this slot by setting seq → pos + BufSize (Release).
        //    That number (pos + BufSize) is exactly what the next‐round
        //    producer will be waiting for.
        slot.seq.store(pos + BufSize, std::memory_order_release);

        // 7) Wake exactly one producer that might be parked on this slot:
        slot.seq.notify_one();

        // 8) Advance our tail index
        _tail.store(pos + 1, std::memory_order_relaxed);

        return value;
    }
};

}  // namespace lockfree
