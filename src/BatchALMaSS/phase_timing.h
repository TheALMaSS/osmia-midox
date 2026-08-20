#pragma once
//
// phase_timing.h — opt-in per-phase wall-clock profiler for the population Step loop.
//
// Accumulates total wall time spent in each named phase across the whole run, plus a call count, and
// prints a summary at shutdown. Zero cost unless PHASE_TIMING is defined at compile time.
//
// Usage: wrap a phase with a scoped timer -- { PHASE_TIMER(PH_Step); ...phase code... }
// or at function scope -- PHASE_TIMER(PH_DoFirst); at the top of the function.
// Print: PhaseTiming::Report();  (call once at CloseDownSim).
//
// Timers are additive and thread-agnostic: they measure WALL time of the (serial) region that drives each
// parallel phase, i.e. how long that phase takes end-to-end including the OpenMP fork/join and any
// contention inside. That is exactly the "where does the day's wall time go" signal we want.
//

#ifdef PHASE_TIMING
#include <atomic>
#include <chrono>
#include <cstdio>

enum PhaseKind {
    PH_SetStepDone = 0,  // per-day reset sweep
    PH_DoFirst,          // serial daily setup (UpdateOsmiaNesting, forage hours, density grid)
    PH_BeginStep,        // BeginStep phase (all animals)
    PH_DoBefore,
    PH_StepLoop,         // the whole do..while multi-pass Step phase (the suspected hot one)
    PH_DoAfter,          // DoAfter + CheckEmptySubArray
    PH_EndStep,          // EndStep phase
    PH_DoLast,           // serial daily teardown (population dynamics store, births/deaths)
    PH_OsmiaForage,      // Osmia st_ReproductiveBehaviour forage section (polygon-locked)
    PH_OsmiaFindNest,    // Osmia FindNestLocation (the spiral nest search)
    PH_OsmiaRepro,       // WHOLE Osmia_Female::st_ReproductiveBehaviour (superset of Forage+FindNest)
    PH_OsmiaFemDevelop,  // Osmia_Female::st_Develop
    PH_OsmiaDispersal,   // Osmia_Female::st_Dispersal
    PH_OsmiaPollenPoly,  // Osmia_Female::GetPollenInPolygon (pollen accounting, polygon-locked)
    PH_OsmiaImmStep,     // Egg/Larva/Prepupa/Pupa/InCocoon Step() -- the numerous immature stages
    PH_OsmiaNestStep,    // Osmia_Nest::Step
    PH_COUNT
};

namespace PhaseTiming {
    inline std::atomic<long long> g_ns[PH_COUNT];
    inline std::atomic<long long> g_n [PH_COUNT];
    inline void add(int k, long long ns){ g_ns[k].fetch_add(ns, std::memory_order_relaxed);
                                          g_n[k].fetch_add(1, std::memory_order_relaxed); }
    inline const char* name(int k){
        switch(k){ case PH_SetStepDone:return "SetStepDone"; case PH_DoFirst:return "DoFirst";
                   case PH_BeginStep:return "BeginStep"; case PH_DoBefore:return "DoBefore";
                   case PH_StepLoop:return "StepLoop(do-while)"; case PH_DoAfter:return "DoAfter+Refill";
                   case PH_EndStep:return "EndStep"; case PH_DoLast:return "DoLast";
                   case PH_OsmiaForage:return "OsmiaForage"; case PH_OsmiaFindNest:return "OsmiaFindNest";
                   case PH_OsmiaRepro:return "OsmiaRepro(all)"; case PH_OsmiaFemDevelop:return "OsmiaFemDevelop";
                   case PH_OsmiaDispersal:return "OsmiaDispersal"; case PH_OsmiaPollenPoly:return "OsmiaPollenInPoly";
                   case PH_OsmiaImmStep:return "OsmiaImmatureStep"; case PH_OsmiaNestStep:return "OsmiaNestStep";
                   default:return "?"; }
    }
    inline void Report(){
        std::printf("\n=== PHASE TIMING (wall time per phase over the whole run) ===\n");
        std::printf("%-20s %14s %14s %14s\n","phase","calls","total_s","ms/call");
        double tot=0; for(int k=0;k<PH_COUNT;k++) tot += g_ns[k].load()/1e9;
        for(int k=0;k<PH_COUNT;k++){
            long long c=g_n[k].load(), t=g_ns[k].load();
            std::printf("%-20s %14lld %14.3f %14.4f\n", name(k), c, t/1e9, c? (t/1e6)/(double)c : 0.0);
        }
        std::printf("(note: OsmiaForage/OsmiaFindNest are SUBSETS of StepLoop, summed across all bees/threads)\n");
        std::printf("============================================================\n");
    }
}

struct PhaseTimer {
    int k; std::chrono::high_resolution_clock::time_point t0;
    explicit PhaseTimer(int kind): k(kind), t0(std::chrono::high_resolution_clock::now()) {}
    ~PhaseTimer(){ PhaseTiming::add(k, std::chrono::duration_cast<std::chrono::nanoseconds>(
                       std::chrono::high_resolution_clock::now()-t0).count()); }
};
#define PHASE_TIMER(kind) PhaseTimer _pht_##__LINE__(kind)

#else
#define PHASE_TIMER(kind) ((void)0)
namespace PhaseTiming { inline void Report(){} }
#endif
