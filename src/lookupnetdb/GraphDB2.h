#include <db.h>
#include "SimpleGraph.h"

void env_dir_create(char* envdir);
void env_open(DB_ENV **dbenvp, char* envdir, unsigned int cachesize);
int addRecord(DB_ENV *dbenv, DB* db, int* gkey, int size, int* nnei, int* nei, int count);
int addRecord2(DB_ENV *dbenv, DB* db, int* gkey, int size, int* nnei, int* nei, int count);
int db_dump(DB_ENV *dbenv, DB* db, int size);
int db_merge(DB_ENV *dbenv, DB* db, char* from);
void db_open(DB_ENV *dbenv, DB **dbp, char *name, int dups);
sSimpleGraph* db_getnext(DB_ENV *dbenv, DB* db, DBC *dbc, int getflag, int minsize);


