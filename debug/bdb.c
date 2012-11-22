#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"

#define TEST_DIR    "./"
#define TEST_DB     "./test.db"
#define TEST_KEY    "key"
#define TEST_VAL    "value"

DB *OpenDb(const char *pszPath, DB_ENV *env);
void CloseDb(DB *db);
int PutDb(DB *db, DB_TXN *txn, const char *pszKey, const char *pszValue);
int GetDb(DB *db, DB_TXN *txn, const char *pszKey, char *buff, int size);
int DelDb(DB *db, DB_TXN *txn, const char *pszKey);
DB_ENV *OpenDbEnv(const char *pszDir);
void CloseDbEnv(DB_ENV *env);

int main(int argc, char **argv)
{
    DB *db = NULL;   /* DB structure handle */
    DB_ENV *env = NULL;
    DB_TXN *txn = NULL;
    int nRet = 0;
    char szValue[0xFF] = {0};

    unlink(TEST_DB);

    if ( (env = OpenDbEnv(TEST_DIR)) == NULL )
        goto err;
    
    if ( (db = OpenDb(TEST_DB, env)) == NULL )
        goto err;

    printf("open database %s\n", TEST_DB);

    /* Get the txn handle */
    if ((nRet = env->txn_begin(env, NULL, &txn, 0)) != 0)
    {
        printf("fail to begin transaction, %s\n", db_strerror(nRet));
        goto err;
    }

    if ( PutDb(db, txn, TEST_KEY, TEST_VAL) != 0 )
    {
        txn->abort(txn);
        goto err;
    }

    printf("insert %s/%s\n", TEST_KEY, TEST_VAL);

    if ( GetDb(db, txn, TEST_KEY, szValue, sizeof(szValue)-1) != 0 )
        goto err;

    if ( DelDb(db, txn, TEST_KEY) != 0 )
        goto err;

    if ( GetDb(db, txn, TEST_KEY, szValue, sizeof(szValue)-1) != DB_NOTFOUND )
        goto err;

    if ( (nRet = txn->commit(txn, 0)) != 0 )
    {
        printf("fail to commit transaction, %s\n", db_strerror(nRet));
        goto err;
    }

    printf("fetch %s/%s\n", TEST_KEY, szValue);

err:
    CloseDb(db);
    CloseDbEnv(env);
    
    return 0;
}

DB *OpenDb(const char *pszPath, DB_ENV *env)
{
    DB *db = NULL;
    int nRet = 0;
    u_int32_t db_flags = 0;

    /* 
        Initialize the structure. This database is not opened in an environment, 
        so the environment pointer is NULL. 
    */
    if ( (nRet = db_create(&db, env, 0)) != 0 )
    {
        printf("fail to db_create, %s\n", db_strerror(nRet));
        return NULL;
    }

    /*
        DB_CREATE : If the database does not exist, create it.
    */
    db_flags = DB_CREATE | DB_AUTO_COMMIT;
    
    /* open the database */
    nRet = db->open(db,        /* DB structure pointer */
        NULL,       /* Transaction pointer */
        pszPath,    /* On-disk file that holds the database. */
        NULL,       /* Optional logical database name */
        DB_BTREE,   /* Database access method */
        db_flags,   /* Open flags */
        0);         /* File mode (using defaults) */
    
    if (nRet != 0)
    {
        printf("fail to open db, %s\n", db_strerror(nRet));
        return NULL;
    }

    return db;
}

void CloseDb(DB *db)
{
    /* When we're done with the database, close it. */
    if ( db != NULL )
        db->close(db, 0); 
}

int PutDb(DB *db, DB_TXN *txn, const char *pszKey, const char *pszValue)
{
    int nRet;
    DBT key, data;

    /* Zero out the DBTs before using them. */
    //memset(&key, 0, sizeof(DBT));
    //memset(&data, 0, sizeof(DBT));
	
    
    key.data = (void *)pszKey;
    key.size = strlen(pszKey) + 1;
    
    data.data = (void *)pszValue;
    data.size = strlen(pszValue) + 1;

    if ( (nRet = db->put(db, txn, &key, &data, DB_NOOVERWRITE)) != 0 )
    {
        printf("fail to db put, %s\n", db_strerror(nRet));
        return nRet;
    }
    
    return nRet;
}

int GetDb(DB *db, DB_TXN *txn, const char *pszKey, char *buff, int size)
{
    int nRet;
    DBT key, data;
    
    //memset(&key, 0, sizeof(DBT));
    //memset(&data, 0, sizeof(DBT));

    key.data = (void *)pszKey;
    key.size = strlen(pszKey) + 1;

    data.data = (void *)buff;
    data.ulen = size;
    data.flags = DB_DBT_USERMEM;

    if ( (nRet = db->get(db, txn, &key, &data, 0)) != 0 )
    {
        printf("fail to db get, %s\n", db_strerror(nRet));
        return nRet;
    }
    
    buff[data.size] = 0;
    return nRet;
}

int DelDb(DB *db, DB_TXN *txn, const char *pszKey)
{
    int nRet;
    DBT key;

    //memset(&key, 0, sizeof(DBT));

    key.data = (void *)pszKey;
    key.size = strlen(pszKey) + 1;

    if ( (nRet = db->del(db, txn, &key, 0)) != 0 )
    {
        printf("fail to db del, %s\n", db_strerror(nRet));
        return nRet;
    }

    return nRet;
}

DB_ENV *OpenDbEnv(const char *pszDir)
{
    int nRet = 0;
    u_int32_t env_flags;
    DB_ENV *env = NULL;

    /* Open the environment */
    if ( (nRet = db_env_create(&env, 0)) != 0) 
    {
        printf("fail to db_env_create, %s\n", db_strerror(nRet));
        return NULL;
    }
    
    /*
        DB_CREATE     : Create the environment if it does not already exist
        DB_INIT_TXN   : Initialize transactions
        DB_INIT_LOCK  : Initialize locking
        DB_INIT_MPOOL : Initialize the in-memory cache
    */
    env_flags = DB_CREATE  | DB_INIT_MPOOL| DB_INIT_TXN | DB_INIT_LOCK;
    
    if ( (nRet = env->open(env, "./tmp", env_flags, 0)) != 0) 
    {
		
        printf("pszDir %s\n", pszDir);
        printf("fail to env open, %s\n", db_strerror(nRet));
        printf("Error code %d\n", nRet);
		//printf("DB_RUNRECOVERY code %d\n", DB_RUNRECOVERY);
		//printf("DB_VERSION_MISMATCH code %d\n", DB_VERSION_MISMATCH);
		//printf("EAGAIN code %d\n", DB_EAGAIN);
		//printf("EINVAL code %d\n", DB_EINVAL);
		//printf("ENOENT code %d\n", DB_ENOENT);
		
        return NULL;
    }

    return env;
}

void CloseDbEnv(DB_ENV *env)
{
    if ( env )
        env->close(env, 0);
}

