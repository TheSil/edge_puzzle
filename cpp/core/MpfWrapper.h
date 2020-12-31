#pragma once

#include <string>
#include <gmp.h>

namespace edge {

namespace backtracker {

class MpfWrapper {
public:
    MpfWrapper(mpf_ptr other);

    MpfWrapper(const MpfWrapper& other);

    MpfWrapper& operator=(const MpfWrapper& other);

    ~MpfWrapper();

    void PrintExp(std::string& out);

private:
    mpf_t val;

};

}

}