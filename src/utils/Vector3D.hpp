#pragma once

#include <vector>

// to make life easy :)
template <typename T>
class Vector3DView {
public:
    Vector3DView(std::vector<T>& data, size_t x_size, size_t y_size, size_t z_size)
        : data_(data), x_size_(x_size), y_size_(y_size), z_size_(z_size) 
    {
        assert(data.size() == x_size * y_size * z_size && "Vector size must match dimensions");
    }

    T& at(size_t x, size_t y, size_t z) {
        return data_[index(x, y, z)];
    }

    const T& at(size_t x, size_t y, size_t z) const {
        return data_[index(x, y, z)];
    }

    size_t x_size() const { return x_size_; }
    size_t y_size() const { return y_size_; }
    size_t z_size() const { return z_size_; }

private:
    size_t index(size_t x, size_t y, size_t z) const {
        return z * y_size_ * x_size_ + y * x_size_ + x;
    }

    std::vector<T>& data_;
    size_t x_size_, y_size_, z_size_;
};

// to make life easy x2
template <typename T>
class Vector3D {
    size_t x_size, y_size, z_size;
    std::vector<T> buff;
public:
    Vector3D(size_t x, size_t y, size_t z)
        : x_size(x), y_size(y), z_size(z), buff(x * y * z) {}
    
    Vector3D(size_t l) 
        : x_size(l), y_size(l), z_size(l), buff(l * l * l) {}

    T& at(size_t x, size_t y, size_t z) {
        return buff[index(x, y, z)];
    }

    const T& at(size_t x, size_t y, size_t z) const {
        return buff[index(x, y, z)];
    }

    void fill(const T& value) {
        std::fill(buff.begin(), buff.end(), value);
    }

    size_t size_x() const { return x_size; }
    size_t size_y() const { return y_size; }
    size_t size_z() const { return z_size; }

    const std::vector<T>& data() const { return buff; }
    std::vector<T>& data() { return buff; }

private:
    size_t index(size_t x, size_t y, size_t z) const {
        return z * y_size * x_size + y * x_size + x;
    }
};