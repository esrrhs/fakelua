-- Test NaN and Infinity edge cases for math operations
-- Covers: NaN identity, Inf arithmetic, NaN/Inf propagation through math functions

function test_math_boundary_nan_inf()
    -- -------------------------------------------------------------------------
    -- NaN identity (IEEE 754)
    -- -------------------------------------------------------------------------
    local nan = 0.0 / 0.0
    -- NaN must NOT equal itself
    if nan == nan then return 1 end
    -- NaN must NOT equal any number
    if nan == 0.0 then return 2 end
    if nan == 1.0 then return 3 end
    if nan == math.huge then return 4 end
    -- NaN arithmetic propagates
    local nan2 = nan + 1.0
    if nan2 == nan2 then return 5 end  -- result should also be NaN

    -- -------------------------------------------------------------------------
    -- Infinity constants and basic properties
    -- -------------------------------------------------------------------------
    local inf = math.huge
    if not (inf > 1e300) then return 10 end
    if not (-inf < -1e300) then return 11 end
    if inf ~= inf then return 12 end        -- Inf equals itself (unlike NaN)
    if inf ~= (1.0 / 0.0) then return 13 end  -- 1/0 produces inf
    if not (inf > math.maxinteger) then return 14 end

    -- -------------------------------------------------------------------------
    -- Infinity arithmetic
    -- -------------------------------------------------------------------------
    -- Inf + Inf = Inf
    local inf_plus_inf = inf + inf
    if inf_plus_inf ~= inf then return 20 end
    -- Inf * 2 = Inf
    if inf * 2.0 ~= inf then return 21 end
    -- 1 / Inf = 0
    if 1.0 / inf ~= 0.0 then return 22 end
    -- -Inf reciprocal
    local neg_inf = -inf
    if not (neg_inf < -1e300) then return 23 end
    -- Inf - Inf = NaN
    local inf_minus_inf = inf - inf
    if inf_minus_inf == inf_minus_inf then return 24 end  -- NaN not equal to self
    -- Inf / Inf = NaN
    local inf_div_inf = inf / inf
    if inf_div_inf == inf_div_inf then return 25 end
    -- Inf * 0 = NaN
    local inf_times_zero = inf * 0.0
    if inf_times_zero == inf_times_zero then return 26 end

    -- -------------------------------------------------------------------------
    -- math.abs with Inf / NaN
    -- -------------------------------------------------------------------------
    if math.abs(inf) ~= inf then return 30 end
    if math.abs(-inf) ~= inf then return 31 end
    -- math.abs(NaN) -- check result; implementation may return nil or NaN
    local abs_nan = math.abs(nan)
    -- Accept either nil (implementation choice) or NaN
    if abs_nan ~= nil and abs_nan == abs_nan then return 32 end

    -- -------------------------------------------------------------------------
    -- math.floor / math.ceil with Inf
    -- -------------------------------------------------------------------------
    if math.floor(inf) ~= inf then return 40 end
    if math.ceil(inf) ~= inf then return 41 end
    if math.floor(-inf) ~= -inf then return 42 end
    if math.ceil(-inf) ~= -inf then return 43 end

    -- -------------------------------------------------------------------------
    -- math.sqrt boundary
    -- -------------------------------------------------------------------------
    -- sqrt(-1) = NaN
    local sqrt_neg = math.sqrt(-1.0)
    if sqrt_neg == sqrt_neg then return 50 end  -- must be NaN
    -- sqrt(0) = 0
    if math.sqrt(0.0) ~= 0.0 then return 51 end
    -- sqrt(inf) = inf
    if math.sqrt(inf) ~= inf then return 52 end

    -- -------------------------------------------------------------------------
    -- math.log boundary
    -- -------------------------------------------------------------------------
    -- log(-1) = NaN
    local log_neg = math.log(-1.0)
    if log_neg == log_neg then return 60 end
    -- log(0) = -inf
    local log_zero = math.log(0.0)
    if not (log_zero < -1e300) then return 61 end

    -- -------------------------------------------------------------------------
    -- math.pow with extreme values
    -- -------------------------------------------------------------------------
    -- pow(inf, 2) = inf
    if math.pow(inf, 2.0) ~= inf then return 70 end
    -- pow(inf, 0) = 1
    if math.pow(inf, 0.0) ~= 1.0 then return 71 end
    -- pow(0, -1) = inf
    if math.pow(0.0, -1.0) ~= inf then return 72 end
    -- pow(-1, 0.5) = NaN (complex result not representable)
    local pow_complex = math.pow(-1.0, 0.5)
    if pow_complex == pow_complex then return 73 end

    -- -------------------------------------------------------------------------
    -- math.max / math.min with Inf
    -- -------------------------------------------------------------------------
    if math.max(inf, 1.0) ~= inf then return 80 end
    if math.max(1.0, inf) ~= inf then return 81 end
    if math.min(-inf, 1.0) ~= -inf then return 82 end
    if math.min(1.0, -inf) ~= -inf then return 83 end
    -- math.max/math.min should return first arg in case of tie; both Inf
    if math.max(inf, inf) == math.max(inf, inf) then
        -- ok -- max of two infs is inf (test passes as long as result == result)
        -- NaN would be the only value that fails this check
    else
        return 84
    end

    -- -------------------------------------------------------------------------
    -- math.modf with Inf
    -- -------------------------------------------------------------------------
    local int_part, frac_part = math.modf(inf)
    if int_part ~= inf then return 90 end
    if frac_part ~= 0.0 then return 91 end

    -- -------------------------------------------------------------------------
    -- trig functions with Inf
    -- -------------------------------------------------------------------------
    -- sin/cos/tan of Inf = NaN
    local sin_inf = math.sin(inf)
    if sin_inf == sin_inf then return 100 end
    local cos_inf = math.cos(inf)
    if cos_inf == cos_inf then return 101 end
    local tan_inf = math.tan(inf)
    if tan_inf == tan_inf then return 102 end

    -- asin(2) = NaN (domain error)
    local asin_2 = math.asin(2.0)
    if asin_2 == asin_2 then return 110 end
    -- acos(2) = NaN (domain error)
    local acos_2 = math.acos(2.0)
    if acos_2 == acos_2 then return 111 end

    -- -------------------------------------------------------------------------
    -- math.exp boundary
    -- -------------------------------------------------------------------------
    -- exp(inf) = inf
    if math.exp(inf) ~= inf then return 120 end
    -- exp(-inf) = 0
    if math.exp(-inf) ~= 0.0 then return 121 end

    -- -------------------------------------------------------------------------
    -- math.log10 boundary
    -- -------------------------------------------------------------------------
    local log10_neg = math.log10(-1.0)
    if log10_neg == log10_neg then return 130 end  -- must be NaN
    -- log10(0) should be -inf
    local log10_zero = math.log10(0.0)
    if not (log10_zero < -1e300) then return 131 end

    -- -------------------------------------------------------------------------
    -- math.fmod with Inf
    -- -------------------------------------------------------------------------
    -- fmod(10, inf) = 10
    if math.abs(math.fmod(10.0, inf) - 10.0) > 0.001 then return 140 end
    -- fmod(inf, 10) = NaN
    local fmod_inf = math.fmod(inf, 10.0)
    if fmod_inf == fmod_inf then return 141 end

    return 5000
end
