#include <db.h>

int main(int argc, char *argv[])
{

DB_ENV *myEnv; /* Env structure handle */
DB *dbp; /* DB structure handle */
u_int32_t db_flags; /* database open flags */
u_int32_t env_flags; /* env open flags */
int ret; /* function return value */
/*
Create an environment object and initialize it for error
reporting.
*/
ret = db_env_create(&myEnv, 0);
if (ret != 0) {
fprintf(stderr, "Error creating env handle: %s\n", db_strerror(ret));
return -1;
}
/* Open the environment. */
env_flags = DB_CREATE | /* If the environment does not exist,
* create it. */
DB_INIT_MPOOL; /* Initialize the in-memory cache. */
ret = myEnv->open(myEnv, /* DB_ENV ptr */
"/export1/testEnv", /* env home directory */
env_flags, /* Open flags */
0); /* File mode (default) */
if (ret != 0) {
fprintf(stderr, "Environment open failed: %s", db_strerror(ret));
return -1;
}

/*
Once an environment is opened, you can open databases in it. Note that by default databases
are stored in the environment's home directory, or relative to that directory if you provide any
sort of a path in the database's file name:
Library Version 11.2.5.3 Databases
12/19/2011 Getting Started with DB Page 16
/*
* Initialize the DB structure. Pass the pointer
* to the environment in which this DB is opened.
*/
ret = db_create(&dbp, myEnv, 0);
if (ret != 0) {
/* Error handling goes here */
}
/* Database open flags */
db_flags = DB_CREATE; /* If the database does not exist,
* create it.*/
/* open the database */
ret = dbp->open(dbp, /* DB structure pointer */
NULL, /* Transaction pointer */
"my_db.db", /* On-disk file that holds the database. */
NULL, /* Optional logical database name */
DB_BTREE, /* Database access method */
db_flags, /* Open flags */
0); /* File mode (using defaults) */
if (ret != 0) {
/* Error handling goes here */
}

/*
When you are done with an environment, you must close it. It is recommended that before
closing an environment, you close any open databases.
*/
/*
* Close the database and environment
*/
if (dbp != NULL) {
dbp->close(dbp, 0);
}
if (myEnv != NULL) {
myEnv->close(myEnv, 0);
}

}
