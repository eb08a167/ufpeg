#ifndef UFPEG_REFERENCE_HPP
#define UFPEG_REFERENCE_HPP

#include <exception>

namespace ufpeg {
    class Reference {
    public:
        void resolve(std::size_t offset) {
            this->offset = offset;
            this->is_resolved = true;
        }

        std::size_t get_offset() const {
            if (this->is_resolved) {
                return this->offset;
            } else {
                throw std::logic_error("Reference is not yet resolved");
            }
        }
    private:
        std::size_t offset;
        bool is_resolved = false;
    };
}

#endif
