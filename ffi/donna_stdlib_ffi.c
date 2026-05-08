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
#include <dirent.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <time.h>

long donna_ffi_time_now_ms(void);
long donna_ffi_time_now_us(void);
const char *donna_rt_getenv(const char *name);
void donna_rt_flush(void);
char *donna_float_to_string(double d);
double donna_float_parse(const char *s);
double donna_float_min(double a, double b);
double donna_float_max(double a, double b);
double donna_float_clamp(double x, double lo, double hi);
long donna_float_is_positive(double x);
long donna_float_is_negative(double x);
long donna_float_is_zero(double x);
char *donna_string_char_from_code(long code);
int donna_ffi_shell_exec(const char *cmd);
char *donna_ffi_shell_capture(const char *cmd);
char *donna_ffi_file_read(const char *path);
int donna_ffi_file_write(const char *path, const char *content);
int donna_ffi_file_append(const char *path, const char *content);
int donna_ffi_file_exists(const char *path);
int donna_ffi_file_is_file(const char *path);
int donna_ffi_file_is_dir(const char *path);
int donna_ffi_file_mkdir(const char *path);
int donna_ffi_file_delete(const char *path);
int donna_ffi_file_copy(const char *src, const char *dst);
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
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000L + (long)(ts.tv_nsec / 1000000L);
}

long donna_ffi_time_now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000000L + (long)(ts.tv_nsec / 1000L);
}

/* Environment */

const char* donna_rt_getenv(const char* name) {
    const char* v = getenv(name);
    return v ? v : "";
}

void donna_rt_flush(void) {
    fflush(stdout);
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

/* String helpers */

/* Create a 1-byte string from a character code (byte value). */
char *donna_string_char_from_code(long code) {
    char *s = malloc(2);
    s[0] = (char)(code & 0xFF);
    s[1] = '\0';
    return s;
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

/* Delete a file or empty directory. Returns 0 on success, -1 on error. */
int donna_ffi_file_delete(const char *path) {
    return remove(path);
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
