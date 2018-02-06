#if !defined(OMR_OM_TRANSITIONSET_INL_HPP_)
#define OMR_OM_TRANSITIONSET_INL_HPP_

#include <OMR/Om/ArrayBuffer.inl.hpp>
#include <OMR/Om/TransitionSet.hpp>

namespace OMR {
namespace Om {

inline bool TransitionSet::construct(Context& cx,
                                     MemHandle<TransitionSet> self) {
  auto table = ArrayBuffer<Entry>::allocate(cx, 32);
  assert(table != nullptr);
  if (table == nullptr) {
    return false;
  }
  self->table_ = table;
  return true;
}

inline ObjectMap* TransitionSet::lookup(
    Infra::Span<const SlotDescriptor> descriptors, std::size_t hash) const {
  std::size_t sz = size();

  for (std::size_t i = 0; i < sz; i++) {
    std::size_t idx = (hash + i) % sz;
    auto map = table_->get(idx).map;
    if (map == nullptr) {
      return nullptr;
    }
    if (descriptors == map->slotDescriptors()) {
      return map;
    }
  }
  return nullptr;
}

inline bool TransitionSet::tryStore(ObjectMap* map, std::size_t hash) {
  std::size_t sz = size();

  for (std::size_t i = 0; i < sz; i++) {
    std::size_t idx = (hash + i) % sz;
    if (table_->get(idx).map == nullptr) {
      table_->set(idx, {map});
      return true;
    }
  }
  return false;
}

template <typename VisitorT>
inline void TransitionSet::visit(Context& cx, VisitorT& visitor) {
  if (table_ == nullptr) {
    return;
  }

  visitor.edge(cx, (Cell*)this, (Cell*)table_);

  for (std::size_t i = 0; i < size(); i++) {
    const Entry& e = table_->get(i);
    if (e.map != nullptr) {
      visitor.edge(cx, (Cell*)this, (Cell*)e.map);
    }
  }
}

}  // namespace Om
}  // namespace OMR

#endif  // OMR_OM_TRANSITIONSET_INL_HPP_
