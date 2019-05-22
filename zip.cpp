//
// Created by YuraYeliseev on 2019-04-22.
//

#include "zip.h"

int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    off_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return ARCHIVE_OK;
        if (r < ARCHIVE_OK)
            return r;
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            return r;
        }
    }
}

bool Zip::unzip(std::string zipFile, std::string ExtractPath) {
    boost::filesystem::path zip(zipFile);
//    std::vector<std::string> files;
    if (boost::filesystem::exists(zip)) {
        boost::filesystem::path path(ExtractPath);
        boost::filesystem::path currentDir = boost::filesystem::current_path();
        struct archive *a;
        struct archive *ext;
        struct archive_entry *entry;
        int flags = ARCHIVE_EXTRACT_TIME;
        int r;

        a = archive_read_new();
        archive_read_support_format_all(a);
        ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, flags);
        archive_write_disk_set_standard_lookup(ext);
        boost::filesystem::current_path(zip.parent_path());
        if ((r = archive_read_open_filename(a, zip.filename().c_str(), 10240))) {
            if (boost::filesystem::extension(zip.filename()) == ".txt") {
                boost::filesystem::copy_file(zip, path.string() + zip.filename().string(),
                                             boost::filesystem::copy_option::overwrite_if_exists);
                boost::filesystem::current_path(currentDir);
                return true;
            }

            throw std::runtime_error("Can`t open this type of archive");
        }
        for (;;) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF)
                break;
            if (r < ARCHIVE_OK || r < ARCHIVE_WARN)
                return false;
            if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path)) {
                std::string newPath = path.string() + "/" + std::string(archive_entry_pathname(entry));
                archive_entry_set_pathname(entry, newPath.c_str());
//                files.push_back(newPath);
            }
            r = archive_write_header(ext, entry);
            if (r < ARCHIVE_OK)
                return false;
            else if (archive_entry_size(entry) > 0) {
                r = copy_data(a, ext);
                if (r < ARCHIVE_OK || r < ARCHIVE_WARN)
                    return false;
            }
            r = archive_write_finish_entry(ext);
            if (r < ARCHIVE_OK || r < ARCHIVE_WARN)
                return false;
        }
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
        boost::filesystem::current_path(currentDir);
        return true;
    }
    throw std::invalid_argument("There is no path for the file to unzip!");
}

std::vector<std::string> Zip::list_items(const char *file_path) {
    std::vector<std::string> archive_files;
    struct archive *a;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    archive_read_support_format_all(a);
    r = archive_read_open_filename(a, file_path, 10240);
    if (r != ARCHIVE_OK)
        throw std::invalid_argument("No such file or bad extension");
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        archive_files.emplace_back(archive_entry_pathname(entry));
        archive_read_data_skip(a);
    }
    r = archive_read_free(a);
    if (r != ARCHIVE_OK)
        throw std::runtime_error("Some this happen to archive");
    return archive_files;
}