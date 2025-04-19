
#include "engine.h"
#include <chrono>

namespace chess
{
    class Timer
    {
    public:
        Timer() : m_startTime(std::chrono::steady_clock::now()) {}

        void reset() { m_startTime = std::chrono::steady_clock::now(); }

        double elapsedSeconds() const
        {
            return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_startTime).count();
        }

    private:
        std::chrono::steady_clock::time_point m_startTime;
    };

    template <>
    void Engine::bench<Engine::BenchType::Depth>(double quantity)
    {
        // We need to convert the quantity to an int
        // since the depth is an int
        int depth = static_cast<int>(quantity);
        if (depth < 1)
        {
            std::cout << "Depth must be greater than 0" << std::endl;
            return;
        }

        Search::SearchConfig config;
        config.evalFunction = Evaluator::evaluate;
        config.repTable = &m_repTable;
        config.transTable = &m_transTable;

        Search s(m_currentBoard, config);

        Move out = Move::Null();
        Timer timer;
        if (m_currentBoard.whitesMove())
            s.minimax<true, true>(m_currentBoard, quantity, out);
        else
            s.minimax<false, true>(m_currentBoard, quantity, out);

        double seconds = timer.elapsedSeconds();
        std::cout << "took " << seconds << " seconds" << std::endl;
    }
}