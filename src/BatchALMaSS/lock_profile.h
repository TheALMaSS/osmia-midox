#pragma once
//
// lock_profile.h — lightweight, opt-in lock-contention profiler.
//
// Measures, per named lock class, the total wall time threads spend WAITING to acquire the lock
// (contention) and the total time the lock is HELD, plus an acquisition count. Accumulators are atomic
// so it works across OpenMP threads. Zero cost unless LOCK_PROFILE is defined at compile time.
//
// Usage at a lock site (wait time):
//     { LP_WAIT(LPK_Polygon); omp_set_nest_lock(x); }   // scope ends right after acquire -> times the wait
// Held time: wrap the critical section:
//     LP_HELD_BEGIN(LPK_Polygon); ...critical...; LP_HELD_END(LPK_Polygon);
// Print at shutdown: LockProfile::Report();
//

#ifdef LOCK_PROFILE
#include <atomic>
#include <chrono>
#include <cstdio>

enum LockKind {
    LPK_Guard = 0,   // guard-map cell lock (PopulationManager pre-Step)
    LPK_Polygon,     // landscape polygon lock (Osmia Forage pollen/nectar depletion)
    LPK_NestCell,    // Osmia nest cell lock
    LPK_PolyList,    // Osmia poly-list nest lock (create/release nest)
    LPK_FemaleWeight,// Osmia female-weight record lock (testing)
    LPK_COUNT
};

namespace LockProfile {
    inline std::atomic<long long> g_wait_ns[LPK_COUNT];   // time blocked acquiring
    inline std::atomic<long long> g_held_ns[LPK_COUNT];   // time lock held
    inline std::atomic<long long> g_count [LPK_COUNT];    // acquisitions

    inline void addWait(int k, long long ns){ g_wait_ns[k].fetch_add(ns, std::memory_order_relaxed);
                                              g_count[k].fetch_add(1, std::memory_order_relaxed); }
    inline void addHeld(int k, long long ns){ g_held_ns[k].fetch_add(ns, std::memory_order_relaxed); }

    inline const char* name(int k){
        switch(k){ case LPK_Guard:return "Guard"; case LPK_Polygon:return "Polygon";
                   case LPK_NestCell:return "NestCell"; case LPK_PolyList:return "PolyList";
                   case LPK_FemaleWeight:return "FemaleWt"; default:return "?"; }
    }
    inline void Report(){
        std::printf("\n=== LOCK PROFILE (wait = contention time, held = critical-section time) ===\n");
        std::printf("%-10s %14s %14s %14s %12s\n","lock","acquisitions","wait_s","held_s","wait_us/acq");
        for(int k=0;k<LPK_COUNT;k++){
            long long c=g_count[k].load(), w=g_wait_ns[k].load(), h=g_held_ns[k].load();
            std::printf("%-10s %14lld %14.3f %14.3f %12.3f\n", name(k), c,
                        w/1e9, h/1e9, c? (w/1e3)/(double)c : 0.0);
        }
        std::printf("=========================================================================\n");
    }
}

// Times the wait to acquire: declare LP_WAIT just before the omp_set_nest_lock; its dtor fires right
// after acquisition returns, recording the elapsed (blocked) time.
struct LpWaitTimer {
    int k; std::chrono::high_resolution_clock::time_point t0;
    explicit LpWaitTimer(int kind): k(kind), t0(std::chrono::high_resolution_clock::now()) {}
    ~LpWaitTimer(){ LockProfile::addWait(k, std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::high_resolution_clock::now()-t0).count()); }
};
#define LP_WAIT(kind) LpWaitTimer _lpw_##__LINE__(kind)

// Held-time: record a start timestamp, then LP_HELD_END computes elapsed. Uses a thread_local stack-ish
// single slot per kind is not safe for nesting; for simple non-nested sections use the pair below.
#define LP_HELD_BEGIN(kind) { auto _lph0_##kind = std::chrono::high_resolution_clock::now();
#define LP_HELD_END(kind)   LockProfile::addHeld(kind, std::chrono::duration_cast<std::chrono::nanoseconds>( \
                                std::chrono::high_resolution_clock::now()-_lph0_##kind).count()); }

#else  // LOCK_PROFILE off -> everything compiles to nothing

#define LP_WAIT(kind)        ((void)0)
#define LP_HELD_BEGIN(kind)  {
#define LP_HELD_END(kind)    }
namespace LockProfile { inline void Report(){} }

#endif
