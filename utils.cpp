#include "utils.h"

std::string get_file_ext(std::string file_name) {
    std::vector<std::string> file_and_ext;
    std::string extension;
    boost::split(file_and_ext, file_name, boost::is_any_of("."));
    return file_and_ext[file_and_ext.size() - 1];
}

std::string check_input(const std::string &file_path) {
    std::string my_txt;
    boost::filesystem::path input(file_path);
    if (boost::filesystem::exists(input)) {
        std::string extension = get_file_ext(file_path);
        if (extension == "txt") {
            my_txt = file_path;
        } else if (extension == "zip") {
            Zip::unzip(file_path, "../");
            auto arch_items = Zip::list_items(file_path.c_str());
            for (const auto &item: arch_items)
                if (get_file_ext(item) == "txt") {
                    my_txt = "../" + item;
                    break;
                }
        }
        std::ifstream fin(my_txt, std::ifstream::binary);
        std::string data = static_cast<std::ostringstream >(std::ostringstream{} << fin.rdbuf()).str();
        return data;
    } else {
        throw std::runtime_error("No file for input!");
    }
}

void unzip_files(std::string dst, std::string src) {
    boost::filesystem::path dir = src;
    boost::filesystem::directory_iterator it(dir), end;
    std::vector<std::string> files;
    for (auto &entry: boost::make_iterator_range(it, end)) {
        if(boost::filesystem::is_directory(entry.path())){
            unzip_files(boost::filesystem::canonical(dst).string() + "/", boost::filesystem::canonical(entry.path().string()).string());
        }
        else {
            std::string previous = entry.path().string();
            try {
                Zip::unzip(previous, dst);
            }
            catch (std::exception &e) {
            }
        }
    }
}
