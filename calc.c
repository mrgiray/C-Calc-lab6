// Artun Giray OZTEKIN 251ADB223
// https://github.com/mrgiray/C-Calc-lab6
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: ChatGPT, W3Schools, GeeksforGeeks, PDFs and friends.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>

#define STUDENT_NAME "artun"
#define STUDENT_LASTNAME "oztekin"
#define STUDENT_ID "251ADB223"


void skip_spaces(const char **p)
{
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}


int parse_expression(const char **p, const char *line,
                     long long *value, size_t *error_offset);


int parse_number(const char **p, const char *line,
                 long long *value, size_t *error_offset)
{
    skip_spaces(p);

    if (!isdigit((unsigned char)**p)) {
        *error_offset = (size_t)(*p - line);
        return -1;
    }

    long long number = 0;

    while (isdigit((unsigned char)**p)) {
        number = number * 10 + (**p - '0');
        (*p)++;
    }

    *value = number;
    return 0;
}


int parse_primary(const char **p, const char *line,
                  long long *value, size_t *error_offset)
{
    skip_spaces(p);

    if (isdigit((unsigned char)**p)) {
        return parse_number(p, line, value, error_offset);
    }

    if (**p == '(') {
        (*p)++;

        if (parse_expression(p, line, value, error_offset) != 0) {
            return -1;
        }

        skip_spaces(p);

        if (**p != ')') {
            *error_offset = (size_t)(*p - line);
            return -1;
        }

        (*p)++;
        return 0;
    }

    *error_offset = (size_t)(*p - line);
    return -1;
}


long long integer_power(long long base, long long exponent)
{
    long long result = 1;

    while (exponent > 0) {
        result *= base;
        exponent--;
    }

    return result;
}


int parse_power(const char **p, const char *line,
                long long *value, size_t *error_offset)
{
    if (parse_primary(p, line, value, error_offset) != 0) {
        return -1;
    }

    skip_spaces(p);

    if (**p == '*' && *(*p + 1) == '*') {
        (*p) += 2;

        skip_spaces(p);

        size_t exponent_start = (size_t)(*p - line);

        long long exponent;

        if (parse_power(p, line, &exponent, error_offset) != 0) {
            return -1;
        }

        if (exponent < 0) {
            *error_offset = exponent_start;
            return -1;
        }

        *value = integer_power(*value, exponent);
    }

    return 0;
}


int parse_term(const char **p, const char *line,
               long long *value, size_t *error_offset)
{
    if (parse_power(p, line, value, error_offset) != 0) {
        return -1;
    }

    while (1) {
        skip_spaces(p);

        if (**p != '*' && **p != '/') {
            return 0;
        }

        char op = **p;
        (*p)++;

        skip_spaces(p);

        size_t right_operand = (size_t)(*p - line);

        long long number;

        if (parse_power(p, line, &number, error_offset) != 0) {
            return -1;
        }

        if (op == '/' && number == 0) {
            *error_offset = right_operand;
            return -1;
        }

        if (op == '*') {
            *value *= number;
        }
        else {
            *value /= number;
        }
    }
}


int parse_expression(const char **p, const char *line,
                     long long *value, size_t *error_offset)
{
    long long result;

    if (parse_term(p, line, &result, error_offset) != 0) {
        return -1;
    }

    while (1) {
        skip_spaces(p);

        if (**p == ')' || **p == '\n' || **p == '\0') {
            *value = result;
            return 0;
        }

        if (**p != '+' && **p != '-') {
            *error_offset = (size_t)(*p - line);
            return -1;
        }

        char op = **p;
        (*p)++;

        long long number;

        if (parse_term(p, line, &number, error_offset) != 0) {
            return -1;
        }

        if (op == '+') {
            result += number;
        }
        else {
            result -= number;
        }
    }
}


int evaluate_line(const char *line, long long *answer,
                  size_t *error_offset)
{
    const char *p = line;

    if (parse_expression(&p, line, answer, error_offset) != 0) {
        return -1;
    }

    skip_spaces(&p);

    if (*p != '\n' && *p != '\0') {
        *error_offset = (size_t)(p - line);
        return -1;
    }

    return 0;
}


void get_input_base(const char *path, char *base, size_t size)
{
    const char *name = strrchr(path, '/');

    if (name != NULL) {
        name++;
    }
    else {
        name = path;
    }

    snprintf(base, size, "%s", name);

    char *dot = strrchr(base, '.');

    if (dot != NULL && strcmp(dot, ".txt") == 0) {
        *dot = '\0';
    }
}


int line_is_ignored(const char *line)
{
    const char *p = line;

    skip_spaces(&p);

    if (*p == '\n' || *p == '\0' || *p == '#') {
        return 1;
    }

    return 0;
}


int has_txt_extension(const char *name)
{
    const char *dot = strrchr(name, '.');

    return dot != NULL && strcmp(dot, ".txt") == 0;
}


int process_file(const char *input_path, const char *outdir)
{
    FILE *input = fopen(input_path, "r");

    if (input == NULL) {
        perror(input_path);
        return 1;
    }

    mkdir(outdir, 0777);

    char base[256];
    get_input_base(input_path, base, sizeof(base));

    char output_path[1024];

    snprintf(output_path,
             sizeof(output_path),
             "%s/%s_%s_%s_%s.txt",
             outdir,
             base,
             STUDENT_NAME,
             STUDENT_LASTNAME,
             STUDENT_ID);

    FILE *output = fopen(output_path, "w");

    if (output == NULL) {
        perror(output_path);
        fclose(input);
        return 1;
    }

    char line[4096];

    long long file_position = 1;
    long long error_position = 0;
    int has_error = 0;

    while (fgets(line, sizeof(line), input) != NULL) {
        long long result;
        size_t error_offset;

        if (!line_is_ignored(line)) {
            if (evaluate_line(line, &result, &error_offset) != 0) {
                error_position =
                    file_position + (long long)error_offset;

                has_error = 1;
                break;
            }
        }

        file_position += (long long)strlen(line);
    }

    if (has_error) {
        fprintf(output, "ERROR:%lld\n", error_position);
    }
    else {
        rewind(input);

        while (fgets(line, sizeof(line), input) != NULL) {
            long long result;
            size_t error_offset;

            if (line_is_ignored(line)) {
                continue;
            }

            evaluate_line(line, &result, &error_offset);
            fprintf(output, "%lld\n", result);
        }
    }

    fclose(input);
    fclose(output);

    return 0;
}


int scan_directory(const char *dirpath, const char *outdir)
{
    DIR *dir = opendir(dirpath);

    if (dir == NULL) {
        perror(dirpath);
        return 1;
    }

    struct dirent *entry;
    char path[1024];

    while ((entry = readdir(dir)) != NULL) {

        if (!has_txt_extension(entry->d_name)) {
            continue;
        }

        snprintf(path,
                 sizeof(path),
                 "%s/%s",
                 dirpath,
                 entry->d_name);

        struct stat info;

        if (stat(path, &info) != 0) {
            continue;
        }

        if (!S_ISREG(info.st_mode)) {
            continue;
        }

        if (process_file(path, outdir) != 0) {
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

void get_folder_base(const char *path, char *base, size_t size)
{
    size_t len = strlen(path);

    while (len > 0 && path[len - 1] == '/') {
        len--;
    }

    const char *start = path + len;

    while (start > path && *(start - 1) != '/') {
        start--;
    }

    size_t name_len = (size_t)((path + len) - start);

    if (name_len >= size) {
        name_len = size - 1;
    }

    memcpy(base, start, name_len);
    base[name_len] = '\0';
}

int main(int argc, char *argv[])
{
    const char *dirpath = NULL;
    const char *outdir = NULL;
    const char *input_path = NULL;

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-d") == 0 ||
            strcmp(argv[i], "--dir") == 0) {

            if (i + 1 >= argc) {
                fprintf(stderr, "Missing directory after %s\n", argv[i]);
                return 1;
            }

            dirpath = argv[++i];
        }

        else if (strcmp(argv[i], "-o") == 0 ||
                 strcmp(argv[i], "--output-dir") == 0) {

            if (i + 1 >= argc) {
                fprintf(stderr, "Missing output directory after %s\n", argv[i]);
                return 1;
            }

            outdir = argv[++i];
        }

        else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }

        else {
            if (input_path != NULL) {
                fprintf(stderr, "Too many input files\n");
                return 1;
            }

            input_path = argv[i];
        }
    }

    if (dirpath != NULL && input_path != NULL) {
        fprintf(stderr, "Cannot use directory and input file together\n");
        return 1;
    }

    if (dirpath == NULL && input_path == NULL) {
        fprintf(stderr, "No input file or directory given\n");
        return 1;
    }

    char default_outdir[1024];

    if (outdir == NULL) {

        char base[256];

        if (input_path != NULL) {
            get_input_base(input_path, base, sizeof(base));
        }
        else {
            get_folder_base(dirpath, base, sizeof(base));
        }

        snprintf(default_outdir,
                 sizeof(default_outdir),
                 "%s_%s_%s",
                 base,
                 STUDENT_NAME,
                 STUDENT_ID);

        outdir = default_outdir;
    }

    if (dirpath != NULL) {
        return scan_directory(dirpath, outdir);
    }

    return process_file(input_path, outdir);
}