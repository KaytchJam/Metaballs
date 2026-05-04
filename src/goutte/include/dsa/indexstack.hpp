#pragma once

namespace gtt {
    namespace dsa {
    
        /** Wrapper over std::vector. The stack is tracked using an index. When pushing
         * we increment our index or grow the vector. When popping we decrement. */
        template <typename T>
        struct IndexedStack {
            std::vector<T> container;
            size_t stack_size = 0;

            IndexedStack reserve(size_t size) {
                container.reserve(size);
                return *this;
            }

            /** Push an item to the stack */
            IndexedStack& push(const T& t) {
                if (stack_size >= container.size()) {
                    container.push_back(t);
                } else {
                    container[stack_size] = t;
                }

                stack_size += 1;
                return *this;
            }

            // Pop an item off the stack, and return it by value
            T pop() {
                stack_size -= (int32_t) (stack_size > 0) *  1;
                return container[stack_size];
            }

            // Get the item at the top of the stack
            T& top() {
                return container[stack_size - 1];
            }

            // Return whether the stack is empty or not
            bool empty() const {
                return stack_size == 0;
            }

            // Reset the stack
            IndexedStack& reset() {
                stack_size = 0;
                return *this;
            }
        };
    }
}