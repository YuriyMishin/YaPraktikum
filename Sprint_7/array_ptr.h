#include <cstdlib>
#include <algorithm>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;
    
    explicit ArrayPtr(size_t size) {
        if (size){
            raw_ptr_ = new Type [size]{};
        } else {
            raw_ptr_ = nullptr;
        }
    }
    
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
        
    }
    
    ArrayPtr(const ArrayPtr&) = delete;
    
    ~ArrayPtr() {
        delete [] raw_ptr_;
        raw_ptr_ = nullptr;
    }
    
    ArrayPtr& operator=(const ArrayPtr&) = delete;
    
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
        return raw_ptr_!=nullptr;
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
