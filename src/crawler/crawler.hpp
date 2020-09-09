#include <vector>
#include <string>
#include "data/document.hpp"

#define USER_AGENT "scam agent"

namespace scam::crawler
{
    // Prototypes.
    void crawl(const std::vector<std::string>& urls, std::vector<document>& result_documents);
    std::vector<document> crawl(const std::vector<std::string>& urls, unsigned long doc_limit);
}