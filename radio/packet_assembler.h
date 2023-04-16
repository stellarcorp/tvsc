#pragma once

#include <cstdint>
#include <iterator>
#include <unordered_map>
#include <unordered_set>

#include "radio/encoding.h"
#include "radio/fragment.h"

namespace tvsc::radio {

template <typename PacketT>
class PacketAssembler final {
 private:
  std::unordered_multimap<uint64_t, PacketT> incoming_{};
  std::unordered_set<uint64_t> complete_packets_{};

  // TODO(james): This looks useful. Extract this to a place where it can be used elsewhere.
  template <typename PairIterator>
  class SecondIterator {
   private:
    PairIterator iter_;

   public:
    // This iterator_category is a bit of a hack. SecondIterator implements bidirectional iterator
    // (std::bidirectional_iterator_tag). std::map and std::set can give bidirectional iterators,
    // but std::unordered_map and std::unordered_set can only give forward iterators. We assume that
    // PairIterator will be from one of those containers. If it is from a different container with a
    // more advanced iterator, this iterator_category will claim abilities that are not implemented.
    // Conversely, for the unordered containers, SecondIterator contains code that uses the
    // PairIterator as if it were a bidirectional iterator. This will be fine if it is used as a
    // forward iterator only, but will give misleading compiler messages otherwise.
    using iterator_category = typename PairIterator::iterator_category;
    using difference_type = typename PairIterator::difference_type;
    using value_type = typename PairIterator::value_type;
    using pointer = typename PairIterator::value_type::second_type*;
    using reference = typename PairIterator::value_type::second_type&;

    SecondIterator(PairIterator iter) : iter_(iter) {}

    reference operator*() const { return iter_->second; }
    pointer operator->() { return &iter_->second; }

    SecondIterator& operator++() {
      ++iter_;
      return *this;
    }

    SecondIterator operator++(int) {
      SecondIterator tmp{*this};
      ++iter_;
      return tmp;
    }

    // Note: may not actually be available. PairIterator must be at least a bidirectional itertor
    // for this work.
    SecondIterator& operator--() {
      --iter_;
      return *this;
    }

    // Note: may not actually be available. PairIterator must be at least a bidirectional itertor
    // for this work.
    SecondIterator operator--(int) {
      SecondIterator tmp{*this};
      --iter_;
      return tmp;
    }

    friend bool operator==(const SecondIterator& lhs, const SecondIterator& rhs) {
      return lhs.iter_ == rhs.iter_;
    };

    friend bool operator!=(const SecondIterator& lhs, const SecondIterator& rhs) {
      return lhs.iter_ != rhs.iter_;
    };
  };

 public:
  template <size_t MTU>
  void add_fragment(const Fragment<MTU>& fragment) {
    PacketT packet{};
    decode(fragment, packet);
    incoming_.insert({packet.header_hash(), std::move(packet)});
    if (packet.is_last_fragment()) {
      complete_packets_.insert(packet.header_hash());
    }
  }

  bool have_complete_packets() const { return !complete_packets_.empty(); }

  void consume_packet(PacketT& output) {
    const auto first{complete_packets_.begin()};
    const uint64_t id{*first};
    const auto range{incoming_.equal_range(id)};
    assemble(SecondIterator{range.first}, SecondIterator{range.second}, output);
    incoming_.erase(range.first, range.second);
    complete_packets_.erase(first);
  }

  PacketT consume_packet() {
    PacketT output{};
    consume_packet(output);
    return output;
  }
};

}  // namespace tvsc::radio
