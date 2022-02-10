#include <atomic>

class dekkers_mutex
{
public:
    dekkers_mutex()
    {
        wants_to_enter[0].store(false, std::memory_order_relaxed);
        wants_to_enter[1].store(false, std::memory_order_relaxed);
        turn.store(0, std::memory_order_relaxed);
    }

    void lock(int tid)
    {
        int otherTid = (tid == 0) ? 1 : 0;
        // wants_to_enter[0] ← true
        wants_to_enter[tid].store(true, std::memory_order_relaxed);
        // while wants_to_enter[1] {
        while (wants_to_enter[otherTid].load(std::memory_order_relaxed))
        {
            // if turn ≠ 0 {
            if (turn.load(std::memory_order_relaxed) != tid)
            {
                // wants_to_enter[0] ← false
                wants_to_enter[tid].store(false, std::memory_order_relaxed);
                // while turn ≠ 0 {
                while (turn.load(std::memory_order_relaxed) != tid)
                    ; // busy wait
                // wants_to_enter[0] ← true
                wants_to_enter[tid].store(true, std::memory_order_relaxed);
            }
        }
    }

    void unlock(int tid)
    {
        int otherTid = (tid == 0) ? 1 : 0;
        turn.store(otherTid, std::memory_order_relaxed);
        wants_to_enter[tid].store(false, std::memory_order_relaxed);
    }

private:
    std::atomic_bool wants_to_enter[2];
    std::atomic_int turn;
};
