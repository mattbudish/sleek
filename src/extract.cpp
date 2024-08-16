/*
 * This file is in the public domain.
 * Use it as you wish.
 */

#include <archive.h>
#include <archive_entry.h>
#include <cstdlib>
#include <cstring>

#include "extract.hpp"

using namespace std;

static void	extract_impl(int fd, int do_extract, int flags, int strip);

void sleek::archive::extract(int fd, int strip)
{
    extract_impl(fd, 1, 0, strip);
}

static void	errmsg(const char *);
static void	fail(const char *, const char *, int);
static int	copy_data(struct archive *, struct archive *);
static void	msg(const char *);
static void	warn(const char *, const char *);
int edit_pathname(int strip, struct archive_entry *entry);

static int verbose = 0;

static void extract_impl(int fd, int do_extract, int flags, int strip)
{
	struct archive *a;
	struct archive *ext;
	struct archive_entry *entry;
	int r;

	a = archive_read_new();
	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	/*
	 * Note: archive_write_disk_set_standard_lookup() is useful
	 * here, but it requires library routines that can add 500k or
	 * more to a static executable.
	 */
	archive_read_support_format_tar(a);
#ifndef NO_BZIP2_EXTRACT
	archive_read_support_filter_bzip2(a);
#endif
#ifndef NO_GZIP_EXTRACT
	archive_read_support_filter_gzip(a);
#endif
#ifndef NO_COMPRESS_EXTRACT
	archive_read_support_filter_compress(a);
#endif
#ifndef NO_TAR_EXTRACT
	archive_read_support_format_tar(a);
#endif
#ifndef NO_CPIO_EXTRACT
	archive_read_support_format_cpio(a);
#endif
#ifndef NO_LOOKUP
	archive_write_disk_set_standard_lookup(ext);
#endif
	if ((r = archive_read_open_fd(a, fd, 10240)))
		fail("archive_read_open_FILE()",
		    archive_error_string(a), r);
	for (int i = 0;;i++) {
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r != ARCHIVE_OK)
			fail("archive_read_next_header()",
			    archive_error_string(a), 1);
		if (i < strip)
			continue;
		edit_pathname(strip, entry);
		if (verbose && do_extract)
			msg("x ");
		if (verbose || !do_extract)
			msg(archive_entry_pathname(entry));
		if (do_extract) {
			r = archive_write_header(ext, entry);
			if (r != ARCHIVE_OK)
				warn("archive_write_header()",
				    archive_error_string(ext));
			else {
				copy_data(a, ext);
				r = archive_write_finish_entry(ext);
				if (r != ARCHIVE_OK)
					fail("archive_write_finish_entry()",
					    archive_error_string(ext), 1);
			}

		}
		if (verbose || !do_extract)
			msg("\n");
	}
	archive_read_close(a);
	archive_read_free(a);
	
	archive_write_close(ext);
  	archive_write_free(ext);
	exit(0);
}

static int
copy_data(struct archive *ar, struct archive *aw)
{
	int r;
	const void *buff;
	size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
	int64_t offset;
#else
	off_t offset;
#endif

	for (;;) {
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);
		if (r != ARCHIVE_OK)
			return (r);
		r = archive_write_data_block(aw, buff, size, offset);
		if (r != ARCHIVE_OK) {
			warn("archive_write_data_block()",
			    archive_error_string(aw));
			return (r);
		}
	}
}

/*
 * These reporting functions use low-level I/O; on some systems, this
 * is a significant code reduction.  Of course, on many server and
 * desktop operating systems, malloc() and even crt rely on printf(),
 * which in turn pulls in most of the rest of stdio, so this is not an
 * optimization at all there.  (If you're going to pay 100k or more
 * for printf() anyway, you may as well use it!)
 */
static void
msg(const char *m)
{
	write(1, m, strlen(m));
}

static void
errmsg(const char *m)
{
	write(2, m, strlen(m));
}

static void
warn(const char *f, const char *m)
{
	errmsg(f);
	errmsg(" failed: ");
	errmsg(m);
	errmsg("\n");
}

static void
fail(const char *f, const char *m, int r)
{
	warn(f, m);
	exit(r);
}

static const char *
strip_components(const char *p, int elements)
{
	/* Skip as many elements as necessary. */
	while (elements > 0) {
		switch (*p++) {
		case '/':
#if defined(_WIN32) && !defined(__CYGWIN__)
		case '\\': /* Support \ path sep on Windows ONLY. */
#endif
			elements--;
			break;
		case '\0':
			/* Path is too short, skip it. */
			return (NULL);
		}
	}

	/* Skip any / characters.  This handles short paths that have
	 * additional / termination.  This also handles the case where
	 * the logic above stops in the middle of a duplicate //
	 * sequence (which would otherwise get converted to an
	 * absolute path). */
	for (;;) {
		switch (*p) {
		case '/':
#if defined(_WIN32) && !defined(__CYGWIN__)
		case '\\': /* Support \ path sep on Windows ONLY. */
#endif
			++p;
			break;
		case '\0':
			return (NULL);
		default:
			return (p);
		}
	}
}

int
edit_pathname(int strip, struct archive_entry *entry)
{
	const char *name = archive_entry_pathname(entry);
	const char *original_name = name;
	const char *hardlinkname = archive_entry_hardlink(entry);
	const char *original_hardlinkname = hardlinkname;

	/* Strip leading dir names as per --strip-components option. */
	if (strip > 0) {
		name = strip_components(name, strip);
		if (name == NULL)
			return (1);

		if (hardlinkname != NULL) {
			hardlinkname = strip_components(hardlinkname,
			    strip);
			if (hardlinkname == NULL)
				return (1);
		}
	}

	/* Replace name in archive_entry. */
	if (name != original_name) {
		archive_entry_copy_pathname(entry, name);
	}
	if (hardlinkname != original_hardlinkname) {
		archive_entry_copy_hardlink(entry, hardlinkname);
	}
	return (0);
}