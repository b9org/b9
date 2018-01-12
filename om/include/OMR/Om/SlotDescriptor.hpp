#if !defined(OMR_OM_MAPDESCRIPTOR_HPP_)
#define OMR_OM_MAPDESCRIPTOR_HPP_

#include <OMR/Infra/HashUtilities.hpp>
#include <OMR/Om/Id.hpp>

#include <cstddef>
#include <functional>

namespace OMR {
namespace Om {

class SlotType {
 public:
  // temporary default constructor, until we start passing type id's through
  // SlotMap constructors.
  constexpr SlotType() noexcept : id_(~0u) {}

  constexpr SlotType(Id id) noexcept : id_(id) {}

  constexpr Id id() const noexcept { return id_; }

  SlotType& id(Id id) noexcept {
    id_ = id;
    return *this;
  }

  constexpr std::size_t hash() const noexcept { return id_.hash(); }

  constexpr bool operator==(const SlotType& rhs) const noexcept {
    return id_ == rhs.id_;
  }

  constexpr bool operator!=(const SlotType& rhs) const noexcept {
    return id_ != rhs.id_;
  }

 private:
  Id id_;
};

class SlotDescriptor {
 public:
  SlotDescriptor(Id id) : id_(id) {}

  SlotDescriptor(const SlotType& type, const Id& id) : type_(type), id_(id) {}

  Id id() const { return id_; }

  SlotDescriptor& id(Id id) {
    id_ = id;
    return *this;
  }

  SlotType& type() { return type_; }

  const SlotType& type() const { return type_; }

  SlotDescriptor& type(const SlotType& type) {
    type_ = type;
    return *this;
  }

  constexpr std::size_t hash() const {
    return Infra::Hash::mix(id_.hash(), type_.hash());
  }

  constexpr bool operator==(const SlotDescriptor& rhs) const {
    return (id_ == rhs.id_) && (type_ == rhs.type_);
  }

  constexpr bool operator!=(const SlotDescriptor& rhs) const {
    return (id_ != rhs.id_) || (type_ != rhs.type_);
  }

 private:
  SlotType type_;
  Id id_;
};

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_MAPDESCRIPTOR_HPP_