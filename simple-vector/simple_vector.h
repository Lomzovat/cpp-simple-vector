#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity_to_reserve) :
        capacity_(capacity_to_reserve)
    {}
    size_t Reserve_capacity() {
        return capacity_;
    }
private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) :
        SimpleVector(size, Type{})
    {}

    SimpleVector(size_t size, const Type& value) :
        simple_vector_(size),
        size_(size),
        capacity_(size)
    {
        std::fill(simple_vector_.Get(), simple_vector_.Get() + size_, value);
    }

    SimpleVector(size_t size, Type&& value) :
        simple_vector_(size),
        size_(size),
        capacity_(size)
    {
        std::generate(simple_vector_.Get(), simple_vector_.Get() + size_, [value]() {return value;});
    }

    SimpleVector(std::initializer_list<Type> init) :
        simple_vector_(init.size()),
        size_(init.size()),
        capacity_(init.size())
    {
        std::copy(std::make_move_iterator(init.begin()), std::make_move_iterator(init.end()), begin());
    }

    SimpleVector(ReserveProxyObj capacity_to_reserve) {
        Reserve(capacity_to_reserve.Reserve_capacity());
    }

    SimpleVector(const SimpleVector& other) :
        simple_vector_(other.GetSize()),
        size_(other.GetSize()),
        capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), simple_vector_.Get());
    }

    SimpleVector(SimpleVector&& other) noexcept :
        simple_vector_(std::move(other.simple_vector_)),
        size_(std::exchange(other.size_, 0)),
        capacity_(std::exchange(other.capacity_, 0))
    {}

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            simple_vector_.swap(rhs.simple_vector_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }
        return *this;
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return simple_vector_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return simple_vector_[index];
    }

    Type& At(size_t index) {
        if (index < size_) {
            return simple_vector_[index];
        }
        else {
            throw std::out_of_range("This item doesn't exist");
        }
    }

    const Type& At(size_t index) const {
        if (index < size_) {
            return simple_vector_[index];
        }
        else {
            throw std::out_of_range("This item doesn't exist");
        }
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > size_) {
            if (new_size > capacity_) {
                size_t new_capacity = std::max(capacity_ * 2, new_size);
                Reserve(new_capacity);
            }
            else {
                std::generate(begin() + size_, end() + new_size, []() {return Type{}; });
                size_ = new_size;
            }
        }
        size_ = new_size;
    }

    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    void PushBack(const Type& item) {
        Insert(cend(), item);
    }

    void PushBack(Type&& item) {
        Insert(cend(), std::move(item));
    }

    Iterator begin() noexcept {
        return Iterator(&simple_vector_[0]);
    }

     Iterator end() noexcept {
        return Iterator(&simple_vector_[size_]);
    }

    ConstIterator begin() const noexcept {
        return cbegin();
    }

    ConstIterator end() const noexcept {
        return cend();
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator(&simple_vector_[0]);
    }

    ConstIterator cend() const noexcept {
        return ConstIterator(&simple_vector_[size_]);
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        if (pos >= begin() && pos <= end()) {
            auto index = std::distance(cbegin(), pos);
            if (size_ >= capacity_) {
                size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
                Reserve(new_capacity);
            }
            Iterator position = begin() + index;
            std::move_backward(position, cend(), end() + 1);
            ++size_;
            return position;
        }
        else
        {
            throw std::out_of_range("There is no such pos.");
        }
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        if (pos >= begin() && pos <= end()) {
            auto index = std::distance(cbegin(), pos);
            if (size_ >= capacity_) {
                size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
                Reserve(new_capacity);
            }
            Iterator position = begin() + index;
            std::move_backward(position, end(), end() + 1);
            ++size_;
            (*this)[index] = std::move(value);
            return position;
        }
        else {
            throw std::out_of_range("There is no such pos.");
        }
    }

    Iterator Erase(ConstIterator pos) {
        if (pos >= begin() && pos <= end()) {
            auto index = std::distance(cbegin(), pos);
            std::move(&simple_vector_[index + 1], end(), const_cast<Iterator>(pos));
            --size_;
            return const_cast<Iterator>(pos);
        }
        else {
            throw std::out_of_range("There is no such pos.");
        }
    }

    void swap(SimpleVector& other) noexcept {
        simple_vector_.swap(other.simple_vector_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> temp(new_capacity);
            std::move(begin(), end(), temp.Get());
            simple_vector_.swap(temp);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> simple_vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(),
        rhs.begin(), rhs.end()
    );
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
