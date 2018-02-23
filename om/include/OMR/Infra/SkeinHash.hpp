#if !defined(OB_MAPTREE_DATA_HPP)
#define OB_MAPTREE_DATA_HPP_

namespace Ob {

class MapTreeData {
 public:
  SlotCount maxSlotCount;
};

}  // namespace Ob
}

/// https://en.wikipedia.org/wiki/Skein_(hash_function)
///

namespace OMR {

namespace ThreeFish {

/// Mix A and B
inline void mix(std::uint64_t a, std::uint64_t b) {}

using Tweak = std::uint64_t[2];
using Key256 = std::uint64_t[4];

template <typename Key>
void encrypt(const Key& key, const Tweak& tweak,
             const Span<const char>& plaintext) {}

void skein256_64(const Key256& key, const Tweak& tweak,
                 const std::uint64_t[4] & plain) {
  constexpr std::size_t ROUNDS = 72;
  for (word : key) {
  }
}

class Skein256 {
 public:
  Skein256() : state_ = ;

  update();
  digest(const);

 private:
  hash(const std::uint64_t[4] & plaintext) {
    const std::size_t ROUNDS = 72;
    for (std::size_t round = 0; round < ROUNDS; i++) {
      if (round % 4 == 0) {
        addSubKey(round);
      }
    }
  }

  static const std::size_t ROTATE_BITS[] = {14, 16, 52, 57, 23, 40, 5,  37,
                                            25, 33, 46, 12, 58, 22, 32, 32};

  static int getRotateBits(int d, int j) { return ROTATE_BITS[d * 2 + j]; }

  mix(int d, int j, std::uint64_t& a, std::uint64_t& b) {
    std::uint64_t a = (a + b) % (2 << 63);
    std::uint64_t b = rotateLeft(b, getRotateBits(round, )) ^ a;
  }

  std::uint64_t[4] state_;
};

skein512()
threefish64(Span<Byte> key, Span<

inline std::uint64_t rotateLeft(const std::uint64_t x, const std::uint8_t n) {
  assert(n < 64);
  return (x << n) | (x >> (-n & 31));
}

void mix(std::uint64_t x0, std::uint64_t x1, std::uint64_t& y0, std::uint64_t& y1) {
  y0 = x0 + x1 % (2 << 63);
  y1 = (x1 <<)
};

}  // namespace ThreeFish

namespace Skein {

struct Configuration {
  static std::size_t
};

template <std::size_t>
skein() {
  const std::size_t WORDS = 1;
}

template <typename T>
struct Skein {
  template <typename T>
  operator()
};

using skein64 = Skein<Skein64Config>;

encrypt()

}  // namespace Skein

struct Hash {
  template <typename T>
  operator()(const T& input) {}

  operator*() const { return data_; }
};