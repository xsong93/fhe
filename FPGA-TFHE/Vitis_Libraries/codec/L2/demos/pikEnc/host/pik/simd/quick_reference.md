# API synopsis / quick reference

## Compiler support

Some compilers require e.g. -mavx2 flags in order to use SIMD intrinsics.
However, this enables AVX2 instructions in the entire translation unit, which
may violate the one-definition rule and cause crashes. Instead, we use
target-specific attribute annotations: any function using SIMD must be prefixed
with `SIMD_ATTR`.

## Preprocessor macros

Let `Target` denote an instruction set: `NONE/SSE4/AVX2/AVX512/PPC8/ARM8`.

*   `SIMD_Target=##` are powers of two uniquely identifying `Target`.
*   `SIMD_ENABLE=##`, defined within `targets.h`, indicates for which Target(s)
    to generate code. The compiler must support them.

*   `SIMD_TARGET = Target` is the currently active instruction set; use this for
    instantiating the `operator()` template called by `Dispatch`. `Target` is a
    struct with a `value` member initialized to the `SIMD_Target` value.

*   `SIMD_TARGET_VALUE == Target::value == SIMD_Target` enables preprocessor
    `#if` based on target. Applications should only rarely need this; examples
    include avoiding `shift_*_var` if `SIMD_TARGET_VALUE == SIMD_SSE4` and
    avoiding shuffles etc. when `SIMD_TARGET_VALUE == SIMD_NONE`.

*   `SIMD_NAMESPACE` is useful for defining target-specific functions called by
    `operator()` specializations.

*   `SIMD_ATTR` expands to `SIMD_ATTR_Target` and must be prefixed to any
    inlined function declaration that (transitively) calls SIMD functions.

*   `SIMD_FULL(T)` and `SIMD_PART(T, N)` expand to `Full<T, SIMD_TARGET>` and
    `Part<T, N, SIMD_TARGET>`.

## Vector types

SIMD vectors consist of one or more 'lanes' of the same built-in type `T =
uint##_t, int##_t, float or double` for `## = 8, 16, 32, 64`. The API includes
three main families of data types:

*   Full vector corresponding to a SIMD register with `N` lanes;
*   Part of a vector with 2^j (<= `N`) contiguous lanes;
*   Scalar with a single lane, useful for loop remainders or portable code.

Full vector lane indices are in little-endian order: least-significant = lane 0.
Due to platform differences, the lane order of parts is undefined. For technical
reasons (see "Overloaded function API" in README.md), overloaded functions are
selected using 'descriptors' (abbreviated as `D`) rather than the actual data
types. For example, `setzero(Desc<T, N, Target>())` returns a `Desc<T, N,
Target>::V`. Users typically define a `Desc` lvalue `d` using alias templates:

*   `const Full<T, Target> d;` for a full vector;
*   `const Part<T, N, Target> d;` for a part or full vector with `N` lanes;
*   `const Scalar<T> d;` for scalars (or `Full<T, NONE>` or `Part<T, 1, NONE>`).

Initializers such as `setzero(d)` return the correct data type and user code can
rely on `auto` to avoid spelling out the data types. For output parameters or
type checking (rather than auto), use `D::V`.

## Operations

Let `V` denote a vector/part/scalar. Operations limited to certain types have
prefixes `V`: `u8/16` or `uif` for unsigned/signed/floating-point types.

### Initialization

*   `V setzero(D)`: returns vector/part/scalar with all bits set to zero.
*   `V set1(D, T)`: returns vector/part/scalar with all lanes set to `T`.
*   `V iota(D, T)`: returns vector/part/scalar with lanes `a[i] == T + i`.
*   `V undefined(D)`: returns vector/part/scalar with uninitialized lanes.

### Arithmetic

*   `V operator+(V a, V b)`: returns `a[i] + b[i]` (mod 2^bits).
*   `V operator-(V a, V b)`: returns `a[i] - b[i]` (mod 2^bits).
*   `V`: `ui8/16` \
    `V saturated_add(V a, V b)` returns `a[i] + b[i]` saturated to the
    minimum/maximum representable value.
*   `V`: `ui8/16` \
    `V saturated_subtract(V a, V b)` returns `a[i] - b[i]` saturated to the
    minimum/maximum representable value.
*   `V`: `u8/16` \
    `V average_round(V a, V b)` returns `(a[i] + b[i] + 1) / 2`.
*   `V`: `i8/16/32` \
    `V abs(V a)` returns the absolute value of `a[i]`; `LimitsMin()` maps to
    `LimitsMax() + 1`.

*   `V`: `ui16/32/64` \
    `V shift_left<int>(V a)` returns `a[i] <<` a compile-time constant count.
    Making it a template argument avoids constant-propagation issues with Clang
    on ARM. ARM also requires the count be less than the lane size. This is the
    fastest shift variant on x86.

*   `V`: `u16/32/64`, `i16/32` \
    `V shift_right<int>(V a)` returns `a[i] >>` a compile-time constant count.
    Making it a template argument avoids constant-propagation issues with Clang
    on ARM. ARM also requires the count be less than the lane size. This is the
    fastest shift variant on x86. Inserts zero or sign bit(s) depending on `V`.

*   `V`: `ui16/32/64` \
    `V shift_left_same(V a, Count bits)` returns `a[i] << bits`, where `bits` is
    returned from `set_shift_left_count(D, int)`.

*   `V`: `u16/32/64`, `i16/32` \
    `V shift_right_same(V a, Count bits)` returns `a[i] >> bits`, where `bits`
    is returned from `set_shift_right_count(D, int)`. Inserts 0 or sign bit(s).

*   `V`: `ui32/64` \
    `V operator<<(V a, V b)` returns `a[i] << b[i]`, which is zero when the
    shift count `b[i] >= sizeof(T)*8`. Not supported by SSE4, but more efficient
    than the `shift_*_same` functions on AVX2+.

*   `V`: `u32/64`, `i32` \
    `V operator>>(V a, V b)` returns `a[i] >> b[i]`, which is zero when the
    shift count `b[i] >= sizeof(T)*8`. Not supported by SSE4, but more efficient
    than the `shift_*_same` functions on AVX2+. Inserts zero or sign bit(s).

*   `V`: `ui8/16/32`, `f` \
    `V min(V a, V b)`: returns `min(a[i], b[i])`.

*   `V`: `ui8/16/32`, `f` \
    `V max(V a, V b)`: returns `max(a[i], b[i])`.

*   `V`: `ui8/16/32`, `f` \
    `V clamp(V a, V lo, V hi)`: returns `a[i]` clamped to `[lo[i], hi[i]]`.

*   `V`: `ui16/32` \
    `V operator*(V a, V b)`: returns the lower half of `a[i] * b[i]` in each
    lane.

*   `V`: `f` \
    `V operator*(V a, V b)`: returns `a[i] * b[i]` in each lane.

*   `V`: `f` \
    `V operator/(V a, V b)`: returns `a[i] / b[i]` in each lane.

*   `V`: `i16` \
    `V ext::mul_high(V a, V b)`: returns the upper half of `a[i] * b[i]` in each
    lane.

*   `V`: `i16` \
    `V mul_high_round(V a, V b)`: returns `(((a[i] * b[i]) >> 14) + 1) >> 1`.

*   `V`: `ui32` \
    `V mul_even(V a, V b)`: returns double-wide result of `a[i] * b[i]` for
    every even `i`, in lanes `i` (lower) and `i + 1` (upper).

*   `V`: `f` \
    `V mul_add(V a, V b, V c)`: returns `a[i] * b[i] + c[i]`.

*   `V`: `f` \
    `V nmul_add(V a, V b, V c)`: returns `-a[i] * b[i] + c[i]`.

*   `V`: `f` \
    `V fadd(V x, V k1, V add)`: returns `x[i] + add[i]` (k1 must equal 1.0).
    Clobbers the register holding x due to 3-address encoding.

*   `V`: `f` \
    `V fsub(V x, V k1, V sub)`: returns `x[i] - sub[i]` (k1 must equal 1.0).
    Clobbers the register holding x due to 3-address encoding.

*   `V`: `f` \
    `V fnadd(V sub, V k1, V x)`: returns `x[i] - sub[i]` (k1 must equal
    1.0). Clobbers the register holding sub due to 3-address encoding.

*   `V`: `f` \
    `V ext::mul_subtract(V a, V b, V c)`: returns `a[i] * b[i] - c[i]`.

*   `V`: `f` \
    `V ext::nmul_subtract(V a, V b, V c)`: returns `-a[i] * b[i] - c[i]`.

*   `V`: `f` \
    `V sqrt(V a)`: returns `sqrt(a[i])`.

*   `V`: `f32` \
    `V approximate_reciprocal_sqrt(V a)`: returns an approximation of `1.0 /
    sqrt(a[i])`. `sqrt(a) ~= approximate_reciprocal_sqrt(a) * a`. x86 and PPC
    provide 12-bit approximations but the error on ARM may be closer to 1%.

*   `V`: `f32` \
    `V approximate_reciprocal(V a)`: returns an approximation of `1.0 / a[i]`.

*   `V`: `f` \
    `V round(V a)`: returns `a[i]` rounded towards the nearest integer, with
    ties to even.

*   `V`: `f` \
    `V trunc(V a)`: returns `a[i]` rounded towards zero (truncate).

*   `V`: `f` \
    `V ceil(V a)`: returns `a[i]` rounded towards positive infinity (ceiling).

*   `V`: `f` \
    `V floor(V a)`: returns `a[i]` rounded towards negative infinity.

### Comparisons

These set a lane to 1-bits if the condition is true, otherwise all zero.

*   `V operator==(V a, V b)`: returns `a[i] == b[i]`.
*   `V`: `if` \
    `V operator<(V a, V b)`: returns `a[i] < b[i]`.
*   `V`: `if` \
    `V operator>(V a, V b)`: returns `a[i] > b[i]`.
*   `V`: `f` \
    `V operator<=(V a, V b)`: returns `a[i] <= b[i]`.
*   `V`: `f` \
    `V operator>=(V a, V b)`: returns `a[i] >= b[i]`.

### Logical

These operate on individual bits, even for floating-point vector types.

*   `V operator&(V a, V b)`: returns `a[i] & b[i]`.
*   `V andnot(V a, V b)`: returns `~a[i] & b[i]`.
*   `V operator|(V a, V b)`: returns `a[i] | b[i]`.
*   `V operator^(V a, V b)`: returns `a[i] ^ b[i]`.
*   `V`: `f` \
    `V condition_from_sign(V v)`: returns `s` such that `select(a, b, s)` is
    equivalent to `v.sign_bit ? b : a`. This is a no-op on x86.
*   `V select(V a, V b, V mask)`: returns `mask[i] ? b[i] : a[i]`. **Note**:
    each `mask[i]` must be all zero or all 1-bits, or returned from
    `condition_from_sign`.
*   `V odd_even(V a, V b)`: returns a vector whose odd lanes are taken from `a`
    and the even lanes from `b`.

### Memory

Memory operands are little-endian, otherwise their order would depend on the
lane configuration. Pointers are the addresses of `N` consecutive `T` values,
either naturally-aligned (`aligned`) or possibly unaligned (`p`).

*   `D::V load(D, const D::T* aligned)`: returns `aligned[i]`. **Note**: the
    lane order of parts is undefined; use `broadcast_part` to get a full vector.
*   `D::V load_unaligned(D, const D::T* p)`: returns `p[i]`.
*   `D::V load_dup128(D, const D::T* p)`: returns one 128-bit block loaded from
    `p` and broadcasted into all 128-bit block\[s\]. This enables a `convert_to`
    overload that avoids a 3-cycle overhead on AVX2/AVX-512. This is faster than
    broadcasting single values and useful for specifying constants without
    having to know the (maximum) vector length.
*   `void store(D::V a, D, D::T* aligned)`: copies `a[i]` into `aligned[i]`.
*   `void store_unaligned(D::V a, D, D::T* p)`: copies `a[i]` into `p[i]`.
*   `void stream(D::V a, D, const D::T* aligned)`: copies `a[i]` into
    `aligned[i]` with non-temporal hint on x86 (for good performance, call for
    all consecutive vectors within the same cache line).
*   `V`,`VI`: (`uif32,i32`), (`uif64,i64`) \
    `D::V gather_offset(D, const D::T* base, VI offsets)`. Returns elements of
    base selected by signed/possibly repeated *byte* `offsets[i]`.
*   `V`,`VI`: (`uif32,i32`), (`uif64,i64`) \
    `D::V gather_index(D, const D::T* base, VI indices)`. Returns vector of
    `base[indices[i]]`. Indices are signed and need not be unique.
*   `T`: `u32/64` \
    `void stream(T, T* aligned)`: copies `T` into `*aligned` with non-temporal
    hint on x86.

*   `void load_fence()`: delays subsequent loads until prior loads are visible.
    Also a full fence on Intel CPUs. No effect on non-x86.

*   `void store_fence()`: ensures previous non-temporal stores are visible. No
    effect on non-x86.

*   `void flush_cacheline(const void* p)`: invalidates and flushes the cache
    line containing "p". No effect on non-x86.

*   `void prefetch(const T* p)`: begins loading the cache line containing "p".

### Type conversion

*   `D::V cast_to(D, V)`: returns the bits of `V` reinterpreted as type `D::V`.

*   `V`,`D`: (`u8,i16`), (`u8,i32`), (`u16,i32`), (`i8,i16`), (`i8,i32`),
    (`i16,i32`), (`f32,f64`) \
    `D::V convert_to(D, V part)`: returns `part[i]` zero- or sign-extended to
    the wider `D::T` type.

*   `V`,`D`: (`u8,u32`) \
    `D::V u32_from_u8(V)`: special-case `u8` to `u32` conversion when all blocks
    of `V` are identical, e.g. from `load_dup128`.

*   `V`,`D`: (`u32,u8`) \
    `D::V u8_from_u32(V)`: special-case `u32` to `u8` conversion when all lanes
    of `V` are already clamped to `[0, 256)`.

*   `V`,`D`: (`i16,i8`), (`i32,i8`), (`i32,i16`), (`i16,u8`), (`i32,u8`),
    (`i32,u16`) \
    `D::V convert_to(D, V a)`: returns `a[i]` after packing with signed/unsigned
    saturation, i.e. a vector part with narrower lane type `D::T`.

*   `V`,`D`: (`i32`,`f32`) \
    `D::V convert_to(D, V)`: converts an int32_t value to float.

*   `V`,`D`: (`f32`,`i32`) \
    `D::V convert_to(D, V)`: rounds float towards zero and converts the value to
    int32_t.

*   `V`: `f32`; `Ret`: `i32` \
    `Ret nearest_int(V a)`: returns the integer nearest to `a[i]`.

### Parts

The part abstraction is necessary because the preferred lane to get/set differs
depending on platform.

*   `D::N == 1` \
    `D::V set_part(D, D::T)`: returns a part containing the single value `T` in
    an unspecified lane.

*   `D::V any_part(D, V)`: returns a contiguous part of the full vector `V`.
    **Note**: returns either the least- or most-significant bits depending on
    platform; use `broadcast_part` to obtain a full vector.

*   `V2 get_half(Upper/Lower, V)`: returns upper or lower half of the full
    vector `V`. `SIMD_HALF` evaluates to an instance of Upper or Lower
    (whichever is more efficient) that can be passed as the first argument. When
    a specific half is needed, `V2 upper_half(V)` and `V2 lower_half(V)` are
    more convenient alternatives.

*   `D::V broadcast_part<int i>(D, V)`: returns a full vector with the `i`-th
    element broadcasted. The interpretation of `i < N` is platform-dependent.
    For `V` from `load(Part<T, N>(), p)`, `i` is the index into `p[]`; for `V`
    from `set_part`, `N == 1` and thus `i == 0`.

*   `D::N == 1` \
    `D::T get_part(D, V)`: returns the single value stored within `V`. This is
    also useful for extracting `sum_of_lanes` results.

### Swizzle

**Note**: if vectors are larger than 128 bits, the following operations split
their operands into independently processed 128-bit *blocks*.

*   `V`: `ui16/32/64`, `f` \
    `V broadcast<int i>(V)`: returns individual *blocks*, each with lanes set to
    `input_block[i]`, `i = [0, 16/sizeof(T))`.

*   `Ret`: double-width `u/i`; `V`: `u8/16/32`, `i8/16/32` \
    `Ret zip_lo(V a, V b)`: returns the same bits as interleave_lo, except that
    `Ret` is a vector with double-width lanes (required in order to use this
    operation with `scalar`).

*   `Ret`: double-width u/i; `V`: `u8/16/32`, `i8/16/32` \
    `Ret zip_hi(V a, V b)`: returns the same bits as interleave_hi, except that
    `Ret` is a vector with double-width lanes (required in order to use this
    operation with `scalar`).

**Note**: the following are only available for full vectors (`N > 1, Target !=
NONE`), and split their operands into independently processed 128-bit *blocks*:

*   `Ret`: half-sized vector part \
    `Ret other_half(V v)`: returns the other half-sized vector part, i.e. the
    part not returned by `any_part(Desc<T, N / 2>, V)`.

*   `V`: `ui` \
    `V shift_left_bytes<int>(V)`: returns the result of shifting independent
    *blocks* left by `int` bytes \[1, 15\].

*   `V`: `ui` \
    `V shift_left_lanes<int>(V)`: returns the result of shifting independent
    *blocks* left by `int` lanes \[1, 15\].

*   `V`: `ui` \
    `V shift_right_bytes<int>(V)`: returns the result of shifting independent
    *blocks* right by `int` bytes \[1, 15\].

*   `V`: `ui` \
    `V shift_right_lanes<int>(V)`: returns the result of shifting independent
    *blocks* right by `int` lanes \[1, 15\].

*   `V`: `ui` \
    `V combine_shift_right_bytes<int>(V hi, V lo)`: returns the result of
    shifting two concatenated *blocks* `hi[i] || lo[i]` right by `int` bytes
    \[1, 15\].

*   `V`: `ui`; `VI`: `ui` \
    `V table_lookup_bytes(V bytes, VI from)`: returns *blocks* with
    `bytes[from[i]]`, or zero if `from[i] >= 0x80`.

*   `V`: `uif32` \
    `V shuffle_1032(V)`: returns *blocks* with 64-bit halves swapped.

*   `V`: `uif64` \
    `V shuffle_01(V)`: returns *blocks* with 64-bit halves swapped.

*   `V`: `uif32` \
    `V shuffle_0321(V)`: returns *blocks* rotated right (toward the lower end)
    by 32 bits.

*   `V`: `uif32` \
    `V shuffle_2103(V)`: returns *blocks* rotated left (toward the upper end) by
    32 bits.

*   `V`: `uif32` \
    `V shuffle_0123(V)`: returns *blocks* with lanes in reverse order.

*   `V interleave_lo(V a, V b)`: returns *blocks* with alternating lanes from
    the lower halves of `a` and `b` (`a[0]` in the least-significant lane).

*   `V interleave_hi(V a, V b)`: returns *blocks* with alternating lanes from
    the upper halves of `a` and `b` (`a[N/2]` in the least-significant lane).

**Note**: the following operations cross block boundaries, which is typically
more expensive on AVX2/AVX-512 than within-block operations.

*   `V concat_lo_lo(V hi, V lo)`: returns the concatenation of the lower halves
    of `hi` and `lo` without splitting into blocks.
*   `V concat_hi_hi(V hi, V lo)`: returns the concatenation of the upper halves
    of `hi` and `lo` without splitting into blocks.
*   `V concat_lo_hi(V hi, V lo)`: returns the inner half of the concatenation of
    `hi` and `lo` without splitting into blocks. Useful for swapping the two
    blocks in 256-bit vectors.
*   `V concat_hi_lo(V hi, V lo)`: returns the outer quarters of the
    concatenation of `hi` and `lo` without splitting into blocks. Unlike the
    other variants, this does not incur a block-crossing penalty on AVX2.

*   `V`: `uif32` \
    `V table_lookup_lanes(V a, VI)` returns a vector of `a[indices[i]]`,
    where `VI` is returned from `set_lane_indices(D, &indices[0])`.

*   `VI set_lane_indices(D, int* idx)` prepares for `table_lookup_lanes`
    with lane indices `idx = [0, d.N)` (need not be unique).

### Misc

*   `void pause()`: call during spin loops to reduce power consumption.

**Note**: the following are only available for full vectors (`N > 1, Target !=
NONE`):

*   `V`: `u8`, `f` \
    `uint32_t ext::movemask(V a)`: returns sum of `upper_bit(a[i]) << i`.

*   `V`: `ui` \
    `bool ext::all_zero(V a)`: returns whether all lanes are zero.

*   `V`: `u8`; `Ret`: `u64` \
    `Ret ext::sums_of_u8x8(V)`: returns the sums of 8 consecutive bytes in each
    64-bit lane.

*   `V`: `uif32/64` \
    `V ext::sum_of_lanes(V v)`: returns the sum of all lanes in each lane; to
    obtain the result, use `get(D, horz_sum_result)`.
