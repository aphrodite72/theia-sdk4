# Additional libraries and helpers

Standalone libraries and utilities that can aid in protection. Provided as-is, please review [LICENSE](LICENSE).

## Autocrypted

`autocrypted` is a C++20 container class designed to obscure contained data. Each instantiation of `autocrypted` is automatically seeded from the filename and line number where it is declared. This means that an attacker cannot simply find one instance of a key in order to decrypt every encrypted value.

### Configuration

Every key in autocrypted is seeded from the root seed. This root seed can be changed by defining `AUTOCRYPTED_SEED` in the preprocessor before including `autocrypted.hpp`. It is strongly recommended that you set this define in your build system (e.g., using `target_compile_definitions` in CMake) rather than in the source code. Failure to do so may result in undefined behavior due to mismatched definitions between translation units. Rotating the autocrypted root seed for each build means that an attacker would need to reanalyze the binary in order to recover new keys. **We recommend automating seed randomization within your build pipeline.**

### Limitations

1. In order for a type to be autocryptable, it needs to be [move_constructible](https://en.cppreference.com/w/cpp/types/is_move_constructible) and [destructible](https://en.cppreference.com/w/cpp/types/is_destructible). Both of these conditions are checked and enforced at compile time.
2. Additionally, the type should also be conforming to the requirements of [trivially_relocatable](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1144r7.html#intro). Unfortunately, this cannot be checked at the time, as it is not part of the standard yet.
