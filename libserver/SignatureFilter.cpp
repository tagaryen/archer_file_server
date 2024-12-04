#include "SignatureFilter.h"

using namespace fs::server;

void SignatureFilter::save(std::string const& signature) {
    SignatureSpi *spi = (SignatureSpi *)calloc(1, sizeof(SignatureSpi));
    memcpy(spi->key, signature.c_str(), 128);
    
    std::unique_lock<std::mutex> lock(m_mtx);
    m_signatureSet.insert(signature);
    if(!m_head) {
        m_head = m_tail = spi;
    } else {
        m_tail->next = spi;
        m_tail = m_tail->next;
    }
    if(m_signatureSet.size() > m_maxSize) {
        std::string oldKey(m_head->key, 256);
        m_signatureSet.erase(oldKey);
        SignatureSpi *head = m_head;
        m_head = m_head->next;
        free(head);
    }
    lock.unlock();
}

bool SignatureFilter::find(std::string const& signature) {
    return m_signatureSet.find(signature) != m_signatureSet.end();
}