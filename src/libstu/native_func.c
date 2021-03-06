/*
 * Copyright (c) 2017 Raphael Sousa Santos <contact@raphaelss.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "native_func.h"
#include "stu_private.h"
#include "utils.h"

typedef struct Sv_native_func {
    Sv_native_func_t func;
    unsigned arity;
    unsigned char flags;
} Sv_native_func;

typedef struct Sv_native_closure {
    Sv_native_func_t func;
    unsigned arity, bound_num;
    unsigned char flags;
    Sv *bound_args[];
} Sv_native_closure;

static void
resize_args(Stu *stu, unsigned size)
{
    Sv **array = CHECKED_REALLOC(stu->native_func_args, size * sizeof(Sv*));
    stu->native_func_args = array;
    stu->native_func_args_capacity = size;
}

extern Sv_native_func
*Sv_native_func_new(Stu *stu, Sv_native_func_t func, unsigned arity, unsigned char flags)
{
    Sv_native_func *f = CHECKED_MALLOC(sizeof(*f));
    f->func = func;
    f->arity = arity;
    f->flags = flags;
    if (arity > stu->native_func_args_capacity)
        resize_args(stu, arity);
    return f;
}

static Sv
*Native_call(Stu *stu, Env *env, Sv_native_func_t f,
             unsigned arity, unsigned flags, unsigned bound_num,
             Sv **arg_array, Sv *args)
{
    int rest = flags & SV_NATIVE_FUNC_REST;
    if (rest)
        --arity;
    while (!IS_NIL(args) && arity > 0) {
        *(arg_array++) = CAR(args);
        args = CDR(args);
        --arity;
        ++bound_num;
    }

    if (arity > 0) {
        Sv_native_closure *c = CHECKED_MALLOC(
            sizeof(*c) + bound_num * sizeof(Sv*));
        c->arity = arity;
        c->flags = flags;
        c->func = f;
        c->bound_num = bound_num;
        memcpy(c->bound_args, stu->native_func_args, bound_num * sizeof(Sv*));
        Sv *sv = Sv_new(stu, SV_NATIVE_CLOS);
        sv->val.clos = c;
        return sv;
    } else {
        if (rest) {
            *arg_array = args;
            ++bound_num;
        }
        else if (!IS_NIL(args))
            return Sv_new_err(stu, "Wrong number of arguments");
        return f(stu, env, stu->native_func_args);
    }
}

extern Sv
*Sv_native_func_call(Stu *stu, Env *env, Sv_native_func *f, Sv *args)
{
    return Native_call(
        stu, env, f->func, f->arity, f->flags, 0, stu->native_func_args, args);
}

extern Sv
*Sv_native_closure_call(Stu *stu, Env *env, Sv_native_closure *f, Sv *args)
{
    memcpy(stu->native_func_args, f->bound_args, f->bound_num * sizeof(Sv*));
    return Native_call(
        stu, env, f->func, f->arity, f->flags, f->bound_num,
        stu->native_func_args + f->bound_num, args);
}

extern Sv
*Sv_native_func_register(Stu *stu, const char *name, Sv_native_func_t f, unsigned args, unsigned flags)
{
    Sv *x = Sv_new_native_func(stu, f, args, flags);
    Env_main_put(stu, Sv_new_sym(stu, name), x);
    return x;
}

extern int
Sv_native_func_is_macro(Sv_native_func *f)
{
    return f->flags & SV_NATIVE_FUNC_MACRO;
}
