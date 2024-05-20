#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include "array_ptr.h"

class ReserveProxyObj {
public:
  //  size_t new_capacity;
    ReserveProxyObj(size_t capacity_to_reserve)
        : new_capacity{ capacity_to_reserve } {}
    size_t GetNewCapacity() {
        return new_capacity;
    }
private:
    size_t new_capacity;
};

template <typename Type>
class SimpleVector {

private:
    ArrayPtr <Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : SimpleVector(size, Type()) {}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : items_(size), size_(size), capacity_(size)
    {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }


    // Конструктор копирования
    SimpleVector(const SimpleVector& other) : items_(new Type[other.size_]), size_(other.size_)
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }
    //Конструктор перемещения
    SimpleVector(SimpleVector&& other) {
       
       items_=std::move(other.items_);
       size_=std::exchange(other.size_,0);
       capacity_=std::exchange(other.capacity_,0);
    }

    //Конструктор с вызовом функции Reserve
    SimpleVector(ReserveProxyObj  obj)
    {
        Reserve(obj.GetNewCapacity());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

   
    SimpleVector& operator=(SimpleVector&& rhs){
    if(this!=&rhs){
    
       size_=std::exchange(rhs.size_,0);
       capacity_=std::exchange(rhs.capacity_,0);
       items_=std::move(rhs.items_);
    }
    return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return *(items_.Get() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return *(items_.Get() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {

        if (!(index < size_))
            throw std::out_of_range("Index must be less than vector size");
        return (*this)[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (!(index < size_))
            throw std::out_of_range("Index must be less than vector size");
        return (*this)[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        Resize(0);
    }
    void Fill(Iterator first, Iterator last)
    {
        assert(first <= last);

        for (; first != last; ++first)
        {
            *first = Type(); // move
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        if (new_size <= size_)
        {
            size_ = new_size;
        }
        if (new_size <= capacity_)
        {
            Fill(items_.Get() + size_, items_.Get() + new_size);
        }
        if (new_size > capacity_)
        {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);

            Fill(temp.Get(), temp.Get() + new_capacity);
            std::move(items_.Get(), items_.Get() + capacity_, temp.Get());

            items_.swap(temp);

            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> temp(new_capacity);

            std::copy(items_.Get(), items_.Get() + size_, temp.Get());

            items_.swap(temp);
            capacity_ = new_capacity;
        }
    }

    void PushBack(const Type& item) {
        if (size_ + 1 > capacity_)
        {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);

            std::copy(items_.Get(), items_.Get() + size_, temp.Get());

            items_.swap(temp);
            capacity_ = new_capacity;
        }

        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ + 1 > capacity_)
        {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);

            std::move(items_.Get(), items_.Get() + size_, temp.Get());
            items_.swap(temp);

            capacity_ = new_capacity;
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    Iterator insert(ConstIterator pos, const Type& value)
    {
        assert(pos >= begin() && pos <= end());

        size_t count = pos - items_.Get();

        if (capacity_ == 0)
        {
            ArrayPtr<Type> temp(1);
            temp[count] = value;
            items_.swap(temp);
            ++capacity_;
        }
        else if (size_ < capacity_)
        {
            std::copy_backward(items_.Get() + count, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[count] = value;
        }
        else
        {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);

            std::copy(items_.Get(), items_.Get() + size_, temp.Get());
            std::copy_backward(items_.Get() + count, items_.Get() + size_, temp.Get() + size_ + 1);

            temp[count] = value;
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;
        return &items_[count];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());

        size_t count = pos - items_.Get();

        if (capacity_ == 0)
        {
            ArrayPtr<Type> temp(1);

            temp[count] = std::move(value);
            items_.swap(temp);
            ++capacity_;
        }
        else if (size_ < capacity_)
        {
            std::move_backward(items_.Get() + count, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[count] = std::move(value);
        }
        else
        {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);

            std::move(items_.Get(), items_.Get() + size_, temp.Get());
            std::move_backward(items_.Get() + count, items_.Get() + size_, temp.Get() + size_ + 1);

            temp[count] = std::move(value);
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;

        return &items_[count];
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            Resize(size_ - 1);
        }
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());

        size_t count = pos - items_.Get();

        if (count > size_)
        {
            throw std::out_of_range("Position is out of range");
        }

        std::move(items_.Get() + count + 1, items_.Get() + size_, items_.Get() + count);
        --size_;

        return &items_[count];
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает итератор на начало массива
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
        
        if (&lhs == &rhs)  return true;
    
        return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    };

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
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