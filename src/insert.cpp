#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>

#include "insert.hpp"

using namespace std;

void sleek::archive::insert(const string &outname, const vector<string> &inFiles)
{
    struct archive *a;
    struct archive_entry *entry;
    struct stat st;
    char buff[8192];
    int len;
    int fd;

    a = archive_write_new();
    archive_write_add_filter_gzip(a);
    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, outname.c_str());
    entry = archive_entry_new(); 

    for (const auto &filename : inFiles)
    {
        stat(filename.c_str(), &st);
        archive_entry_set_pathname(entry, filename.c_str());
        archive_entry_set_size(entry, st.st_size);
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, st.st_mode);
        archive_write_header(a, entry);
        fd = open(filename.c_str(), O_RDONLY);
        len = read(fd, buff, sizeof(buff));
        while ( len > 0 ) {
            archive_write_data(a, buff, len);
            len = read(fd, buff, sizeof(buff));
        }
        close(fd);
        archive_entry_clear(entry);
    }

    archive_entry_free(entry);
    archive_write_close(a);
    archive_write_free(a);
}