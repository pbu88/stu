#ifndef ENV_DEFINED
#define ENV_DEFINED

#include "hash.h"
#include "sv.h"

typedef struct Env {
    Hash *hash;
} Env;

extern Env *Env_new(void);
extern void Env_destroy(Env **);
extern void Env_put(Env *, const char *, Sv *);
extern Sv *Env_get(Env *, const char *);
extern Sv *Env_del(Env *, const char *);

#endif
