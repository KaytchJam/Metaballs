#pragma once

#include <cstdint>

namespace gtt {
    namespace dsa {

        /** Namespace for Generic wrapper types. */
        namespace wrap {

            /** Iterator that holds a single pointer to a type `T`. When the iterator is advanced
             * a single time it gets exhausted. */
            template <typename T>
            struct NullItemIterator {
                T* data = nullptr;
                    
                NullItemIterator(T* t) : data(t) {}
                NullItemIterator() : data(nullptr) {}

                /** Iterator traits */
                
                using difference_type = size_t;
                using value_type = T;
                using pointer = T*;
                using reference = T&;
                using iterator_category = std::forward_iterator_tag;
                
                reference operator*() { return *data; }
                pointer operator->() { return data; }
                
                NullItemIterator& operator++() {
                    data = nullptr;
                    return *this;
                }
                
                NullItemIterator operator++(int) {
                    NullItemIterator clone = *this;
                    (*this)++;
                    return clone;
                }
                
                bool operator==(const NullItemIterator& other) const { return this->data == other.data; }
                bool operator!=(const NullItemIterator& other) const { return this->data != other.data; }
            };

            /** Simple container that pairs some item of type `T` with an index. */
            template <typename T>
            struct IndexWrapper {
                T item;
                int32_t index;
                
                IndexWrapper(const T& t, int index) : item(t), index(index) {}
                
                T& operator*() { return item; }
                const T& operator*() const { return item; }
                
                T* operator->() { return &item; }
                const T* operator->() const { return &item; }
                
                using iterator = NullItemIterator<T>;
                
                iterator begin() { return iterator(&item); }
                iterator end() { return iterator(); }
            };

            /** Acts as a "mock" wrapper. It doesn't couple any information with the inner type `T`, and instead
             * exists to store and access said type `T` through the operators `*` and `->`. */
            template <typename T>
            struct PlaceboWrapper {
                T item;
                
                PlaceboWrapper(const T& t) : item(t) {};
                PlaceboWrapper(T&& t) : item(t) {}
                
                T& operator*() { return item; }
                const T& operator*() const { return item; }
                
                T* operator->() { return &item; }
                const T* operator->() const { return &item; }
                
                using iterator = NullItemIterator<T>;
                
                iterator begin() { return iterator(&item); }
                iterator end() { return iterator(); }
            };
        }
    }
}