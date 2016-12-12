#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <editline/readline.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "symtab.h"
#include "parse.h"
#include "builtins.h"

extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
    int option = 0, repl = 0, files = 0, debug = 0;
    Sv *result = NULL;

    Symtab_init();
    PUSH_SCOPE;
    Builtin_init();
    Sv_init();
    POP_SCOPE;

    while((option = getopt(argc, argv, "rl:f:d")) != -1) {
        switch(option) {
        case 'f':
            files = 1;
            PUSH_SCOPE;
            result = Parse_file(optarg);
            Sv_dump(Sv_eval(MAIN_ENV, result));
            POP_SCOPE;
            printf("\n");
            break;

        case 'l':
            files = 1;
            PUSH_SCOPE;
            Sv_eval(MAIN_ENV, Parse_file(optarg));
            POP_SCOPE;
            break;

        case 'r':
            repl = 1;
            break;

        case 'd':
            debug = 1;
            break;
        }
    }

    if (!files || repl) {
        for (;;) {
            char *input = readline("stutter> ");
            if (input == NULL) {
                printf("\nBye!\n");
                break;
            }

            if (*input != '\0') {
                add_history(input);
                PUSH_SCOPE;
                result = Parse_buf(input);
                Sv_dump(Sv_eval(MAIN_ENV, result));
                POP_SCOPE;
                printf("\n");
            }

            free(input);
        }
    }

    Gc_sweep(0);
    if (debug)
        Gc_dump_stats();
    Symtab_destroy();

    return 0;
}
