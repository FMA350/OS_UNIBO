#include <stdio.h>
#include <stdarg.h>

int vrprintf(const char *format, va_list ap);

//static function are functions that are only visible to functions in this file

//writes the given character (as int) to the standard output it the character is not 0
static int putcx(int c) {
    if (c!=0) {
        putchar(c);
        return 1;
    } else
        return 0;
}

static char backchar[128] = {
        [0 ... 127] = 0,
        ['a'] = '\a',
        ['b'] = '\b',
        ['f'] = '\f',
        ['n'] = '\n',
        ['r'] = '\r',
        ['t'] = '\t',
        ['v'] = '\v',
        ['\\'] = '\\', // double escape character is needed for saving the escape character itself in a variable
        ['\''] = '\'', //use the escape character for saving the apostrophe
        ['"'] = '"',
        ['?'] = '?',
};

static int put_backslash(char escapeChar) {
    //write the backslash
    //the and-bitwise is for switch off the last bit (ASCII compatibility)
    return putcx(backchar[escapeChar & 0x7f]);
}

static int rvrp_int(int val) {
    if (val == 0)
        return 0;
    else  //recurvise call on the more significant digit and print the less significant
        return rvrp_int(val / 10) + putcx('0' + val % 10);
}

static int vrp_int(int val) {
    if (val != 0) {
        if (val < 0)
            return putcx('-') + rvrp_int(-val);
        else
            return rvrp_int(val);
    } else
        return putcx('0');
}

static int vrp_string(char *s) {
    switch (*s) {
        case 0:
            return 0;
        case '\\':
            //if after a \ the string not ends recursive call on next character otherwise return 0
            return put_backslash(*(s+1)) ? vrp_string(s + 2) + 1 : 0;
        default:
            return putcx(*s) + vrp_string(s + 1);
    }
}

static int vrp_percent(const char *format, va_list ap) {
    switch (*format) {
        case 0:
            return 0;
        case '%':
            //case when a % is followed by a % so print a % and advance
            return putcx(*format) + vrprintf(format + 1, ap);
        case 'd':
            //call vrp_int with the next argument of the list with type int
            return vrp_int(va_arg(ap, int)) + vrprintf(format + 1, ap);
        case 's':
            //call vrp_int with the next argument of the list with type pointer to char
            return vrp_string(va_arg(ap, char *)) + vrprintf(format + 1, ap);;
        default:
            //if the format is not recognized
            printf("ERROR\n");
            return 0;
    }
}

int vrprintf(const char *format, va_list ap) {
    //parse the format
    switch (*format) {
        //case format null
        case 0:
            return 0;
            //case format %: call vrp_percent and advance 1 position
        case '%':
            return vrp_percent(format + 1, ap);
        case '\\':
            //if after a \ the string not ends recursive call on next character otherwise return 0
            return put_backslash(*(format+1)) ? vrprintf(format + 2, ap) + 1 : 0;
        default:
            //print the character and recursive call on next character
            return putcx(*format) + vrprintf(format + 1, ap);
    }
}

//function with undetermined number of arguments. Returns the number of characters printed.
int rprintf(const char *format, ...) {
    int rval;
    //declare a list of arguments
    va_list ap;
    //initialize the list
    va_start (ap, format);
    rval = vrprintf(format, ap);
    //clean up the list
    va_end(ap);
    return rval;
}

int main() {
    int v;
    v = rprintf("hello world\n");
    printf("%d\n", v);
    v = printf("hello world\n");
    printf("%d\n", v);
    v = rprintf("hello world %d\n", 10);
    printf("%d\n", v);
    v = printf("hello world %d\n", 10);
    printf("%d\n", v);
    v = rprintf("hello world %s %d\n", "piripicchio", 42);
    printf("%d\n", v);
    v = printf("hello world %s %d\n", "piripicchio", 42);
    printf("%d\n", v);
    v = rprintf("hello world %% \"%s\" %d\n", "piripicchio\tbackslash", 42);
    printf("%d\n", v);
    v = printf("hello world %% \"%s\" %d\n", "piripicchio\tbackslash", 42);
    printf("%d\n", v);
    v = rprintf("%%\n");
    printf("%d\n", v);
    v = printf("%%\n");
    printf("%d\n", v);
}
