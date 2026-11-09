#pragma once
#include <cstddef>
#include <deque>
#include <string>
class DestiniaDiagnosticLog {
    std::size_t limit_;
    std::deque<std::string> entries_;
public:
    DestiniaDiagnosticLog() : limit_(400) {}
    explicit DestiniaDiagnosticLog(std::size_t limit) : limit_(limit) {}
    void add(const std::string &entry) {
        if (!limit_) return;
        if (entries_.size() == limit_) entries_.pop_front();
        entries_.push_back(entry);
    }
    std::string text() const {
        std::string result;
        for (const auto &entry : entries_) { result += entry; result += '\n'; }
        return result;
    }
};
