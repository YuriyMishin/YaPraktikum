#pragma once

#include <initializer_list>
#include "array_ptr.h"
#include <algorithm>
#include <utility>

struct ReserveProxyObj {
    ReserveProxyObj() = default;
    ReserveProxyObj(size_t reserve_capacity) : capacity(reserve_capacity){}
    size_t capacity = 0;
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
    
    SimpleVector(ReserveProxyObj reserve) : size_(0), capacity_(reserve.capacity), items_(reserve.capacity) {
        std::fill(begin(), end(), 0);
    }
    
    SimpleVector(size_t size) : size_(size), capacity_(size), items_(size_) {
        std::fill(begin(), end(), 0);
    }
    
    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size), items_(size_) {
        std::fill(begin(), end(), value);
    }
    
    SimpleVector(std::initializer_list<Type> init) : size_(init.size()), capacity_(size_), items_(size_) {
        std::move(init.begin(), init.end(), begin());
    }
    
    SimpleVector(const SimpleVector& other) : size_(other.GetSize()), capacity_(other.GetCapacity()), items_(other.GetSize()) {
        std::copy(other.begin(), other.end(), begin());
    }
    
    SimpleVector(SimpleVector&& other) {
        swap(other);
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs){
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& other) {
        return *this = other;
    }

    void PushBack(const Type& item) {
        Insert(end(), item);
    }
    
    void PushBack(Type&& value) {
        Insert(end(), std::move(value));
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        return InsertItem(pos, value);
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        return InsertItem(pos, std::move(value));
    }
    
    Iterator InsertItem(ConstIterator pos, Type&& value) {
        auto dist = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = std::max(size_t(1), capacity_ * 2);
            ArrayPtr<Type> tmp_items(new_capacity);
            std::move(begin(), end(), tmp_items.Get());
            items_.swap(tmp_items);
            capacity_ = new_capacity;
        }
        auto pos_new_item = begin() + dist;
        std::move_backward(pos_new_item, end(), end() + 1);
        *(pos_new_item) = std::move(value);
        ++size_;
        return pos_new_item;
    }

    void PopBack() noexcept {
        if (size_) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        auto erase_num_pos = pos - begin();
        std::move(begin() + erase_num_pos + 1, end(), begin() + erase_num_pos);
        --size_;
        return begin() + erase_num_pos;
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp_items(new_capacity);
            std::move(begin(), end(), tmp_items.Get());
            items_.swap(tmp_items);
            capacity_ = new_capacity;
        }
    }
    
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> temp_items(new_capacity);
            std::copy(begin(), end(), temp_items.Get());
            items_.swap(temp_items);
            std::fill(end(), begin() + (new_capacity - capacity_), 0);
            capacity_ = new_capacity;
        } else if (new_size > size_) {
            std::fill(end(), begin() + new_size, Type{});
        }
        size_ = new_size;
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }
    
    size_t GetSize() const noexcept {
        return size_;
    }
    
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
    
    bool IsEmpty() const noexcept {
        return size_==0;
    }
    
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }
    
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }
    
    Type& At(size_t index) {
        if(index >= size_) {
            throw std::out_of_range("index does not exist");
        }
        return items_[index];
    }
    
    const Type& At(size_t index) const {
        if(index >= size_) {
            throw std::out_of_range("index does not exist");
        }
        return items_[index];
    }
    
    void Clear() noexcept {
        size_ = 0;
    }
    
    Iterator begin() noexcept {
        return items_.Get();
    }
    Iterator end() noexcept {
        return items_.Get() + size_;
    }
    ConstIterator begin() const noexcept {
        return items_.Get();
    }
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs==rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(),lhs.end(),rhs.begin(),rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
