#include <atomic>

class dekkers_mutex
{
public:
    dekkers_mutex()
    {
        wants_to_enter[0].store(false);
        wants_to_enter[1].store(false);
        turn.store(0);
    }

    void lock(int tid)
    {
        int otherTid = (tid == 0) ? 1 : 0;
        // wants_to_enter[0] ← true
        wants_to_enter[tid].store(true);
        // while wants_to_enter[1] {
        while (wants_to_enter[otherTid].load())
        {
            // if turn ≠ 0 {
            if (turn.load() != tid)
            {
                // wants_to_enter[0] ← false
                wants_to_enter[tid].store(false);
                // while turn ≠ 0 {
                while (turn.load() != tid)
                    ; // busy wait
                // wants_to_enter[0] ← true
                wants_to_enter[tid].store(true);
            }
        }
    }

    void unlock(int tid)
    {
        int otherTid = (tid == 0) ? 1 : 0;
        turn.store(otherTid);
        wants_to_enter[tid].store(false);
    }

private:
    std::atomic_bool wants_to_enter[2];
    std::atomic_int turn;
};
