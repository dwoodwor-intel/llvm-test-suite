#include <CL/sycl.hpp>
#include <cassert>
#include <iostream>
#include <thread>

using namespace cl::sycl;
using namespace cl::sycl::access;

static constexpr size_t NUM_THREADS = 4;
static constexpr size_t RANGE_SIZE = 1024;

void assertFunc() { assert(false && "from assert statement"); }

template <class kernel_name> void assertTest(queue *Q) {
  Q->submit([&](handler &CGH) {
    CGH.parallel_for<kernel_name>(
        nd_range<2>{{RANGE_SIZE, RANGE_SIZE}, {1, 1}}, [=](nd_item<2> it) {
          if (it.get_global_id(0) == 7 && it.get_global_id(1) == 9)
            assertFunc();
        });
  });
  Q->wait();
}

void runTestForTid(queue *Q, size_t Tid) {
  switch (Tid % 4) {
  case 0: {
    assertTest<class the_kernel1>(Q);
    break;
  }
  case 1: {
    assertTest<class the_kernel2>(Q);
    break;
  }
  case 2: {
    assertTest<class the_kernel3>(Q);
    break;
  }
  case 3: {
    assertTest<class the_kernel4>(Q);
    break;
  }
  }
}

int main(int Argc, const char *Argv[]) {
  std::vector<std::thread> threadPool;
  threadPool.reserve(NUM_THREADS);

  std::vector<std::unique_ptr<queue>> Queues;
  for (size_t i = 0; i < NUM_THREADS; ++i) {
    Queues.push_back(std::make_unique<queue>());
  }

  for (size_t tid = 0; tid < NUM_THREADS; ++tid) {
    threadPool.push_back(std::thread(runTestForTid, Queues[tid].get(), tid));
  }

  for (auto &currentThread : threadPool) {
    currentThread.join();
  }

  std::cout << "The test ended." << std::endl;
  return 0;
}