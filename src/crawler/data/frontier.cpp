#include "frontier.hpp"
#include <functional>

namespace scam::crawler
{
    // Frontier constructor.
    frontier::frontier(const std::initializer_list<std::string>& il) noexcept
    {
        for (auto& el : il)
        {
            this->urls.push(el);
        }
    }

    // Frontier destructor.
    frontier::~frontier() {}

    // Checks whether URL already exists in queue.
    bool frontier::url_exists(const std::string& url) noexcept
    {
        for (std::set<std::string>::iterator it = this->seen_urls.begin(); it != this->seen_urls.end(); it++)
        {
            if (url.compare(*it) == 0)
                return true;
        }

        return false;
    }

    // Adds URL if not already added.
    void frontier::add_url(const std::string& url) noexcept
    {
        this->mtx.lock();

        if (!url_exists(url))
        {
            this->seen_urls.insert(url);
            this->urls.push(url);
        }

        this->mtx.unlock();
    }

    // Returns next URL in line and removes it.
    std::string frontier::get_next()
    {
        this->mtx.lock();

        if (empty())
        {
            this->mtx.unlock();
            return "";
        }

        std::string next = this->urls.front();
        this->urls.pop();
        this->mtx.unlock();

        return next;
    }

    // Tells wether frontier is empty.
    bool frontier::empty() const noexcept
    {
        return this->urls.empty();
    }

    // Returns size of frontier.
    size_t frontier::size() const noexcept
    {
        return this->urls.size();
    }

    // Mercator non-class member prototypes.
    static inline unsigned new_back_index(unsigned start_index, const std::vector<std::queue<std::string>>& back_queue);

    // Mercator constructor.
    mercator::mercator(unsigned short prio_depth, unsigned short back_size) noexcept
    {
        this->mtx.lock();
        
        for (int i = 0; i < prio_depth; i++)
        {
            this->front_queue.push_back(std::queue<std::string>());
        }

        for (int i = 0; i < back_size; i++)
        {
            this->back_queue.push_back(std::queue<std::string>());
        }

        this->mtx.unlock();
    }

    // Mercator constructor.
    mercator::mercator(std::initializer_list<std::pair<std::string, unsigned>>& il, unsigned short prio_depth, unsigned short back_size) throw()
        : mercator(prio_depth, back_size)
    {
        this->mtx.lock();
        
        for (std::initializer_list<std::pair<std::string, unsigned>>::iterator it = il.begin(); it != il.end(); it++)
        {
            if (it->second < 0 || it->second >= this->front_queue.size())
            {
                this->mtx.unlock();
                throw priority_exception();
            }

            this->front_queue[it->second].push(it->first);
        }

        this->mtx.unlock();
    }

    // Add url into front queue.
    void mercator::add_url(const std::string& url, unsigned short priority) throw()
    {
        this->mtx.lock();
        
        if (priority < 0 || priority >= this->front_queue.size())
        {
            this->mtx.unlock();
            throw priority_exception();
        }

        if (!url_seen(url))
        {
            this->urls.insert(url);
            this->front_queue[priority].push(url);
        }

        this->mtx.unlock();
    }

    // Checks for frontier being empty.
    bool mercator::empty() const noexcept
    {
        unsigned front_size = this->front_queue.size(), back_size = this->back_queue.size();

        for (unsigned i = 0; i < front_size; i++)
        {
            if (!this->front_queue[i].empty())
                return false;
        }

        for (unsigned i = 0; i < back_size; i++)
        {
            if (!this->back_queue[i].empty())
                return false;
        }

        return true;
    }

    // Gets next URL from back queue. Fills up a back queue from front queue if empty.
    std::string mercator::get_next()
    {
        if (empty())
            return "";
        
        this->mtx.lock();
        static unsigned index = 0;
        
        if (this->back_queue[index].empty() && !front_queue_empty())
            fill_back_queue(index);

        else if (this->back_queue[index].empty() && front_queue_empty())
            index = new_back_index(index, this->back_queue);

        std::string next = this->back_queue[index].front();
        this->back_queue[index].pop();
        index = (index + 1) % this->back_queue.size();
        this->mtx.unlock();

        return next;
    }

    // Fills up back queue from front queue with highest priority.
    // Must not be called if front queue is entirely empty.
    void mercator::fill_back_queue(unsigned back_index) noexcept
    {
        unsigned h_index = 0;  
        while (this->front_queue[h_index].empty())
        {
            h_index++;
        }

        while (!this->front_queue[h_index].empty())
        {
            this->back_queue[back_index].push(this->front_queue[h_index].front());
            this->front_queue[h_index].pop();
        }
    }

    // Computes next index in back queue linearly.
    // Will iterate forever if back queue is empty.
    static inline unsigned new_back_index(unsigned start_index, const std::vector<std::queue<std::string>>& back_queue)
    {
        unsigned start = start_index;

        while (back_queue[start].empty())
        {
            start = (start + 1) % back_queue.size();
        }

        return start;
    }

    // Returns size of mercator.
    size_t mercator::size() const noexcept
    {
        unsigned front_size = this->front_queue.size(), back_size = back_queue.size(), size = 0;

        for (int i = 0; i < front_size; i++)
        {
            size += this->front_queue[i].size();
        }

        for (int i = 0; i < back_size; i++)
        {
            size += back_queue[i].size();
        }

        return size;
    }

    // Checks whether the front queue is empty.
    bool mercator::front_queue_empty() const noexcept
    {
        unsigned length = this->front_queue.size();

        for (unsigned i = 0; i < length; i++)
        {
            if (!this->front_queue[i].empty())
                return false;
        }

        return true;
    }

    // Checks if URL has been in mercator aldready.
    bool mercator::url_seen(const std::string& url) const noexcept
    {
        for (std::set<std::string>::iterator it = this->urls.begin(); it != this->urls.end(); it++)
        {
            if (url.compare(*it) == 0)
                return true;
        }

        return false;
    }

    // Overridden exception class.
    const char* priority_exception::what() const noexcept
    {
        return "Bad priority";
    }
}
