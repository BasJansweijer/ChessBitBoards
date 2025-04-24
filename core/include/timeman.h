#pragma once

#include <algorithm>
#include <chrono>

namespace chess
{
    using Time = std::chrono::milliseconds::rep;

    struct ClockState
    {
        Time btime; // blacks time
        Time wtime; // whites time
        Time binc;
        Time winc;

        // No increment
        ClockState(Time wtime, Time btime)
            : wtime(wtime), btime(btime), winc(0), binc(0)
        {
        }

        // No increment
        ClockState(Time wtime, Time btime, Time winc, Time binc)
            : wtime(wtime), btime(btime), winc(winc), binc(binc)
        {
        }

        template <bool whitesMove>
        Time currentMoveTime(int moveCounter)
        {
            Time ourTime = whitesMove ? wtime : btime;
            Time ourIncrement = whitesMove ? winc : binc;

            constexpr int AVG_GAME_LENGTH = 45;
            // we divide our think time evenly
            // for each second of increment we are 1 move less conservative in our time slices
            Time thinkTime = ourTime / (AVG_GAME_LENGTH - moveCounter);

            // we always use up our increment:
            thinkTime += ourIncrement;

            // when the alotted think time is more than 10% of our remaining time
            // we cap it to 10% of the remaining time
            thinkTime = std::min(ourTime / 10, thinkTime);

            return thinkTime;
        }
    };

    inline double timeToSeconds(Time t)
    {
        return t / 1000.0;
    }
}