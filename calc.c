// Artun Giray OZTEKIN 251ADB223
// https://github.com/mrgiray/C-Calc-lab6
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used:ChatGPT, W3Schools, GeekForGeeks, PDF's and friends.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define STUDENT_NAME "artun"
#define STUDENT_LASTNAME "oztekin"
#define STUDENT_ID "251ADB223"

/*
 * AI (ChatGPT): helped adapt my earlier argc/argv, file I/O,
 * and calculator lab code into the first Grade 4 version.
 */

/* Skip spaces and tabs between numbers and operators. */
void skip_spaces(const char **p)
{
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

/* Grade 4 parser: integers with only + and -. */
long long evaluate_line(const char *line)
{
    const char *p = line;

    skip_spaces(&p);

    long long result = 0;

    while (isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
    }

    while (1) {
        skip_spaces(&p);

        if (*p == '\n' || *p == '\0') {
            break;
        }

        char op = *p;
        p++;

        skip_spaces(&p);

        long long number = 0;

        while (isdigit((unsigned char)*p)) {
            number = number * 10 + (*p - '0');
            p++;
        }

        if (op == '+') {
            result += number;
        }
        else if (op == '-') {
            result -= number;
        }
    }

    return result;
}

/* Get filename without path and without .txt. */
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

    /*
     * mkdir creates the output folder.
     * If it already exists, continuing is fine for this assignment.
     */
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

    while (fgets(line, sizeof(line), input) != NULL) {
        long long result = evaluate_line(line);
        fprintf(output, "%lld\n", result);
    }

    fclose(input);
    fclose(output);

    return 0;
}