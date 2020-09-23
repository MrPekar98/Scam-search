#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

#include <string>

namespace scam::crawler
{
    struct document
    {
        std::string url, content;
        unsigned id;

        document(const std::string& url, const std::string& content) noexcept;
        document operator=(const document& doc) noexcept;
    };
}

#endif