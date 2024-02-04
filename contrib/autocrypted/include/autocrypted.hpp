#pragma once
#include <array>
#include <atomic>
#include <bit>
#include <type_traits>

#ifndef AUTOCRYPTED_SEED
#error Please define AUTOCRYPTED_SEED in your build system.
#endif

#if defined(__clang__)
#define AUTOCRYPTED_FORCE_INLINE     [[clang::always_inline]]
#define AUTOCRYPTED_FORCE_RUNTIME(u) asm("" : "=r"(u) : "0"(u) :)
#elif defined(__GNUC__)
#define AUTOCRYPTED_FORCE_INLINE     [[gnu::always_inline]]
#define AUTOCRYPTED_FORCE_RUNTIME(u) asm("" : "=r"(u) : "0"(u) :)
#elif defined(_MSC_VER)
#define AUTOCRYPTED_FORCE_INLINE __forceinline
#define AUTOCRYPTED_FORCE_RUNTIME(u) \
  do {                               \
    (void)u;                         \
  } while (0)
#else
#define AUTOCRYPTED_FORCE_INLINE
#define AUTOCRYPTED_FORCE_RUNTIME(u) \
  do {                               \
    (void)u;                         \
  } while (0)
#endif

namespace encrypted::utl {
  template <size_t N>
  using uint_by_size_t = std::conditional_t<N <= 1, uint8_t, std::conditional_t<N <= 2, uint16_t, std::conditional_t<N <= 4, uint32_t, std::conditional_t<N <= 8, uint64_t, void>>>>;

  template <typename T>
  using larger_uint_t = uint_by_size_t<sizeof(T)>;

  template <typename T>
  constexpr auto ceil_to_multiple_of(T val, T multiple_of) -> T {
    return (val + multiple_of - 1) / multiple_of * multiple_of;
  }

  template <typename T>
  constexpr auto bit_cast_from_uint(larger_uint_t<T> v) -> T {
    if constexpr (sizeof(v) == sizeof(T)) {
      return std::bit_cast<T>(v);
    } else {
      struct holder {
        T t;
        char pad[sizeof(v) - sizeof(T)];
      };

      holder h = std::bit_cast<holder>(v);
      return h.t;
    }
  }

  template <typename T, typename U = larger_uint_t<T>>
  constexpr auto bit_cast_to_uint(const T& v) -> U {
    if constexpr (sizeof(U) == sizeof(T)) {
      return std::bit_cast<U>(v);
    } else {
      struct holder {
        T t;
        char pad[sizeof(U) - sizeof(T)];
      };

      holder h{v, {}};
      return std::bit_cast<U>(h);
    }
  }

  template <typename T>
  constexpr auto bit_cast_to_bytes(const T& t) -> std::array<uint8_t, sizeof(T)> {
    return std::bit_cast<std::array<uint8_t, sizeof(T)>>(t);
  }

  template <typename T>
  constexpr auto bit_cast_from_bytes(const std::array<uint8_t, sizeof(T)>& a) {
    return std::bit_cast<T>(a);
  }

  constexpr const char* filename(const char* path) {
    const char* last = path;
    while (*path) {
      if (*path == '/' || *path == '\\')
        last = path + 1;
      ++path;
    }
    return last;
  }

  constexpr uint64_t fnv64a(const char* str, uint64_t seed = 0xcbf29ce484222325) {
    while (*str) {
      seed ^= (uint8_t)*str;
      seed *= 0x100000001B3;
      ++str;
    }
    return seed;
  }

  constexpr uint64_t fnv64a(unsigned x, uint64_t seed = 0xcbf29ce484222325) {
    seed ^= x;
    seed *= 0x100000001B3;
    return seed;
  }

  constexpr uint64_t autoseed(const char* filepath, int line) {
    uint64_t key = AUTOCRYPTED_SEED;
    key = fnv64a(line, key);
    key = fnv64a(filename(filepath), key);
    return key;
  }
} // namespace encrypted::utl

namespace encrypted {
  // See: https://eprint.iacr.org/2013/404.pdf
  template <size_t Rounds>
  struct speck_128_128 {
#define R(x, y, k) (x = std::rotr<uint64_t>(x, 8), x += y, x ^= k, y = std::rotl<uint64_t>(y, 3), y ^= x)

    AUTOCRYPTED_FORCE_INLINE static constexpr void encrypt(uint64_t ct[2], const uint64_t pt[2], const uint64_t K[2]) {
      uint64_t y = pt[0], x = pt[1], b = K[0], a = K[1];

      R(x, y, b);
      for (size_t i = 0; i < Rounds - 1; i++) {
        R(a, b, i);
        R(x, y, b);
      }

      ct[0] = y;
      ct[1] = x;
    }

#undef R
  };

  template <size_t N, uint64_t Key>
  class encrypted_small_storage {
    using U = utl::uint_by_size_t<std::bit_ceil(N)>;
    using UA = std::array<uint8_t, std::bit_ceil(N)>;
    using A = std::array<uint8_t, N>;

    U _s{};

    static constexpr auto _k = (U)utl::fnv64a("_k suffix", Key);
    static constexpr auto _d = (U)utl::fnv64a("_d suffix", Key);

    static constexpr U encode(U u) {
      return (U)(std::rotr((U)(u + _k), 7) ^ _d);
    }

    static constexpr U decode(U u) {
      return (U)(std::rotl((U)(u ^ _d), 7) - _k);
    }

    static U decode_runtime(U u) {
      AUTOCRYPTED_FORCE_RUNTIME(u);
      return decode(u);
    }

    static constexpr U encode_ua(const UA& ua) {
      return encode(utl::bit_cast_to_uint(ua));
    }

    static constexpr UA decode_ua(U u) {
      return utl::bit_cast_from_uint<UA>(decode(u));
    }

    static UA decode_ua_runtime(U u) {
      return utl::bit_cast_from_uint<UA>(decode(u));
    }

    static constexpr U encode_a(const A& a) {
      UA copy{};
      for (size_t i = 0; i < N; ++i)
        copy[i] = a[i];
      return encode_ua(copy);
    }

    static constexpr A decode_a(U u) {
      const auto copy = decode_ua(u);
      A a{};
      for (size_t i = 0; i < N; ++i)
        a[i] = copy[i];
      return a;
    }

    static A decode_a_runtime(U u) {
      const auto copy = decode_ua_runtime(u);
      A a{};
      for (size_t i = 0; i < N; ++i)
        a[i] = copy[i];
      return a;
    }

  public:
    constexpr void set(const A& array) {
      _s = encode_a(array);
    }

    constexpr A get() const {
      return decode_a(_s);
    }

    /// get(), but enforced that the decryption happens at runtime
    A get_runtime() const {
      return decode_a_runtime(_s);
    }

    A exchange(const A& desired) {
      static_assert(sizeof(std::atomic<U>) == sizeof(U));
      static_assert(alignof(std::atomic<U>) == alignof(U));
      auto& sa = (std::atomic<U>&)_s;

      return decode_a(sa.exchange(encode_a(desired)));
    }

    bool compare_exchange_weak(A& expected, const A& desired, std::memory_order order = std::memory_order_seq_cst) {
      static_assert(sizeof(std::atomic<U>) == sizeof(U));
      static_assert(alignof(std::atomic<U>) == alignof(U));
      auto& sa = *(std::atomic<U>*)&_s;

      auto e = encode_a(expected);
      const auto d = encode_a(desired);
      const auto ret = sa.compare_exchange_weak(e, d, order);
      expected = decode_a(e);
      return ret;
    }

    bool compare_exchange_strong(A& expected, const A& desired, std::memory_order order = std::memory_order_seq_cst) {
      static_assert(sizeof(std::atomic<U>) == sizeof(U));
      static_assert(alignof(std::atomic<U>) == alignof(U));
      auto& sa = *(std::atomic<U>*)&_s;

      auto e = encode_a(expected);
      const auto d = encode_a(desired);
      const auto ret = sa.compare_exchange_strong(e, d, order);
      expected = decode_a(e);
      return ret;
    }

    constexpr encrypted_small_storage() {
      set({});
    }
  };

  template <size_t N, uint64_t Key>
  class encrypted_big_storage {
  public:
    using S = std::array<uint64_t, 2>;
    static constexpr auto _n = utl::ceil_to_multiple_of(N, sizeof(S)) / sizeof(S);
    using U = std::array<S, _n>;
    using UA = std::array<uint8_t, _n * sizeof(S)>;
    using A = std::array<uint8_t, N>;

  private:
    // reduced round, insecure for proper cryptographic use but secure and fast enough for our purposes
    using Speck = speck_128_128<4>;

    U _s{};

    static constexpr uint64_t _k1 = Key;
    static constexpr uint64_t _k2 = utl::fnv64a("_k2 suffix", Key);
    static constexpr uint64_t _iv1 = utl::fnv64a("iv1 suffix", Key);
    static constexpr uint64_t _iv2 = utl::fnv64a("iv2 suffix", Key);

    static constexpr U encode(U u) {
      // CFB

      uint64_t k[2] = {_k1, _k2};
      uint64_t input[2] = {_iv1, _iv2};
      uint64_t out[2]{};
      for (auto& e : u) {
        Speck::encrypt(out, input, k);
        input[0] = (e[0] ^= out[0]);
        input[1] = (e[1] ^= out[1]);
      }
      return u;
    }

    static constexpr U decode(U u) {
      // CFB

      uint64_t k[2] = {_k1, _k2};
      uint64_t input[2] = {_iv1, _iv2};
      uint64_t out[2]{};
      for (auto& e : u) {
        Speck::encrypt(out, input, k);
        input[0] = e[0];
        input[1] = e[1];
        e[0] ^= out[0];
        e[1] ^= out[1];
      }
      return u;
    }

    // enforces runtime decryption
    static U decode_runtime(U u) {
      // CFB

      uint64_t k[2] = {_k1, _k2};
      uint64_t input[2] = {_iv1, _iv2};
      uint64_t out[2]{};

      // ensure input is materialized into a register to ensure
      // that clang won't drop it into rdata or evaluate it during
      // compile time
      AUTOCRYPTED_FORCE_RUNTIME(u);
      for (auto& e : u) {
        Speck::encrypt(out, input, k);
        input[0] = e[0];
        input[1] = e[1];
        e[0] ^= out[0];
        e[1] ^= out[1];
      }
      return u;
    }

    static constexpr U encode_ua(const UA& ua) {
      return encode(std::bit_cast<U>(ua));
    }

    static constexpr UA decode_ua(U u) {
      return std::bit_cast<UA>(decode(u));
    }

    static UA decode_ua_runtime(U u) {
      return std::bit_cast<UA>(decode_runtime(u));
    }

    static constexpr U encode_a(const A& a) {
      UA copy{};
      for (size_t i = 0; i < N; ++i)
        copy[i] = a[i];
      return encode_ua(copy);
    }

    static constexpr A decode_a(U u) {
      const auto copy = decode_ua(u);
      A a{};
      for (size_t i = 0; i < N; ++i)
        a[i] = copy[i];
      return a;
    }

    static A decode_a_runtime(U u) {
      const auto copy = decode_ua_runtime(u);
      A a{};
      for (size_t i = 0; i < N; ++i)
        a[i] = copy[i];
      return a;
    }

  public:
    constexpr void set(const A& array) {
      _s = encode_a(array);
    }

    constexpr A get() const {
      return decode_a(_s);
    }

    /// get(), but enforced that the decryption happens at runtime
    A get_runtime() const {
      return decode_a_runtime(_s);
    }

    constexpr const U& get_encrypted() const {
      return _s;
    }

    constexpr encrypted_big_storage() {
      set({});
    }
  };

  template <typename T>
  inline static constexpr auto atomic_encrypted_storage =
    // consider only small storage types
    sizeof(T) <= 8
    // type must be aligned
    && alignof(T) == sizeof(T)
    // type must be exactly power of 2 size
    && std::popcount(sizeof(T)) == 1
    // only trivially copyable types can be atomic
    && std::is_trivially_copyable_v<T>
    // finally check if atomic is lock free
    && std::atomic<T>::is_always_lock_free;

  template <typename T, uint64_t K>
  class encrypted_container {
    static_assert(std::is_move_constructible_v<T>);
    static_assert(std::is_destructible_v<T>);

    using S = std::conditional_t<sizeof(T) <= 8, encrypted_small_storage<sizeof(T), K>, encrypted_big_storage<sizeof(T), K>>;

    S _s;

    constexpr void initialize(const T& value) {
      if constexpr (std::is_trivially_copyable_v<T>) {
        _s.set(utl::bit_cast_to_bytes(value));
      } else {
        alignas(alignof(T)) std::array<uint8_t, sizeof(T)> backing{};
        new (&backing) T(value);
        _s.set(backing);
      }
    }

    constexpr void initialize(T&& value) {
      if constexpr (std::is_trivially_copyable_v<T>) {
        _s.set(utl::bit_cast_to_bytes(value));
      } else {
        alignas(alignof(T)) std::array<uint8_t, sizeof(T)> backing{};
        new (&backing) T(std::move(value));
        _s.set(backing);
      }
    }

    constexpr void destroy() {
      if constexpr (!std::is_trivially_copyable_v<T>) {
        alignas(alignof(T)) auto backing = _s.get();
        ((T*)&backing)->~T();
      }
    }

  public:
    constexpr void set(const T& value) {
      if constexpr (std::is_trivially_copyable_v<T>) {
        _s.set(utl::bit_cast_to_bytes(value));
      } else {
        alignas(alignof(T)) auto backing = _s.get();
        new (&backing) T(value);
        _s.set(backing);
      }
    }

    constexpr T get() const
      requires std::is_trivially_copyable_v<T>
    {
      return utl::bit_cast_from_bytes<T>(_s.get());
    }

    constexpr T get()
      requires(!std::is_trivially_copyable_v<T>)
    {
      alignas(alignof(T)) auto backing = _s.get();
      T t{*(const T*)&backing};
      _s.set(backing);
      return t; // implicit move
    }

    constexpr void set(T&& value) {
      if constexpr (std::is_trivially_copyable_v<T>) {
        _s.set(utl::bit_cast_to_bytes(value));
      } else {
        alignas(alignof(T)) auto backing = _s.get();
        *(T*)&backing = std::move(value);
        _s.set(backing);
      }
    }

    constexpr T take() {
      alignas(alignof(T)) auto backing = _s.get();
      T t{std::move(*(T*)&backing)};
      _s.set(backing);
      return t; // implicit move
    }

    T get_runtime() const
      requires std::is_trivially_copyable_v<T>
    {
      return utl::bit_cast_from_bytes<T>(_s.get_runtime());
    }

    T get_runtime()
      requires(!std::is_trivially_copyable_v<T>)
    {
      alignas(alignof(T)) auto backing = _s.get_runtime();
      T t{*(const T*)&backing};
      _s.set(backing);
      return t; // implicit move
    }

    T take_runtime() {
      alignas(alignof(T)) auto backing = _s.get_runtime();
      T t{std::move(*(T*)&backing)};
      _s.set(backing);
      return t; // implicit move
    }

    T exchange(const T& desired)
      requires std::is_trivially_copyable_v<T> && atomic_encrypted_storage<S>
    {
      const auto d = utl::bit_cast_to_bytes(desired);
      return utl::bit_cast_from_bytes<T>(_s.exchange(d));
    }

    bool compare_exchange_weak(T& expected, const T& desired, std::memory_order order = std::memory_order_seq_cst)
      requires std::is_trivially_copyable_v<T> && atomic_encrypted_storage<S>
    {
      auto e = utl::bit_cast_to_bytes(expected);
      const auto d = utl::bit_cast_to_bytes(desired);
      const auto ret = _s.compare_exchange_weak(e, d, order);
      expected = utl::bit_cast_from_bytes<T>(e);
      return ret;
    }

    bool compare_exchange_strong(T& expected, const T& desired, std::memory_order order = std::memory_order_seq_cst)
      requires std::is_trivially_copyable_v<T> && atomic_encrypted_storage<S>
    {
      auto e = utl::bit_cast_to_bytes(expected);
      const auto d = utl::bit_cast_to_bytes(desired);
      const auto ret = _s.compare_exchange_strong(e, d, order);
      expected = utl::bit_cast_from_bytes<T>(e);
      return ret;
    }

    constexpr encrypted_container()
      requires std::is_default_constructible_v<T>
    {
      if constexpr (std::is_pointer_v<T>) {
        _s.set({});
      } else {
        initialize(std::move(T{}));
      }
    }

    constexpr ~encrypted_container()
      requires(!std::is_trivially_destructible_v<T>)
    {
      destroy();
    }

    constexpr ~encrypted_container()
      requires std::is_trivially_destructible_v<T>
    = default;

    constexpr encrypted_container(const encrypted_container&)
      requires std::is_trivially_copy_constructible_v<T>
    = default;

    constexpr encrypted_container(const T& value)
      requires std::is_copy_constructible_v<T> // NOLINT(google-explicit-constructor)
    {
      initialize(value);
    }

    template <typename OtherT, uint64_t OtherS>
    constexpr encrypted_container(const encrypted_container<OtherT, OtherS>& rhs) // NOLINT(google-explicit-constructor)
    {
      initialize(rhs.get());
    }

    constexpr encrypted_container(encrypted_container&&) noexcept
      requires std::is_trivially_move_constructible_v<T>
    = default;

    template <typename OtherT, uint64_t OtherS>
    constexpr encrypted_container(encrypted_container<OtherT, OtherS>&& rhs) // NOLINT(google-explicit-constructor)
    {
      initialize(std::move(rhs.take()));
    }

    constexpr encrypted_container& operator=(const encrypted_container& rhs)
      requires std::is_trivially_copy_assignable_v<T>
    = default;

    template <typename OtherT, uint64_t OtherS>
    constexpr encrypted_container& operator=(const encrypted_container<OtherT, OtherS>& rhs) {
      set(rhs.get());
      return *this;
    }

    constexpr encrypted_container& operator=(encrypted_container&& rhs) noexcept
      requires std::is_trivially_move_assignable_v<T>
    = default;

    template <typename OtherT, uint64_t OtherS>
    constexpr encrypted_container& operator=(encrypted_container<OtherT, OtherS>&& rhs) {
      set(std::move(rhs.take()));
      return *this;
    }

    constexpr encrypted_container& operator=(const T& rhs) {
      set(rhs);
      return *this;
    }

    constexpr encrypted_container& operator=(T&& rhs) {
      set(std::move(rhs));
      return *this;
    }

    class borrowed {
      encrypted_container& _ec;
      T _t;

    public:
      constexpr explicit borrowed(encrypted_container& ec)
          : _ec(ec)
          , _t(std::move(ec.take())) {
      }

      constexpr ~borrowed() {
        _ec.set(std::move(_t));
      }

      constexpr T& operator*() {
        return _t;
      }

      constexpr T* operator->() {
        return &_t;
      }
    };

    constexpr borrowed borrow() {
      return borrowed{*this};
    }
  };
} // namespace encrypted

template <uint64_t K>
struct autocrypted_s {
  template <typename T>
  using type = ::encrypted::encrypted_container<T, K>;
};

#undef AUTOCRYPTED_FORCE_INLINE
#undef AUTOCRYPTED_FORCE_RUNTIME

// public interface macros:
#define AUTOSEED    ::encrypted::utl::autoseed(__FILE__, __LINE__)
#define autocrypted ::autocrypted_s<AUTOSEED>::type
