
#pragma once

#include <libcommon/Common.h>

#include <unordered_set>
#include <mutex>
#include <thread>

namespace fs 
{
namespace server 
{
typedef struct SignatureSpi {
    char                    key[128];
    struct SignatureSpi    *next;
} SignatureSpi;

class SignatureFilter
{
public:
    SignatureFilter(size_t maxSize) {
        m_head = NULL;
        m_tail = NULL;
        m_maxSize = maxSize;
    }
    ~SignatureFilter() {}

    bool find(std::string const& signature);
    void save(std::string const& signature);

private:
    size_t                            m_maxSize;
    SignatureSpi                     *m_head;
    SignatureSpi                     *m_tail;
    std::mutex                        m_mtx;
    std::unordered_set<std::string>   m_signatureSet;
};
}
}