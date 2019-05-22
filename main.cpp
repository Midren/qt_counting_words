#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <QtConcurrent/qtconcurrentmap.h>
#include "boost/locale/boundary.hpp"
#include "boost/locale.hpp"

#define MAP 1
#ifdef MAP
typedef std::map<std::string, size_t> wMap;
#else
typedef std::unordered_map<std::string, size_t> wMap;
#endif

namespace ba=boost::locale::boundary;

void add(int &x, int y) {
    x += y;
}

std::vector<std::vector<std::string>> split_to_words(std::string &data) {
    boost::locale::generator gen;
    std::locale::global(gen("en_US.UTF-8"));

    std::vector<std::vector<std::string>> words_block_vec;
    std::vector<std::string> words_vec;
    ba::ssegment_index words(ba::word, data.begin(), data.end());

    constexpr int words_block = 2048;

    for (auto &&x: words) {
        words_vec.push_back(move(x.str()));
        if (words_vec.size() == words_block) {
            words_block_vec.push_back(std::move(words_vec));
            words_vec = std::vector<std::string>();
            words_vec.reserve(words_block);
        }
    }
    words_block_vec.push_back(std::move(words_vec));
    return words_block_vec;
}

bool any_str(const std::string &str) {
    size_t num = 0;
    for (auto c : str)
        num += isalpha(c);
    return num > 0;
}

wMap count_words(std::vector<std::string> words) {
    wMap cnt;
    for (auto word: words) {
        std::string s = boost::locale::fold_case(boost::locale::normalize(word));
        if (s.length() >= 1 && any_str(s)) {
            ++cnt[s];
        }
    }
    return cnt;
}

void merge(wMap &result, wMap map) {
    for (auto &el : map) {
        result[el.first] += el.second;
    }
}

#if defined _WIN32
using best_resolution_cpp_clock = std::chrono::steady_clock;
#else
using best_resolution_cpp_clock = std::chrono::high_resolution_clock;
#endif

inline best_resolution_cpp_clock::time_point get_current_wall_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = best_resolution_cpp_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline uint64_t to_us(const D &d) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
}

int main() {
    auto start_counting = get_current_wall_time_fenced();
    std::vector<wMap> map_vec;

    std::string word;
    std::ifstream fin("mama.txt");
    std::string data = static_cast<std::ostringstream &>(std::ostringstream{} << fin.rdbuf()).str();
    auto words_blocks = split_to_words(data);

    auto new_vec = QtConcurrent::mappedReduced(words_blocks, count_words, merge);
    new_vec.waitForFinished();
    map_vec.push_back(*new_vec.begin());

    auto end_counting = get_current_wall_time_fenced();
    std::cout << "Total: " << to_us(end_counting - start_counting) << std::endl;
    return 0;
}