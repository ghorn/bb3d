#include <bits/exception.h>  // for exception
#include <sys/types.h>       // for key_t, uint

#include <algorithm>           // for copy, max
#include <chrono>              // for operator""s, chrono_literals
#include <cstdio>              // for fprintf, stderr
#include <cstdlib>             // for EXIT_SUCCESS
#include <eigen3/Eigen/Dense>  // for Matrix, DenseCoeffsBase
#include <functional>          // for function
#include <iostream>            // for operator<<, basic_ostream, cerr, endl, ostream, cha...
#include <mutex>               // for mutex, lock_guard
#include <optional>            // for optional, nullopt
#include <queue>               // for queue
#include <thread>              // for sleep_for, thread
#include <vector>              // for vector

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>  // for glfwDestroyWindow, glfwGetWindowSize, glfwPollEv...

#include <glm/glm.hpp>  // for mat4
#include <nlopt.hpp>    // for opt, LN_NELDERMEAD

#include "bb3d/assert.hpp"             // for ASSERT
#include "bb3d/opengl_context.hpp"     // for Window
#include "bb3d/shader/colorlines.hpp"  // for Window

int run_it(char *argv0) {
  // Boilerplate
  bb3d::Window window(argv0);

  bb3d::ColorLines colored_lines;

  // it's theadn' time
  std::function<void(key_t)> handle_keypress = [&](key_t key) {
    std::cerr << "got keypress: " << key << std::endl;
  };

  std::function<void()> update_visualization = [&colored_lines]() {
    std::vector<std::vector<bb3d::ColoredVec3> > new_lines;
    const int nr = 10;
    const int nb = 16;
    for (int kr = 0; kr < nr; kr++) {
      std::vector<bb3d::ColoredVec3> new_line;
      for (int kb = 0; kb < nb; kb++) {
        const double x = -kr;  //(double)kr / (double)(nr - 1);
        const double y = -kb;  //(double)kb / (double)(nb - 1);
        const double z = 0;
        const double r = static_cast<double>(kr) / static_cast<double>(nr - 1);
        const double b = static_cast<double>(kb) / static_cast<double>(nb - 1);
        const double g = 1 - r * b;
        const double a = 1;
        const bb3d::ColoredVec3 v{{x, y, z}, {r, g, b, a}};
        new_line.push_back(v);
      }
      new_lines.push_back(new_line);
    }
    colored_lines.Update(new_lines);
  };

  std::function<void(const glm::mat4 &, const glm::mat4 &)> draw_visualization =
      [&colored_lines](const glm::mat4 &view, const glm::mat4 &proj) {
        colored_lines.Draw(view, proj, GL_LINES);
      };

  window.Run(handle_keypress, update_visualization, draw_visualization);

  return EXIT_SUCCESS;
}

int main(int argc __attribute__((unused)), char *argv[]) {
  try {
    run_it(argv[0]);
  } catch (const std::exception &e) {
    std::cerr << e.what();
  }
}
