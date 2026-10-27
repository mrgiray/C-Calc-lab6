// Artun Giray OZTEKIN 251ADB223
// https://github.com/mrgiray/C-Calc-lab6
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: ChatGPT: used it to fix(debug) and learn things i didnt understand from W3 or GFG websites and used it to fix-up my explanations for the functions.
//                  GPT chat link: https://chatgpt.com/share/6ae0cc0d-1a60-83ed-abae-1bbc626e76b1
//                  W3Schools: used to learn, remember and check what i know.
//                  GeeksforGeeks: same thing, learn, remember and checking out things
//                  RTU course materials
//                  Friends (ofc we helped each other, lol)                 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h> 

#define STUDENT_NAME "artun"
#define STUDENT_LASTNAME "oztekin"
#define STUDENT_ID "251ADB223"

/* Stores either an integer or a floating point number. */
typedef struct {
    int is_float;
    long long integer;
    double floating;
} Value;

/* Converts a value into a double when a floating point calculation is required. */
double value_as_double(Value value)
{
    if (value.is_float) {
        return value.floating;
    }

    return (double)value.integer;
}

/* Skips spaces and tabs while reading the expression. */
void skip_spaces(const char **p)
{
    while (**p == ' ' || **p == '\t') {
        (*p)++;
    }
}

/* Declares parse_expression early so it can be used by the other parser functions. */
int parse_expression(const char **p, const char *line,
                     Value *value, size_t *error_offset);

/* Reads a number from the expression.
   It reads the digits first, then checks if there is a decimal point.
   And if there is one, it reads the decimal part and saves the number part as a float.
   If not, it keeps it as an integer. */
int parse_number(const char **p, const char *line, Value *value, size_t *error_offset)
{
    skip_spaces(p);

    if (!isdigit((unsigned char)**p)) {
        *error_offset = (size_t)(*p - line);
        return -1;
    }

    long long integer = 0;

    while (isdigit((unsigned char)**p)) {
        integer = integer * 10 + (**p - '0');
        (*p)++;
    }

    if (**p == '.') {
        const char *dot = *p;
        (*p)++;

        if (!isdigit((unsigned char)**p)) {
            *error_offset = (size_t)(dot - line);
            return -1;
        }

        double floating = (double)integer;
        double place = 0.1;

        while (isdigit((unsigned char)**p)) {
            floating += (**p - '0') * place;
            place *= 0.1;
            (*p)++;
        }

        value->is_float = 1;
        value->floating = floating;
        value->integer = 0;
    }
    else {
        value->is_float = 0;
        value->integer = integer;
        value->floating = 0.0;
    }

    return 0;
}

/* Reads the basic parts of an expression.
   It can read a normal number or something thats inside a parentheses.
   Then if it finds parentheses, it reads the expression inside them first. */
// Used ChatGPT for a recursion example and help connecting parentheses to the parser.
int parse_primary(const char **p, const char *line, Value *value, size_t *error_offset)
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

/* Calculates integer powers by multiplying the base by itself. */
long long integer_power(long long base, long long exponent)
{
    long long result = 1;

    while (exponent > 0) {
        result *= base;
        exponent--;
    }

    return result;
}

/* Handles the exponent(**) operator.
   It reads the value on the right side first, so powers are done correctly.
   It also uses different calculations for integer and floating point values. */
int parse_power(const char **p, const char *line, Value *value, size_t *error_offset)
{
    if (parse_primary(p, line, value, error_offset) != 0) {
        return -1;
    }

    skip_spaces(p);

    if (**p == '*' && *(*p + 1) == '*') {
        size_t operator_position = (size_t)(*p - line);

        (*p) += 2;
        skip_spaces(p);

        size_t exponent_start = (size_t)(*p - line);

        Value exponent;

        if (parse_power(p, line, &exponent, error_offset) != 0) {
            return -1;
        }

        if (!exponent.is_float && exponent.integer < 0) {
            *error_offset = exponent_start;
            return -1;
        }

        if (!value->is_float && !exponent.is_float) {
            value->integer =
                integer_power(value->integer, exponent.integer);
        }
        else {
            double result =
                pow(value_as_double(*value),
                    value_as_double(exponent));

            if (!isfinite(result)) {
                *error_offset = operator_position;
                return -1;
            }

            value->is_float = 1;
            value->floating = result;
            value->integer = 0;
        }
    }

    return 0;
}

/* Handles multiplication and division.
   It gets the numbers from parse_power first so powers are finished before multiplication(*) and division(/).
   It also checks for division by zero and keeps normal integer division when needed. */
// Used ChatGPT to understand the parser order and operator precedence.
int parse_term(const char **p, const char *line, Value *value, size_t *error_offset)
{
    if (parse_power(p, line, value, error_offset) != 0) {
        return -1;
    }

    while (1) {
        skip_spaces(p);

        if (**p != '*' && **p != '/') {
            return 0;
        }

        size_t operator_position = (size_t)(*p - line);

        char op = **p;
        (*p)++;

        skip_spaces(p);

        size_t right_operand = (size_t)(*p - line);

        Value number;

        if (parse_power(p, line, &number, error_offset) != 0) {
            return -1;
        }

        int zero = number.is_float
                   ? number.floating == 0.0
                   : number.integer == 0;

        if (op == '/' && zero) {
            *error_offset = right_operand;
            return -1;
        }

        if (value->is_float || number.is_float) {
            double left = value_as_double(*value);
            double right = value_as_double(number);

            double result;

            if (op == '*') {
                result = left * right;
            }
            else {
                result = left / right;
            }

            if (!isfinite(result)) {
                *error_offset = operator_position;
                return -1;
            }

            value->is_float = 1;
            value->floating = result;
            value->integer = 0;
        }
        else {
            if (op == '*') {
                value->integer *= number.integer;
            }
            else {
                value->integer /= number.integer;
            }
        }
    }
}

/* Handles addition and subtraction.
   It gets the values from parse_term first, so multiplication and division
   are already done before + and - are calculated. */
// Used ChatGPT to understand how parsing should stop at a closing parenthesis.
int parse_expression(const char **p, const char *line, Value *value, size_t *error_offset)
{
    Value result;

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

        size_t operator_position = (size_t)(*p - line);

        char op = **p;
        (*p)++;

        Value number;

        if (parse_term(p, line, &number, error_offset) != 0) {
            return -1;
        }

        if (result.is_float || number.is_float) {
            double left = value_as_double(result);
            double right = value_as_double(number);

            double calculated;

            if (op == '+') {
                calculated = left + right;
            }
            else {
                calculated = left - right;
            }

            if (!isfinite(calculated)) {
                *error_offset = operator_position;
                return -1;
            }

            result.is_float = 1;
            result.floating = calculated;
            result.integer = 0;
        }
        else {
            if (op == '+') {
                result.integer += number.integer;
            }
            else {
                result.integer -= number.integer;
            }
        }
    }
}

/* Reads one full line as an expression.
   After getting the result it checks if there are any extra or invalid
   characters left. */
int evaluate_line(const char *line, Value *answer, size_t *error_offset)
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

/* Gets the file name without the path and removes the .txt extension. */
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

/* Checks if a line should be ignored because it is empty or a comment. */
int line_is_ignored(const char *line)
{
    const char *p = line;

    skip_spaces(&p);

    if (*p == '\n' || *p == '\0' || *p == '#') {
        return 1;
    }

    return 0;
}

/* Checks if a file name ends with .txt. */
// Used ChatGPT to debug the .txt extension check.
int has_txt_extension(const char *name)
{
    const char *dot = strrchr(name, '.');

    return dot != NULL && strcmp(dot, ".txt") == 0;
}

/* Reads one full line from a file.
   The buffer gets bigger if the line does not fit in the current size. */
int read_line_dynamic(FILE *file, char **line, size_t *capacity)
{
    if (*line == NULL) {
        *capacity = 128;
        *line = malloc(*capacity);

        if (*line == NULL) {
            return -1;
        }
    }

    size_t length = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF) {

        if (length + 1 >= *capacity) {
            size_t new_capacity = *capacity * 2;

            char *temp = realloc(*line, new_capacity);

            if (temp == NULL) {
                return -1;
            }

            *line = temp;
            *capacity = new_capacity;
        }

        (*line)[length] = (char)ch;
        length++;

        if (ch == '\n') {
            break;
        }
    }

    if (length == 0 && ch == EOF) {
        return 0;
    }

    (*line)[length] = '\0';

    return 1;
}

/* Processes one input file.
   It first goes through the file to see if there is any errors.
   Then if there is an error it writes that error.
   Andif there is no error, it reads the file again and writes all the results. */
// Used ChatGPT to understand moving single-file processing out of main.
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

    char *line = NULL;
    size_t line_capacity = 0;
    int read_status;

    long long file_position = 1;
    long long error_position = 0;
    int has_error = 0;

    while ((read_status = read_line_dynamic(input, &line, &line_capacity)) == 1) {
        Value result;
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

    if (read_status == -1) {
    fprintf(stderr, "Memory allocation failed\n");
    free(line);
    fclose(input);
    fclose(output);
    return 1;
}

    if (has_error) {
        fprintf(output, "ERROR:%lld\n", error_position);
    }
    else {
        rewind(input);

        while ((read_status = read_line_dynamic(input, &line, &line_capacity)) == 1) {
            Value result;
            size_t error_offset;

            if (line_is_ignored(line)) {
                continue;
            }

            evaluate_line(line, &result, &error_offset);
            if (result.is_float) {
                fprintf(output, "%.15g\n", result.floating);
            }
            else {
                fprintf(output, "%lld\n", result.integer);
            }
        }

        if (read_status == -1) {
        fprintf(stderr, "Memory allocation failed\n");
        free(line);
        fclose(input);
        fclose(output);
        return 1;
        }
    }
    free(line);
    fclose(input);
    fclose(output);

    return 0;
}

/* Goes through a directory and looks for .txt files.
   It checks if they are normal files and then processes them one by one. */
// Used ChatGPT to debug the directory path joining and .txt file processing.
int scan_directory(const char *dirpath, const char *outdir)
{
    DIR *dir = opendir(dirpath);

    if (dir == NULL) {
        perror(dirpath);
        return 1;
    }

    mkdir(outdir, 0777);

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

/* Gets the folder name from a full directory path. */
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

/* Reads the command line arguments.
   It checks which file or folder should be used, finds the output folder,
   and then starts either single-file or directory processing. */
// Used ChatGPT to debug argument checks for directory mode.
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