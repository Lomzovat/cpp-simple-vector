#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>

template <typename Type>
class ArrayPtr {
public:
    
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        if (size) {
            raw_ptr_ = new Type[size];
        }
        else {
            raw_ptr_ = nullptr;
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    ArrayPtr(const ArrayPtr&) = delete;

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) noexcept {
        raw_ptr_ = other.Release();
    }

    ArrayPtr& operator=(ArrayPtr&& rhs) noexcept {
        if (this != &rhs) {
            raw_ptr_ = rhs.Release();
        }
        return *this;
    }

    ~ArrayPtr() {
        delete[]raw_ptr_;
    }

    [[nodiscard]] Type* Release() noexcept {
        Type* temp = raw_ptr_;
        raw_ptr_ = nullptr;
        return temp;
    }

    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        if (raw_ptr_) {
            return true;
        }
        else {
            return false;
        }
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};
