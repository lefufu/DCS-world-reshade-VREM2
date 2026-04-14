///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  code for stopwatch, done by claude
// ----------------------------------------------------------------------------------------
// 
// (c) Lefuneste.
//
// All rights reserved.
// https://github.com/xxx
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
//  * Redistributions of source code must retain the above copyright notice, this
//	  list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and / or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This software is using part of code or algorithms provided by
// * Crosire https://github.com/crosire/reshade  
// * FransBouma https://github.com/FransBouma/ShaderToggler
// * ShortFuse https://github.com/clshortfuse/renodx
// 
/////////////////////////////////////////////////////////////////////////

// ============================================================
//  chrono.hpp
//  Stopwatch management for ReShade addon
//
//  Usage :
//    Chrono::Start();
//    Chrono::Pause();
//    Chrono::Resume();
//    Chrono::Reset();
//
//    // In your ReShade render callback :
//    auto [h, m, s] = Chrono::GetTime();
//    // Pass h, m, s to your pixel shader via uniforms
// ============================================================

#pragma once
#include <chrono>

namespace Chrono
{
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double>;

    // ── Internal state ───────────────────────────────────────
    namespace _internal
    {
        inline bool      running = false;
        inline bool      paused = false;
        inline TimePoint startTime = {};
        inline Duration  accumulated = Duration::zero(); // time accumulated before pause
    }

    // ── Raw elapsed time (seconds) ───────────────────────────
    inline double ElapsedSeconds()
    {
        using namespace _internal;

        if (!running)
            return accumulated.count();

        if (paused)
            return accumulated.count();

        Duration since = Clock::now() - startTime;
        return (accumulated + since).count();
    }

    // ── Start ────────────────────────────────────────────────
    //  Resets and starts the stopwatch from zero.
    //  If already running, restarts from zero.
    inline void Start()
    {
        using namespace _internal;
        accumulated = Duration::zero();
        startTime = Clock::now();
        running = true;
        paused = false;
    }

    // ── Pause ────────────────────────────────────────────────
    //  Freezes the stopwatch. Elapsed time is preserved.
    //  No effect if already paused or stopped.
    inline void Pause()
    {
        using namespace _internal;
        if (!running || paused) return;

        accumulated += Clock::now() - startTime;
        paused = true;
    }

    // ── Resume ───────────────────────────────────────────────
    //  Resumes counting from the time at which it was paused.
    //  No effect if not paused.
    inline void Resume()
    {
        using namespace _internal;
        if (!running || !paused) return;

        startTime = Clock::now();
        paused = false;
    }

    // ── Reset ────────────────────────────────────────────────
    //  Resets to zero and stops the stopwatch.
    //  Call Start() after Reset() to begin a new count.
    inline void Reset()
    {
        using namespace _internal;
        accumulated = Duration::zero();
        startTime = {};
        running = false;
        paused = false;
    }

    // ── Shader accessors ─────────────────────────────────────
    //  Returns { hour [0..24[, minute [0..60[, second [0..60[ }
    //  as floats ready to be passed to shader uniforms.

    struct ChronoTime { float h, m, s; };

    inline ChronoTime GetTime()
    {
        double total = ElapsedSeconds();

        // Split into h / m / s
        int totalSec = static_cast<int>(total);
        int hours = totalSec / 3600;
        int minutes = (totalSec % 3600) / 60;
        double secs = total - static_cast<double>(totalSec - (totalSec % 60));
        // secs includes the fractional part for smooth needle rotation

        return {
            static_cast<float>(hours % 24),
            static_cast<float>(minutes),
            static_cast<float>(secs)
        };
    }

    // ── State queries ─────────────────────────────────────────
    inline bool IsRunning() { return _internal::running && !_internal::paused; }
    inline bool IsPaused() { return _internal::running && _internal::paused; }
    inline bool IsStopped() { return !_internal::running; }
}
