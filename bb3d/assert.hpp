#pragma once

#include <cstdio>   // for fprintf, stderr
#include <cstdlib>  // for exit, EXIT_FAILURE

namespace bb3d {

[[noreturn]] void exit_thread_safe(int exit_code);

#define ASSERT(expr)                                                                         \
  {                                                                                          \
    if (!(expr)) {                                                                           \
      fprintf(stderr, "Assertion '" #expr "' failed at %s, line %d.\n", __FILE__, __LINE__); \
      bb3d::exit_thread_safe(EXIT_FAILURE);                                                  \
    }                                                                                        \
  }

};  // namespace bb3d
