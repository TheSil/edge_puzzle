#include "MpfWrapper.h"

using namespace edge::backtracker;

MpfWrapper::MpfWrapper(mpf_ptr other)
{
    mpf_init(val);
    mpf_set(val, other);
}

MpfWrapper::MpfWrapper(const MpfWrapper& other)
{
    mpf_init(val);
    mpf_set(val, other.val);
}

MpfWrapper& MpfWrapper::operator=(const MpfWrapper& other)
{
    if (this != &other)
    {
        mpf_init(val);
        mpf_set(val, other.val);
    }
    return *this;
}

MpfWrapper::~MpfWrapper()
{
    mpf_clear(val);
}

void MpfWrapper::PrintExp(std::string& out)
{
    char buf[128];
    gmp_sprintf(buf, "%.2FE", val);
    out = buf;
}
