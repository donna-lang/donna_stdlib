/*
 * donna_stdlib_ffi.c - C helpers for the Donna bootstrap stdlib.
 *
 * Provides donna/shell, donna/files, and time operations that require OS access.
 * Ships as part of the donna_stdlib package (compiled from ffi/ directory).
 *
 * Functions use the donna_ffi_ prefix to avoid colliding with the QBE-generated
 * symbols produced by the donna/shell and donna/files modules (which get the
 * module prefixes donna_shell_ and donna_files_ respectively).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#endif
#include <time.h>

long donna_ffi_time_now_ms(void);
long donna_ffi_time_now_us(void);
long donna_ffi_time_unix_seconds(void);
long donna_ffi_time_unix_ms(void);
void donna_ffi_time_sleep_ms(long ms);
const char *donna_rt_getenv(const char *name);
void donna_rt_flush(void);
void donna_ffi_io_print(const char *s);
void donna_ffi_io_eprint(const char *s);
void donna_ffi_io_flush_stderr(void);
char *donna_ffi_io_read_line(void);
char *donna_ffi_io_read_all_stdin(void);
char *donna_float_to_string(double d);
double donna_float_parse(const char *s);
double donna_float_min(double a, double b);
double donna_float_max(double a, double b);
double donna_float_clamp(double x, double lo, double hi);
long donna_float_is_positive(double x);
long donna_float_is_negative(double x);
long donna_float_is_zero(double x);
long donna_float_equal(double a, double b);
long donna_float_near(double a, double b, double tolerance);
long donna_float_is_nan(double x);
long donna_float_is_infinite(double x);
long donna_float_is_finite(double x);
long donna_float_sign(double x);
char *donna_string_char_from_code(long code);
long donna_ffi_string_index_of(const char *s, const char *sub, long from);
char *donna_ffi_string_replace(const char *s, const char *from, const char *to);
char *donna_ffi_path_separator(void);
int donna_ffi_shell_exec(const char *cmd);
char *donna_ffi_shell_capture(const char *cmd);
char *donna_ffi_file_read(const char *path);
int donna_ffi_file_write(const char *path, const char *content);
int donna_ffi_file_append(const char *path, const char *content);
int donna_ffi_file_exists(const char *path);
int donna_ffi_file_is_file(const char *path);
int donna_ffi_file_is_dir(const char *path);
int donna_ffi_file_mkdir(const char *path);
int donna_ffi_file_mkdir_all(const char *path);
int donna_ffi_file_delete(const char *path);
int donna_ffi_file_remove_all(const char *path);
int donna_ffi_file_copy(const char *src, const char *dst);
int donna_ffi_file_rename(const char *src, const char *dst);
char *donna_ffi_file_list_dir(const char *path);
__attribute__((weak)) long donna_program_main(void);

#ifdef _WIN32
char *strndup(const char *s, size_t n) {
    size_t len = 0;
    while (len < n && s[len] != '\0') len++;
    char *out = malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}
#endif

/* Time */

long donna_ffi_time_now_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (long)((counter.QuadPart * 1000LL) / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000L + (long)(ts.tv_nsec / 1000000L);
#endif
}

long donna_ffi_time_now_us(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (long)((counter.QuadPart * 1000000LL) / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000000L + (long)(ts.tv_nsec / 1000L);
#endif
}

long donna_ffi_time_unix_seconds(void) {
    return (long)time(NULL);
}

long donna_ffi_time_unix_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    ULARGE_INTEGER uli;
    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (long)((uli.QuadPart - 116444736000000000ULL) / 10000ULL);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long)tv.tv_sec * 1000L + (long)(tv.tv_usec / 1000L);
#endif
}

void donna_ffi_time_sleep_ms(long ms) {
    if (ms <= 0) return;
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    struct timespec req;
    req.tv_sec = ms / 1000L;
    req.tv_nsec = (ms % 1000L) * 1000000L;
    while (nanosleep(&req, &req) == -1 && errno == EINTR) {}
#endif
}

/* Environment */

const char* donna_rt_getenv(const char* name) {
    const char* v = getenv(name);
    return v ? v : "";
}

void donna_rt_flush(void) {
    fflush(stdout);
}

/* IO helpers */

void donna_ffi_io_print(const char *s) {
    if (!s) return;
    fputs(s, stdout);
}

void donna_ffi_io_eprint(const char *s) {
    if (!s) return;
    fputs(s, stderr);
}

void donna_ffi_io_flush_stderr(void) {
    fflush(stderr);
}

char *donna_ffi_io_read_line(void) {
    size_t cap = 128;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) return strdup("");

    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        if (len + 1 >= cap) {
            cap *= 2;
            char *new_buf = realloc(buf, cap);
            if (!new_buf) { free(buf); return strdup(""); }
            buf = new_buf;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    return buf;
}

char *donna_ffi_io_read_all_stdin(void) {
    size_t cap = 4096;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) return strdup("");

    int c;
    while ((c = getchar()) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char *new_buf = realloc(buf, cap);
            if (!new_buf) { free(buf); return strdup(""); }
            buf = new_buf;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    return buf;
}

/* Float helpers */

/* Convert a double to its string representation (malloc'd). */
char *donna_float_to_string(double d) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", d);
    return strdup(buf);
}

/* Parse a float string. Returns 0.0 on invalid input. */
double donna_float_parse(const char *s) {
    return atof(s);
}

double donna_float_min(double a, double b) { return a < b ? a : b; }
double donna_float_max(double a, double b) { return a > b ? a : b; }
double donna_float_clamp(double x, double lo, double hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}
long donna_float_is_positive(double x) { return x > 0.0 ? 1 : 0; }
long donna_float_is_negative(double x) { return x < 0.0 ? 1 : 0; }
long donna_float_is_zero(double x) { return x == 0.0 ? 1 : 0; }
long donna_float_equal(double a, double b) { return a == b ? 1 : 0; }
long donna_float_near(double a, double b, double tolerance) {
    if (tolerance < 0.0) return 0;
    if (isnan(a) || isnan(b)) return 0;
    return fabs(a - b) <= tolerance ? 1 : 0;
}
long donna_float_is_nan(double x) { return isnan(x) ? 1 : 0; }
long donna_float_is_infinite(double x) { return isinf(x) ? 1 : 0; }
long donna_float_is_finite(double x) { return isfinite(x) ? 1 : 0; }
long donna_float_sign(double x) {
    if (x > 0.0) return 1;
    if (x < 0.0) return -1;
    return 0;
}

/* String helpers */

/* Create a 1-byte string from a character code (byte value). */
char *donna_string_char_from_code(long code) {
    char *s = malloc(2);
    s[0] = (char)(code & 0xFF);
    s[1] = '\0';
    return s;
}

long donna_ffi_string_index_of(const char *s, const char *sub, long from) {
    if (!s || !sub) return -1;

    size_t slen = strlen(s);
    size_t sublen = strlen(sub);
    if (from < 0) from = 0;
    if ((size_t)from > slen) return -1;
    if (sublen == 0) return from;
    if (sublen > slen) return -1;

    for (size_t i = (size_t)from; i + sublen <= slen; i++) {
        if (memcmp(s + i, sub, sublen) == 0) return (long)i;
    }
    return -1;
}

char *donna_ffi_string_replace(const char *s, const char *from, const char *to) {
    if (!s) return strdup("");
    if (!from || !to) return strdup(s);

    size_t slen = strlen(s);
    size_t flen = strlen(from);
    size_t tlen = strlen(to);
    if (flen == 0) return strdup(s);

    size_t count = 0;
    for (size_t i = 0; i + flen <= slen;) {
        if (memcmp(s + i, from, flen) == 0) {
            count++;
            i += flen;
        } else {
            i++;
        }
    }
    if (count == 0) return strdup(s);

    size_t out_len = slen;
    if (tlen >= flen) {
        out_len += count * (tlen - flen);
    } else {
        out_len -= count * (flen - tlen);
    }
    char *out = malloc(out_len + 1);
    if (!out) return strdup("");

    size_t i = 0;
    size_t j = 0;
    while (i < slen) {
        if (i + flen <= slen && memcmp(s + i, from, flen) == 0) {
            memcpy(out + j, to, tlen);
            i += flen;
            j += tlen;
        } else {
            out[j++] = s[i++];
        }
    }
    out[j] = '\0';
    return out;
}

/* Path helpers */

char *donna_ffi_path_separator(void) {
#ifdef _WIN32
    return strdup("\\");
#else
    return strdup("/");
#endif
}

/* Shell helpers */

int donna_ffi_shell_exec(const char *cmd) {
    int status = system(cmd);
    if (status == -1) return -1;
#if defined(WEXITSTATUS)
    if (WIFEXITED(status)) return WEXITSTATUS(status);
#endif
    return status;
}

/* Capture stdout of a shell command into a malloc'd string.
 * Trailing newline is stripped (matches shell.capture semantics). */
char *donna_ffi_shell_capture(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return strdup("");

    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) { pclose(fp); return strdup(""); }

    char tmp[256];
    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t n = strlen(tmp);
        if (len + n + 1 > cap) {
            cap = (cap + n + 1) * 2;
            char *newbuf = realloc(buf, cap);
            if (!newbuf) { free(buf); pclose(fp); return strdup(""); }
            buf = newbuf;
        }
        memcpy(buf + len, tmp, n);
        len += n;
    }
    pclose(fp);

    /* Strip single trailing newline */
    if (len > 0 && buf[len - 1] == '\n') len--;
    buf[len] = '\0';
    return buf;
}

/* File helpers */

/* Read entire file into a malloc'd string. Returns "" on error. */
char *donna_ffi_file_read(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return strdup("");

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz < 0) { fclose(fp); return strdup(""); }

    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(fp); return strdup(""); }
    fread(buf, 1, (size_t)sz, fp);
    buf[sz] = '\0';
    fclose(fp);
    return buf;
}

/* Write content to file (overwrite). Returns 0 on success, -1 on error. */
int donna_ffi_file_write(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (!fp) return -1;
    fputs(content, fp);
    fclose(fp);
    return 0;
}

/* Append content to file. Returns 0 on success, -1 on error. */
int donna_ffi_file_append(const char *path, const char *content) {
    FILE *fp = fopen(path, "a");
    if (!fp) return -1;
    fputs(content, fp);
    fclose(fp);
    return 0;
}

/* Returns 1 if path exists, 0 otherwise. */
int donna_ffi_file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 ? 1 : 0;
}

/* Returns 1 if path is a regular file, 0 otherwise. */
int donna_ffi_file_is_file(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
}

/* Returns 1 if path is a directory, 0 otherwise. */
int donna_ffi_file_is_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode) ? 1 : 0;
}

/* Create directory with mode 0755. Returns 0 on success, -1 on error. */
int donna_ffi_file_mkdir(const char *path) {
#ifdef _WIN32
    return mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

int donna_ffi_file_mkdir_all(const char *path) {
    if (!path || path[0] == '\0') return -1;
    if (donna_ffi_file_is_dir(path)) return 0;

    char *tmp = strdup(path);
    if (!tmp) return -1;
    size_t len = strlen(tmp);
    while (len > 1 && (tmp[len - 1] == '/' || tmp[len - 1] == '\\')) {
        tmp[--len] = '\0';
    }

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char old = *p;
            *p = '\0';
#ifdef _WIN32
            if (strlen(tmp) == 2 && tmp[1] == ':') {
                *p = old;
                continue;
            }
#endif
            if (tmp[0] != '\0' && !donna_ffi_file_is_dir(tmp)) {
                if (donna_ffi_file_mkdir(tmp) != 0) {
                    free(tmp);
                    return -1;
                }
            }
            *p = old;
        }
    }

    int rc = 0;
    if (!donna_ffi_file_is_dir(tmp)) rc = donna_ffi_file_mkdir(tmp);
    free(tmp);
    return rc;
}

/* Delete a file or empty directory. Returns 0 on success, -1 on error. */
int donna_ffi_file_delete(const char *path) {
    return remove(path);
}

int donna_ffi_file_remove_all(const char *path) {
    if (!path || path[0] == '\0') return -1;
    if (!donna_ffi_file_exists(path)) return 0;
    if (donna_ffi_file_is_file(path)) return remove(path);

#ifdef _WIN32
    WIN32_FIND_DATAA data;
    size_t path_len = strlen(path);
    char *pattern = malloc(path_len + 3);
    if (!pattern) return -1;
    snprintf(pattern, path_len + 3, "%s\\*", path);
    HANDLE h = FindFirstFileA(pattern, &data);
    free(pattern);
    if (h == INVALID_HANDLE_VALUE) return _rmdir(path);
    do {
        const char *name = data.cFileName;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        char sep = '\\';
        char *child = malloc(strlen(path) + strlen(name) + 2);
        if (!child) { FindClose(h); return -1; }
        sprintf(child, "%s%c%s", path, sep, name);
        if (donna_ffi_file_remove_all(child) != 0) {
            free(child);
            FindClose(h);
            return -1;
        }
        free(child);
    } while (FindNextFileA(h, &data));
    FindClose(h);
    return _rmdir(path);
#else
    DIR *dp = opendir(path);
    if (!dp) return remove(path);
    struct dirent *ent;
    while ((ent = readdir(dp))) {
        const char *name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        char *child = malloc(strlen(path) + strlen(name) + 2);
        if (!child) { closedir(dp); return -1; }
        sprintf(child, "%s/%s", path, name);
        if (donna_ffi_file_remove_all(child) != 0) {
            free(child);
            closedir(dp);
            return -1;
        }
        free(child);
    }
    closedir(dp);
    return rmdir(path);
#endif
}

/* Copy file src to dst. Returns 0 on success, -1 on error. */
int donna_ffi_file_copy(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);

    fclose(in);
    fclose(out);
    return 0;
}

int donna_ffi_file_rename(const char *src, const char *dst) {
    return rename(src, dst);
}

/* List directory entries as a newline-separated string (malloc'd).
 * Skips "." and "..". Returns "" on error or empty dir. */
char *donna_ffi_file_list_dir(const char *path) {
    DIR *dp = opendir(path);
    if (!dp) return strdup("");

    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) { closedir(dp); return strdup(""); }

    struct dirent *ent;
    while ((ent = readdir(dp))) {
        const char *name = ent->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
        size_t n = strlen(name);
        if (len + n + 2 > cap) {
            cap = (cap + n + 2) * 2;
            char *newbuf = realloc(buf, cap);
            if (!newbuf) { free(buf); closedir(dp); return strdup(""); }
            buf = newbuf;
        }
        memcpy(buf + len, name, n);
        len += n;
        buf[len++] = '\n';
    }
    closedir(dp);

    /* Strip trailing newline */
    if (len > 0 && buf[len - 1] == '\n') len--;
    buf[len] = '\0';
    return buf;
}

/* Fallback entry point for projects that don't use argparse.
 * argv.c (argparse package) provides a strong main() that overrides this.
 * A weak donna_program_main keeps library-only projects linkable. */
__attribute__((weak)) long donna_program_main(void) {
    return 0;
}

__attribute__((weak)) int main(void) {
    return (int)donna_program_main();
}
