#ifndef UFPEG_REFERENCE_HPP
#define UFPEG_REFERENCE_HPP

#include <cstddef>

namespace ufpeg {
    class Reference {
    public:
        void resolve(std::size_t offset) {
            this->offset = offset;
        }

        std::size_t get_offset() const {
            return this->offset;
        }
    private:
        std::size_t offset;
    };
}

#endif
