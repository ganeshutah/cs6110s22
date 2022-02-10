#include <atomic>
#include <thread>
#include <iostream>

#if defined(SCDEKKERS)
#include "SCDekkers.h"
#elif defined(RDEKKERS)
#include "RDekkers.h"
#elif defined(TSODEKKERS)
#include "TSODekkers.h"
#else
#error "no mutex specified"
#endif

dekkers_mutex m;
int critical_section_total = 0;
std::atomic_int real_total(0);
std::atomic_bool finished(false);

void t0()
{
  while (finished.load(std::memory_order_relaxed) != true)
  {
    m.lock(0);
    critical_section_total++;
    m.unlock(0);
    atomic_fetch_add_explicit(&real_total, 1, std::memory_order_relaxed);
  }
}

void t1()
{
  while (finished.load(std::memory_order_relaxed) != true)
  {
    m.lock(1);
    critical_section_total++;
    m.unlock(1);
    atomic_fetch_add_explicit(&real_total, 1, std::memory_order_relaxed);
  }
}

int main()
{

  std::thread t0_thread = std::thread(t0);
  std::thread t1_thread = std::thread(t1);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  finished.store(true, std::memory_order_relaxed);
  t0_thread.join();
  t1_thread.join();

  std::cout << "throughput (critical sections per second): " << real_total / 2.0 << std::endl;
  std::cout << "number of critical sections: " << real_total << std::endl;
  std::cout << "number of mutual exclusion violations: " << real_total - critical_section_total << std::endl;
  std::cout << "percent of times that mutual exclusion was violated: " << 100.0 * (float(real_total - critical_section_total) / float(real_total)) << "%" << std::endl;
  return 0;
}
