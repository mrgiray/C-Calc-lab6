// Artun Giray OZTEKIN 251ADB223
// https://github.com/mrgiray/C-Calc-lab6
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used:ChatGPT, W3Schools, GeekForGeeks, PDF's and friends.
/* we use this functuon to skip spaces and tabs when we read the expression.
   It moves character by character until there is a real number, operator or another character we care about.
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define STUDENT_NAME "artun"
#define STUDENT_LASTNAME "oztekin"
#define STUDENT_ID "251ADB223"


void skip_spaces(const char **p)
{
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

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

int parse_term(const char **p, const char *line,
               long long *value, size_t *error_offset)
{
    if (parse_number(p, line, value, error_offset) != 0) {
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

        if (parse_number(p, line, &number, error_offset) != 0) {
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


int evaluate_line(const char *line, long long *answer, size_t *error_offset)
{
    const char *p = line;

    long long result;

    if (parse_term(&p, line, &result, error_offset) != 0) {
        return -1;
    }

    while (1) {
        skip_spaces(&p);

        if (*p == '\n' || *p == '\0') {
            *answer = result;
            return 0;
        }

        if (*p != '+' && *p != '-') {
            *error_offset = (size_t)(p - line);
            return -1;
        }

        char op = *p;
        p++;

        long long number;

        if (parse_term(&p, line, &number, error_offset) != 0) {
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

int main(int argc, char *argv[])
{
    if (argc != 4 || strcmp(argv[1], "-o") != 0) {
        fprintf(stderr, "Usage: %s -o OUTDIR input.txt\n", argv[0]);
        return 1;
    }

    const char *outdir = argv[2];
    const char *input_path = argv[3];

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

    if (evaluate_line(line, &result, &error_offset) != 0) {
        error_position = file_position + (long long)error_offset;
        has_error = 1;
        break;
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

        evaluate_line(line, &result, &error_offset);
        fprintf(output, "%lld\n", result);
    }
}

    fclose(input);
    fclose(output);

    return 0;
}