-- Drives flua itself from CI to assert the host's lifecycle contract:
--   init once -> update repeatedly -> fini once before exit.
--
--   usage: flua cmd/test.lua     (entry points are fixed: package "main")
--
-- What it covers:
--   1) call counts and order: init exactly once, update repeatedly, fini exactly
--      once and last;
--   2) cross-frame state: update can be called again and again without losing
--      counters only because every call is its own top-level Call and the frame
--      arena is reset on entry -- the only thing that survives a frame is what
--      lives in container.map (C++ heap). File-level tables are constants, and a
--      temporary value assigned to a global at runtime dangles across frames,
--      which is why this script uses container.map throughout;
--   3) flua.set_exit semantics: requesting the exit mid-frame lets the rest of
--      the current frame run, the next frame is not entered, and only then fini;
--   4) the exit code is 0 (a failed assert errors out, so flua exits non-zero);
--   5) pcall / error still behave across frame boundaries.
--
-- Expected stdout (apart from fakelua's compile logs):
--   INIT UPDATE-1 UPDATE-2 UPDATE-3 SETEXIT AFTER-SET-EXIT FINI

package "main"

local st = container.map()

local function get(k)
    return st:get(k) or 0
end

local function bump(k)
    local n = get(k) + 1
    st:set(k, n)
    return n
end

function init()
    print("INIT")
    assert(get("init") == 0, "init must not be called twice")
    assert(get("update") == 0, "update must not have run before init finished")
    bump("init")
    -- pcall still catches errors across the frame boundary: error does not
    -- escape into the host.
    local ok = pcall(function()
        error("this error is expected to be caught by pcall")
    end)
    assert(ok == false, "pcall should catch the error and return false")
    -- Pretend init did some heavy one-off work (loading lookup tables, say) that
    -- update then reuses.
    st:set("prepared", 42)
    return 0
end

function update()
    local n = bump("update")
    print("UPDATE-" .. tostring(n))

    -- Values stored in container.map last frame are still there: the only
    -- correct way to keep state across frames.
    assert(st:get("prepared") == 42, "cross-frame state lost")
    assert(get("init") == 1, "init must have been called exactly once")
    assert(get("fini") == 0, "fini must not run while update is running")

    -- Frame-local values are only valid for this frame; they are reclaimed with
    -- the arena when the frame ends.
    local frame_only = {}
    frame_only[n] = true
    assert(frame_only[n] == true, "frame-local table must be usable within its frame")

    if n == 3 then
        print("SETEXIT")
        flua.set_exit(0)
        -- Asking to exit does not abort the frame: this frame runs to the end and
        -- only the next frame is skipped.
        print("AFTER-SET-EXIT")
        assert(frame_only[3] == true)
    end
    return 0
end

function fini()
    print("FINI")
    assert(get("init") == 1, "init should have run exactly once, got " .. tostring(get("init")))
    assert(get("update") == 3, "update should have run exactly 3 times, got " .. tostring(get("update")))
    assert(get("fini") == 0, "fini must not be called twice")
    bump("fini")
    return 0
end
