// This file is derived from llvm::iterator_range.
#pragma once

#include "odb-util/config.hpp"
#include <utility>

namespace odb {

// A range adaptor for a pair of iterators.
//
// This just wraps two iterators into a range-compatible interface. Nothing
// fancy at all.
template <typename IteratorT> class IteratorRange
{
    IteratorT begin_iterator, end_iterator;

public:
    // TODO: Add SFINAE to test that the Container's iterators match the range's
    //       iterators.
    template <typename Container>
    explicit IteratorRange(Container&& c)
        // TODO: Consider ADL/non-member begin/end calls.
        : begin_iterator(c.begin()), end_iterator(c.end())
    {
    }
    IteratorRange(IteratorT begin_iterator, IteratorT end_iterator)
        : begin_iterator(std::move(begin_iterator)), end_iterator(std::move(end_iterator))
    {
    }

    IteratorT begin() const { return begin_iterator; }
    IteratorT end() const { return end_iterator; }
    bool empty() const { return begin_iterator == end_iterator; }
};

// Convenience function for iterating over sub-ranges.
//
// This provides a bit of syntactic sugar to make using sub-ranges
// in for loops a bit easier. Analogous to std::make_pair().
template <typename T> IteratorRange<T> makeRange(T x, T y)
{
    return IteratorRange<T>(std::move(x), std::move(y));
}

} // namespace odb
