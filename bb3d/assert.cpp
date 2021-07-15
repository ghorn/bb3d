#include <cstdlib>
#include <mutex>

namespace bb3d {

std::mutex g_exit_mutex;

[[noreturn]] void exit_thread_safe(int exit_code) {
  const std::lock_guard<std::mutex> lock(g_exit_mutex);
  exit(exit_code);  // NOLINT
}

};  // namespace bb3d
