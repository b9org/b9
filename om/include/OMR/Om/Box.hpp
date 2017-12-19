#ifndef OMR_OM_BOX_HPP_
#define OMR_OM_BOX_HPP_

#include <OMR/Om/Cell.hpp>

namespace OMR {
namespace Om {

/// A managed cell of memory providing general-purpose raw storage.
/// Note that the DataCell is considered a leaf-object, there is no safe way to
/// store heap references inside a data object.
class DataCell : public Cell {
 public:
  DataCell(const DataCell&) = delete;
  DataCell() = delete;

  void* data() { return data_; }

  std::size_t hash() const {
    std::size_t hash = size_ + 1;
    for (std::size_t i = 0; i < size_; i++) {
      hash = mix(hash, data_[i]);
    }
    return hash;
  }

 protected:
  DataCell(std::size_t size) size_(size) {}

 private:
  std::size_t size_;
  char data_[0];
};

/// A box around a native data type, T. BoxCells provide a way to allocate
/// native data types on the managed heap. The box wraps the object in minimal
/// GC facilities. Note that, as a DataCell, there is no way to safetly store
/// GC references inside a BoxCell.
template <typename T>
class BoxCell : public DataCell {
 public:
  template <typename... Args>
  BoxCell(std::in_place_t, Args...&& args) : DataCell(sizeof(T)) {
    new (data()) T(std::forward<Args>(args)...);
  }

  T& unbox() { return *(T*)data(); }

  const T& unbox() const { return *(T*)data(); }
};

template <typename T>
class Box {};

class BoxRef {};

class BoxRootRef {};

}  // namespace Om
}  // namespace OMR

#endif // OMR_OM_BOX_HPP_
