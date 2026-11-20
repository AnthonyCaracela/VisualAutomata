#include "ParallelTester.h"

#include <algorithm>
#include <atomic>
#include <exception>
#include <future>
#include <thread>

namespace fa::sim {

ParallelTester::ParallelTester(unsigned maxThreads)
    : m_maxThreads(maxThreads) {}

std::vector<SimulationResult> ParallelTester::testAll(
    const fa::model::Automaton& automaton,
    const std::vector<std::string>& inputs,
    const ISimulationStrategy& strategy) const {
    std::vector<SimulationResult> results(inputs.size());
    if (inputs.empty()) {
        return results;
    }

    std::size_t workers = m_maxThreads == 0
        ? std::thread::hardware_concurrency()
        : static_cast<std::size_t>(m_maxThreads);
    if (workers == 0) {
        workers = 1;
    }
    workers = std::min(workers, inputs.size());

    std::atomic<std::size_t> nextIndex{0};
    auto worker = [&]() {
        while (true) {
            const std::size_t index = nextIndex.fetch_add(1);
            if (index >= inputs.size()) {
                return;
            }
            try {
                results[index] = strategy.run(automaton, inputs[index]);
            } catch (const std::exception& exception) {
                results[index].error = exception.what();
            }
        }
    };

    std::vector<std::future<void>> futures;
    futures.reserve(workers);
    for (std::size_t index = 0; index < workers; ++index) {
        futures.push_back(std::async(std::launch::async, worker));
    }
    for (auto& future : futures) {
        future.get();
    }

    return results;
}

} // namespace fa::sim
