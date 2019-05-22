#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <functional>

#include <QFutureSynchronizer>
#include <QtConcurrent/qtconcurrentmap.h>
#include "boost/locale/boundary.hpp"
#include "boost/locale.hpp"
#include "boost/filesystem.hpp"

#include "utils.h"
#include "config_parser.h"

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

wMap same(wMap i) {
    return i;
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

int main(int argc, char *argv[]) {
    Attributes *a;
    if (argc < 2)
        a = get_intArgs("../config.dat");
    else if (argc == 2) {
        a = get_intArgs(argv[1]);
    } else {
        std::cout << "Invalid input: ./multiple_threads <path to config>" << std::endl;
    }

    std::string dir = "../.tmp/";
    boost::filesystem::create_directory(dir);
    boost::filesystem::path currentDir = boost::filesystem::current_path();
//    unzip_files(boost::filesystem::canonical(dir).string() + "/", boost::filesystem::canonical(a->infile).string());
    boost::filesystem::current_path(currentDir);
    std::cout << "Started counting words!" << std::endl;
    boost::filesystem::recursive_directory_iterator it(dir), end;

    auto start_counting = get_current_wall_time_fenced();
    std::array<std::vector<std::vector<std::vector<std::string>>>, 2> word_blocks_vecs;
    int curr = 0;
    std::array<QFutureSynchronizer<wMap>, 2> syncs;
    for (auto &entry: boost::make_iterator_range(it, end)) {
        std::string previous = entry.path().string();
        std::string data = check_input(previous);
        word_blocks_vecs[curr].emplace_back(split_to_words(data));
        sync.addFuture(
                QtConcurrent::mappedReduced(word_blocks_vecs[curr][word_blocks_vecs[curr].size() - 1], count_words, merge));
    }

    auto res = QtConcurrent::blockingMappedReduced(sync.futures(),
                                                   std::function<wMap(QFuture<wMap>)>([](QFuture<wMap> prom) {
                                                       return prom.result();
                                                   }), merge);

    auto end_counting = get_current_wall_time_fenced();

    std::vector<std::pair<std::string, size_t>> tmp;
    tmp.reserve(res.size());
    for (auto x: res) {
        tmp.emplace_back(x.first, x.second);
    }
    std::sort(tmp.begin(), tmp.end(), [](auto x, auto y) {
        return x.first < y.first;
    });
    std::ofstream fout1(a->out_by_a);
    for (auto x: tmp) {
        fout1 << x.first << "\t:\t" << x.second << std::endl;
    }
    std::sort(tmp.begin(), tmp.end(), [](auto x, auto y) {
        return x.second > y.second;
    });
    std::ofstream fout2(a->out_by_n);
    for (auto x: tmp) {
        fout2 << x.first << "\t:\t" << x.second << std::endl;
    }

    std::cout << "Counting: " << to_us(end_counting - start_counting) << std::endl;
}
