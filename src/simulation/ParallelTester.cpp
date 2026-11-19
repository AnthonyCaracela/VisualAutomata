#include "ParallelTester.h"

#include "Result.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <future>
#include <thread>

namespace fa::sim {

ParallelTester::ParallelTester(unsigned maxThreads) : m_maxThreads(maxThreads) {}

std::vector<SimulationResult> ParallelTester::testAll(
    const fa::model::Automaton& automaton,
    const std::vector<std::string>& inputs,
    const ISimulationStrategy& strategy) const {

    std::vector<SimulationResult> results(inputs.size());
    if (inputs.empty()) {
        return results;
    }

    unsigned workers = m_maxThreads;
    if (workers == 0) workers = std::thread::hardware_concurrency();
    if (workers == 0) workers = 2;
    workers = std::min<unsigned>(workers, static_cast<unsigned>(inputs.size()));

    // Una cadena = una tarea. Si la estrategia lanza una excepcion, no se propaga.
    auto runOne = [&](const std::string& input) -> Result<SimulationResult> {
        try {
            return Result<SimulationResult>::success(strategy.run(automaton, input));
        } catch (const std::exception& e) {
            return Result<SimulationResult>::failure(e.what());
        } catch (...) {
            return Result<SimulationResult>::failure("error desconocido");
        }
    };

    // Cada hilo toma la siguiente cadena pendiente; cada indice lo escribe un solo hilo.
    std::atomic<std::size_t> nextIndex{0};
    auto worker = [&]() {
        for (std::size_t i = nextIndex.fetch_add(1); i < inputs.size();
             i = nextIndex.fetch_add(1)) {
            Result<SimulationResult> r = runOne(inputs[i]);
            if (r.ok()) {
                results[i] = r.value();
            } else {
                results[i].accepted = false;
                results[i].error = r.error();
            }
        }
    };

    std::vector<std::future<void>> futures;
    futures.reserve(workers);
    for (unsigned w = 0; w < workers; ++w) {
        futures.push_back(std::async(std::launch::async, worker));
    }
    for (auto& f : futures) {
        f.get();
    }
    return results;
}

}  // namespace fa::sim
