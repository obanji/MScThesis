/******************************************from the file hash.h**************************************/
/*
** 2001 September 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This is the header file for the generic hash-table implemenation
** used in SQLite.
**
** $Id: hash.h,v 1.9 2006/02/14 10:48:39 danielk1977 Exp $
*/
#ifndef _SQLITE_HASH_H_
#define _SQLITE_HASH_H_

/* Forward declarations of structures. */
typedef struct Hash Hash;
typedef struct HashElem HashElem;

/* A complete hash table is an instance of the following structure.
** The internals of this structure are intended to be opaque -- client
** code should not attempt to access or modify the fields of this structure
** directly.  Change this structure only by using the routines below.
** However, many of the "procedures" and "functions" for modifying and
** accessing this structure are really macros, so we can't really make
** this structure opaque.
*/
struct Hash {
  char keyClass;          /* SQLITE_HASH_INT, _POINTER, _STRING, _BINARY */
  char copyKey;           /* True if copy of key made on insert */
  int count;              /* Number of entries in this table */
  HashElem *first;        /* The first element of the array */
  void *(*xMalloc)(int);  /* malloc() function to use */
  void (*xFree)(void *);  /* free() function to use */
  int htsize;             /* Number of buckets in the hash table */
  struct _ht {            /* the hash table */
    int count;               /* Number of entries with this hash */
    HashElem *chain;         /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
struct HashElem {
  HashElem *next, *prev;   /* Next and previous elements in the table */
  void *data;              /* Data associated with this element */
  void *pKey; int nKey;    /* Key associated with this element */
};

/*
** There are 4 different modes of operation for a hash table:
**
**   SQLITE_HASH_INT         nKey is used as the key and pKey is ignored.
**
**   SQLITE_HASH_POINTER     pKey is used as the key and nKey is ignored.
**
**   SQLITE_HASH_STRING      pKey points to a string that is nKey bytes long
**                           (including the null-terminator, if any).  Case
**                           is ignored in comparisons.
**
**   SQLITE_HASH_BINARY      pKey points to binary data nKey bytes long. 
**                           memcmp() is used to compare keys.
**
** A copy of the key is made for SQLITE_HASH_STRING and SQLITE_HASH_BINARY
** if the copyKey parameter to HashInit is 1.  
*/
/* #define SQLITE_HASH_INT       1 // NOT USED */
/* #define SQLITE_HASH_POINTER   2 // NOT USED */
#define SQLITE_HASH_STRING    3
#define SQLITE_HASH_BINARY    4

/*
** Access routines.  To delete, insert a NULL pointer.
*/
void sqlite3HashInit(Hash*, int keytype, int copyKey);
void *sqlite3HashInsert(Hash*, const void *pKey, int nKey, void *pData);
void *sqlite3HashFind(const Hash*, const void *pKey, int nKey);
void sqlite3HashClear(Hash*);

/*
** Macros for looping over all elements of a hash table.  The idiom is
** like this:
**
**   Hash h;
**   HashElem *p;
**   ...
**   for(p=sqliteHashFirst(&h); p; p=sqliteHashNext(p)){
**     SomeStructure *pData = sqliteHashData(p);
**     // do something with pData
**   }
*/
#define sqliteHashFirst(H)  ((H)->first)
#define sqliteHashNext(E)   ((E)->next)
#define sqliteHashData(E)   ((E)->data)
#define sqliteHashKey(E)    ((E)->pKey)
#define sqliteHashKeysize(E) ((E)->nKey)

/*
** Number of entries in a hash table
*/
#define sqliteHashCount(H)  ((H)->count)

#endif /* _SQLITE_HASH_H_ */



/********************************from the file sqlite3.h***********************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This header file defines the interface that the SQLite library
** presents to client programs.
**
** @(#) $Id: sqlite.h.in,v 1.165 2006/04/04 01:54:55 drh Exp $
*/
#ifndef _SQLITE3_H_
#define _SQLITE3_H_
#include <stdarg.h>     /* Needed for the definition of va_list */

/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

/*
** The version of the SQLite library.
*/
#ifdef SQLITE_VERSION
# undef SQLITE_VERSION
#endif
#define SQLITE_VERSION         "3.3.5"

/*
** The format of the version string is "X.Y.Z<trailing string>", where
** X is the major version number, Y is the minor version number and Z
** is the release number. The trailing string is often "alpha" or "beta".
** For example "3.1.1beta".
**
** The SQLITE_VERSION_NUMBER is an integer with the value 
** (X*100000 + Y*1000 + Z). For example, for version "3.1.1beta", 
** SQLITE_VERSION_NUMBER is set to 3001001. To detect if they are using 
** version 3.1.1 or greater at compile time, programs may use the test 
** (SQLITE_VERSION_NUMBER>=3001001).
*/
#ifdef SQLITE_VERSION_NUMBER
# undef SQLITE_VERSION_NUMBER
#endif
#define SQLITE_VERSION_NUMBER 3003005

/*
** The version string is also compiled into the library so that a program
** can check to make sure that the lib*.a file and the *.h file are from
** the same version.  The sqlite3_libversion() function returns a pointer
** to the sqlite3_version variable - useful in DLLs which cannot access
** global variables.
*/
extern const char sqlite3_version[];
const char *sqlite3_libversion(void);

/*
** Return the value of the SQLITE_VERSION_NUMBER macro when the
** library was compiled.
*/
int sqlite3_libversion_number(void);

/*
** Each open sqlite database is represented by an instance of the
** following opaque structure.
*/
typedef struct sqlite3 sqlite3;


/*
** Some compilers do not support the "long long" datatype.  So we have
** to do a typedef that for 64-bit integers that depends on what compiler
** is being used.
*/
#ifdef SQLITE_INT64_TYPE
  typedef SQLITE_INT64_TYPE sqlite_int64;
  typedef unsigned SQLITE_INT64_TYPE sqlite_uint64;
#elif defined(_MSC_VER) || defined(__BORLANDC__)
  typedef __int64 sqlite_int64;
  typedef unsigned __int64 sqlite_uint64;
#else
  typedef long long int sqlite_int64;
  typedef unsigned long long int sqlite_uint64;
#endif

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#ifdef SQLITE_OMIT_FLOATING_POINT
# define double sqlite_int64
#endif

/*
** A function to close the database.
**
** Call this function with a pointer to a structure that was previously
** returned from sqlite3_open() and the corresponding database will by closed.
**
** All SQL statements prepared using sqlite3_prepare() or
** sqlite3_prepare16() must be deallocated using sqlite3_finalize() before
** this routine is called. Otherwise, SQLITE_BUSY is returned and the
** database connection remains open.
*/
int sqlite3_close(sqlite3 *);

/*
** The type for a callback function.
*/
typedef int (*sqlite3_callback)(void*,int,char**, char**);

/*
** A function to executes one or more statements of SQL.
**
** If one or more of the SQL statements are queries, then
** the callback function specified by the 3rd parameter is
** invoked once for each row of the query result.  This callback
** should normally return 0.  If the callback returns a non-zero
** value then the query is aborted, all subsequent SQL statements
** are skipped and the sqlite3_exec() function returns the SQLITE_ABORT.
**
** The 4th parameter is an arbitrary pointer that is passed
** to the callback function as its first parameter.
**
** The 2nd parameter to the callback function is the number of
** columns in the query result.  The 3rd parameter to the callback
** is an array of strings holding the values for each column.
** The 4th parameter to the callback is an array of strings holding
** the names of each column.
**
** The callback function may be NULL, even for queries.  A NULL
** callback is not an error.  It just means that no callback
** will be invoked.
**
** If an error occurs while parsing or evaluating the SQL (but
** not while executing the callback) then an appropriate error
** message is written into memory obtained from malloc() and
** *errmsg is made to point to that message.  The calling function
** is responsible for freeing the memory that holds the error
** message.   Use sqlite3_free() for this.  If errmsg==NULL,
** then no error message is ever written.
**
** The return value is is SQLITE_OK if there are no errors and
** some other return code if there is an error.  The particular
** return value depends on the type of error. 
**
** If the query could not be executed because a database file is
** locked or busy, then this function returns SQLITE_BUSY.  (This
** behavior can be modified somewhat using the sqlite3_busy_handler()
** and sqlite3_busy_timeout() functions below.)
*/
int sqlite3_exec(
  sqlite3*,                     /* An open database */
  const char *sql,              /* SQL to be executed */
  sqlite3_callback,             /* Callback function */
  void *,                       /* 1st argument to callback function */
  char **errmsg                 /* Error msg written here */
);

/*
** Return values for sqlite3_exec() and sqlite3_step()
*/
#define SQLITE_OK           0   /* Successful result */
/* beginning-of-error-codes */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_INTERNAL     2   /* NOT USED. Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* NOT USED. Table or record not found */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* Database lock protocol error */
#define SQLITE_EMPTY       16   /* Database is empty */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* NOT USED. Too much data for one row */
#define SQLITE_CONSTRAINT  19   /* Abort due to contraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Auxiliary database format error */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */
/* end-of-error-codes */

/*
** Each entry in an SQLite table has a unique integer key.  (The key is
** the value of the INTEGER PRIMARY KEY column if there is such a column,
** otherwise the key is generated at random.  The unique key is always
** available as the ROWID, OID, or _ROWID_ column.)  The following routine
** returns the integer key of the most recent insert in the database.
**
** This function is similar to the mysql_insert_id() function from MySQL.
*/
sqlite_int64 sqlite3_last_insert_rowid(sqlite3*);

/*
** This function returns the number of database rows that were changed
** (or inserted or deleted) by the most recent called sqlite3_exec().
**
** All changes are counted, even if they were later undone by a
** ROLLBACK or ABORT.  Except, changes associated with creating and
** dropping tables are not counted.
**
** If a callback invokes sqlite3_exec() recursively, then the changes
** in the inner, recursive call are counted together with the changes
** in the outer call.
**
** SQLite implements the command "DELETE FROM table" without a WHERE clause
** by dropping and recreating the table.  (This is much faster than going
** through and deleting individual elements form the table.)  Because of
** this optimization, the change count for "DELETE FROM table" will be
** zero regardless of the number of elements that were originally in the
** table. To get an accurate count of the number of rows deleted, use
** "DELETE FROM table WHERE 1" instead.
*/
int sqlite3_changes(sqlite3*);

/*
** This function returns the number of database rows that have been
** modified by INSERT, UPDATE or DELETE statements since the database handle
** was opened. This includes UPDATE, INSERT and DELETE statements executed
** as part of trigger programs. All changes are counted as soon as the
** statement that makes them is completed (when the statement handle is
** passed to sqlite3_reset() or sqlite_finalise()).
**
** SQLite implements the command "DELETE FROM table" without a WHERE clause
** by dropping and recreating the table.  (This is much faster than going
** through and deleting individual elements form the table.)  Because of
** this optimization, the change count for "DELETE FROM table" will be
** zero regardless of the number of elements that were originally in the
** table. To get an accurate count of the number of rows deleted, use
** "DELETE FROM table WHERE 1" instead.
*/
int sqlite3_total_changes(sqlite3*);

/* This function causes any pending database operation to abort and
** return at its earliest opportunity.  This routine is typically
** called in response to a user action such as pressing "Cancel"
** or Ctrl-C where the user wants a long query operation to halt
** immediately.
*/
void sqlite3_interrupt(sqlite3*);


/* These functions return true if the given input string comprises
** one or more complete SQL statements. For the sqlite3_complete() call,
** the parameter must be a nul-terminated UTF-8 string. For
** sqlite3_complete16(), a nul-terminated machine byte order UTF-16 string
** is required.
**
** The algorithm is simple.  If the last token other than spaces
** and comments is a semicolon, then return true.  otherwise return
** false.
*/
int sqlite3_complete(const char *sql);
int sqlite3_complete16(const void *sql);

/*
** This routine identifies a callback function that is invoked
** whenever an attempt is made to open a database table that is
** currently locked by another process or thread.  If the busy callback
** is NULL, then sqlite3_exec() returns SQLITE_BUSY immediately if
** it finds a locked table.  If the busy callback is not NULL, then
** sqlite3_exec() invokes the callback with three arguments.  The
** second argument is the name of the locked table and the third
** argument is the number of times the table has been busy.  If the
** busy callback returns 0, then sqlite3_exec() immediately returns
** SQLITE_BUSY.  If the callback returns non-zero, then sqlite3_exec()
** tries to open the table again and the cycle repeats.
**
** The default busy callback is NULL.
**
** Sqlite is re-entrant, so the busy handler may start a new query. 
** (It is not clear why anyone would every want to do this, but it
** is allowed, in theory.)  But the busy handler may not close the
** database.  Closing the database from a busy handler will delete 
** data structures out from under the executing query and will 
** probably result in a coredump.
*/
int sqlite3_busy_handler(sqlite3*, int(*)(void*,int), void*);

/*
** This routine sets a busy handler that sleeps for a while when a
** table is locked.  The handler will sleep multiple times until 
** at least "ms" milleseconds of sleeping have been done.  After
** "ms" milleseconds of sleeping, the handler returns 0 which
** causes sqlite3_exec() to return SQLITE_BUSY.
**
** Calling this routine with an argument less than or equal to zero
** turns off all busy handlers.
*/
int sqlite3_busy_timeout(sqlite3*, int ms);

/*
** This next routine is really just a wrapper around sqlite3_exec().
** Instead of invoking a user-supplied callback for each row of the
** result, this routine remembers each row of the result in memory
** obtained from malloc(), then returns all of the result after the
** query has finished. 
**
** As an example, suppose the query result where this table:
**
**        Name        | Age
**        -----------------------
**        Alice       | 43
**        Bob         | 28
**        Cindy       | 21
**
** If the 3rd argument were &azResult then after the function returns
** azResult will contain the following data:
**
**        azResult[0] = "Name";
**        azResult[1] = "Age";
**        azResult[2] = "Alice";
**        azResult[3] = "43";
**        azResult[4] = "Bob";
**        azResult[5] = "28";
**        azResult[6] = "Cindy";
**        azResult[7] = "21";
**
** Notice that there is an extra row of data containing the column
** headers.  But the *nrow return value is still 3.  *ncolumn is
** set to 2.  In general, the number of values inserted into azResult
** will be ((*nrow) + 1)*(*ncolumn).
**
** After the calling function has finished using the result, it should 
** pass the result data pointer to sqlite3_free_table() in order to 
** release the memory that was malloc-ed.  Because of the way the 
** malloc() happens, the calling function must not try to call 
** free() directly.  Only sqlite3_free_table() is able to release 
** the memory properly and safely.
**
** The return value of this routine is the same as from sqlite3_exec().
*/
int sqlite3_get_table(
  sqlite3*,               /* An open database */
  const char *sql,       /* SQL to be executed */
  char ***resultp,       /* Result written to a char *[]  that this points to */
  int *nrow,             /* Number of result rows written here */
  int *ncolumn,          /* Number of result columns written here */
  char **errmsg          /* Error msg written here */
);

/*
** Call this routine to free the memory that sqlite3_get_table() allocated.
*/
void sqlite3_free_table(char **result);

/*
** The following routines are variants of the "sprintf()" from the
** standard C library.  The resulting string is written into memory
** obtained from malloc() so that there is never a possiblity of buffer
** overflow.  These routines also implement some additional formatting
** options that are useful for constructing SQL statements.
**
** The strings returned by these routines should be freed by calling
** sqlite3_free().
**
** All of the usual printf formatting options apply.  In addition, there
** is a "%q" option.  %q works like %s in that it substitutes a null-terminated
** string from the argument list.  But %q also doubles every '\'' character.
** %q is designed for use inside a string literal.  By doubling each '\''
** character it escapes that character and allows it to be inserted into
** the string.
**
** For example, so some string variable contains text as follows:
**
**      char *zText = "It's a happy day!";
**
** We can use this text in an SQL statement as follows:
**
**      char *z = sqlite3_mprintf("INSERT INTO TABLES('%q')", zText);
**      sqlite3_exec(db, z, callback1, 0, 0);
**      sqlite3_free(z);
**
** Because the %q format string is used, the '\'' character in zText
** is escaped and the SQL generated is as follows:
**
**      INSERT INTO table1 VALUES('It''s a happy day!')
**
** This is correct.  Had we used %s instead of %q, the generated SQL
** would have looked like this:
**
**      INSERT INTO table1 VALUES('It's a happy day!');
**
** This second example is an SQL syntax error.  As a general rule you
** should always use %q instead of %s when inserting text into a string 
** literal.
*/
char *sqlite3_mprintf(const char*,...);
char *sqlite3_vmprintf(const char*, va_list);
void sqlite3_free(char *z);
char *sqlite3_snprintf(int,char*,const char*, ...);

#ifndef SQLITE_OMIT_AUTHORIZATION
/*
** This routine registers a callback with the SQLite library.  The
** callback is invoked (at compile-time, not at run-time) for each
** attempt to access a column of a table in the database.  The callback
** returns SQLITE_OK if access is allowed, SQLITE_DENY if the entire
** SQL statement should be aborted with an error and SQLITE_IGNORE
** if the column should be treated as a NULL value.
*/
int sqlite3_set_authorizer(
  sqlite3*,
  int (*xAuth)(void*,int,const char*,const char*,const char*,const char*),
  void *pUserData
);
#endif

/*
** The second parameter to the access authorization function above will
** be one of the values below.  These values signify what kind of operation
** is to be authorized.  The 3rd and 4th parameters to the authorization
** function will be parameters or NULL depending on which of the following
** codes is used as the second parameter.  The 5th parameter is the name
** of the database ("main", "temp", etc.) if applicable.  The 6th parameter
** is the name of the inner-most trigger or view that is responsible for
** the access attempt or NULL if this access attempt is directly from 
** input SQL code.
**
**                                          Arg-3           Arg-4
*/
#define SQLITE_COPY                  0   /* Table Name      File Name       */
#define SQLITE_CREATE_INDEX          1   /* Index Name      Table Name      */
#define SQLITE_CREATE_TABLE          2   /* Table Name      NULL            */
#define SQLITE_CREATE_TEMP_INDEX     3   /* Index Name      Table Name      */
#define SQLITE_CREATE_TEMP_TABLE     4   /* Table Name      NULL            */
#define SQLITE_CREATE_TEMP_TRIGGER   5   /* Trigger Name    Table Name      */
#define SQLITE_CREATE_TEMP_VIEW      6   /* View Name       NULL            */
#define SQLITE_CREATE_TRIGGER        7   /* Trigger Name    Table Name      */
#define SQLITE_CREATE_VIEW           8   /* View Name       NULL            */
#define SQLITE_DELETE                9   /* Table Name      NULL            */
#define SQLITE_DROP_INDEX           10   /* Index Name      Table Name      */
#define SQLITE_DROP_TABLE           11   /* Table Name      NULL            */
#define SQLITE_DROP_TEMP_INDEX      12   /* Index Name      Table Name      */
#define SQLITE_DROP_TEMP_TABLE      13   /* Table Name      NULL            */
#define SQLITE_DROP_TEMP_TRIGGER    14   /* Trigger Name    Table Name      */
#define SQLITE_DROP_TEMP_VIEW       15   /* View Name       NULL            */
#define SQLITE_DROP_TRIGGER         16   /* Trigger Name    Table Name      */
#define SQLITE_DROP_VIEW            17   /* View Name       NULL            */
#define SQLITE_INSERT               18   /* Table Name      NULL            */
#define SQLITE_PRAGMA               19   /* Pragma Name     1st arg or NULL */
#define SQLITE_READ                 20   /* Table Name      Column Name     */
#define SQLITE_SELECT               21   /* NULL            NULL            */
#define SQLITE_TRANSACTION          22   /* NULL            NULL            */
#define SQLITE_UPDATE               23   /* Table Name      Column Name     */
#define SQLITE_ATTACH               24   /* Filename        NULL            */
#define SQLITE_DETACH               25   /* Database Name   NULL            */
#define SQLITE_ALTER_TABLE          26   /* Database Name   Table Name      */
#define SQLITE_REINDEX              27   /* Index Name      NULL            */
#define SQLITE_ANALYZE              28   /* Table Name      NULL            */


/*
** The return value of the authorization function should be one of the
** following constants:
*/
/* #define SQLITE_OK  0   // Allow access (This is actually defined above) */
#define SQLITE_DENY   1   /* Abort the SQL statement with an error */
#define SQLITE_IGNORE 2   /* Don't allow access, but don't generate an error */

/*
** Register a function for tracing SQL command evaluation.  The function
** registered by sqlite3_trace() is invoked at the first sqlite3_step()
** for the evaluation of an SQL statement.  The function registered by
** sqlite3_profile() runs at the end of each SQL statement and includes
** information on how long that statement ran.
**
** The sqlite3_profile() API is currently considered experimental and
** is subject to change.
*/
void *sqlite3_trace(sqlite3*, void(*xTrace)(void*,const char*), void*);
void *sqlite3_profile(sqlite3*,
   void(*xProfile)(void*,const char*,sqlite_uint64), void*);

/*
** This routine configures a callback function - the progress callback - that
** is invoked periodically during long running calls to sqlite3_exec(),
** sqlite3_step() and sqlite3_get_table(). An example use for this API is to 
** keep a GUI updated during a large query.
**
** The progress callback is invoked once for every N virtual machine opcodes,
** where N is the second argument to this function. The progress callback
** itself is identified by the third argument to this function. The fourth
** argument to this function is a void pointer passed to the progress callback
** function each time it is invoked.
**
** If a call to sqlite3_exec(), sqlite3_step() or sqlite3_get_table() results 
** in less than N opcodes being executed, then the progress callback is not
** invoked.
** 
** To remove the progress callback altogether, pass NULL as the third
** argument to this function.
**
** If the progress callback returns a result other than 0, then the current 
** query is immediately terminated and any database changes rolled back. If the
** query was part of a larger transaction, then the transaction is not rolled
** back and remains active. The sqlite3_exec() call returns SQLITE_ABORT. 
**
******* THIS IS AN EXPERIMENTAL API AND IS SUBJECT TO CHANGE ******
*/
void sqlite3_progress_handler(sqlite3*, int, int(*)(void*), void*);

/*
** Register a callback function to be invoked whenever a new transaction
** is committed.  The pArg argument is passed through to the callback.
** callback.  If the callback function returns non-zero, then the commit
** is converted into a rollback.
**
** If another function was previously registered, its pArg value is returned.
** Otherwise NULL is returned.
**
** Registering a NULL function disables the callback.
**
******* THIS IS AN EXPERIMENTAL API AND IS SUBJECT TO CHANGE ******
*/
void *sqlite3_commit_hook(sqlite3*, int(*)(void*), void*);

/*
** Open the sqlite database file "filename".  The "filename" is UTF-8
** encoded for sqlite3_open() and UTF-16 encoded in the native byte order
** for sqlite3_open16().  An sqlite3* handle is returned in *ppDb, even
** if an error occurs. If the database is opened (or created) successfully,
** then SQLITE_OK is returned. Otherwise an error code is returned. The
** sqlite3_errmsg() or sqlite3_errmsg16()  routines can be used to obtain
** an English language description of the error.
**
** If the database file does not exist, then a new database is created.
** The encoding for the database is UTF-8 if sqlite3_open() is called and
** UTF-16 if sqlite3_open16 is used.
**
** Whether or not an error occurs when it is opened, resources associated
** with the sqlite3* handle should be released by passing it to
** sqlite3_close() when it is no longer required.
*/
int sqlite3_open(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
);
int sqlite3_open16(
  const void *filename,   /* Database filename (UTF-16) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
);

/*
** Return the error code for the most recent sqlite3_* API call associated
** with sqlite3 handle 'db'. SQLITE_OK is returned if the most recent 
** API call was successful.
**
** Calls to many sqlite3_* functions set the error code and string returned
** by sqlite3_errcode(), sqlite3_errmsg() and sqlite3_errmsg16()
** (overwriting the previous values). Note that calls to sqlite3_errcode(),
** sqlite3_errmsg() and sqlite3_errmsg16() themselves do not affect the
** results of future invocations.
**
** Assuming no other intervening sqlite3_* API calls are made, the error
** code returned by this function is associated with the same error as
** the strings  returned by sqlite3_errmsg() and sqlite3_errmsg16().
*/
int sqlite3_errcode(sqlite3 *db);

/*
** Return a pointer to a UTF-8 encoded string describing in english the
** error condition for the most recent sqlite3_* API call. The returned
** string is always terminated by an 0x00 byte.
**
** The string "not an error" is returned when the most recent API call was
** successful.
*/
const char *sqlite3_errmsg(sqlite3*);

/*
** Return a pointer to a UTF-16 native byte order encoded string describing
** in english the error condition for the most recent sqlite3_* API call.
** The returned string is always terminated by a pair of 0x00 bytes.
**
** The string "not an error" is returned when the most recent API call was
** successful.
*/
const void *sqlite3_errmsg16(sqlite3*);

/*
** An instance of the following opaque structure is used to represent
** a compiled SQL statment.
*/
typedef struct sqlite3_stmt sqlite3_stmt;

/*
** To execute an SQL query, it must first be compiled into a byte-code
** program using one of the following routines. The only difference between
** them is that the second argument, specifying the SQL statement to
** compile, is assumed to be encoded in UTF-8 for the sqlite3_prepare()
** function and UTF-16 for sqlite3_prepare16().
**
** The first parameter "db" is an SQLite database handle. The second
** parameter "zSql" is the statement to be compiled, encoded as either
** UTF-8 or UTF-16 (see above). If the next parameter, "nBytes", is less
** than zero, then zSql is read up to the first nul terminator.  If
** "nBytes" is not less than zero, then it is the length of the string zSql
** in bytes (not characters).
**
** *pzTail is made to point to the first byte past the end of the first
** SQL statement in zSql.  This routine only compiles the first statement
** in zSql, so *pzTail is left pointing to what remains uncompiled.
**
** *ppStmt is left pointing to a compiled SQL statement that can be
** executed using sqlite3_step().  Or if there is an error, *ppStmt may be
** set to NULL.  If the input text contained no SQL (if the input is and
** empty string or a comment) then *ppStmt is set to NULL.
**
** On success, SQLITE_OK is returned.  Otherwise an error code is returned.
*/
int sqlite3_prepare(
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nBytes,             /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
);
int sqlite3_prepare16(
  sqlite3 *db,            /* Database handle */
  const void *zSql,       /* SQL statement, UTF-16 encoded */
  int nBytes,             /* Length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const void **pzTail     /* OUT: Pointer to unused portion of zSql */
);

/*
** Pointers to the following two opaque structures are used to communicate
** with the implementations of user-defined functions.
*/
typedef struct sqlite3_context sqlite3_context;
typedef struct Mem sqlite3_value;

/*
** In the SQL strings input to sqlite3_prepare() and sqlite3_prepare16(),
** one or more literals can be replace by parameters "?" or ":AAA" or
** "$VVV" where AAA is an identifer and VVV is a variable name according
** to the syntax rules of the TCL programming language.
** The value of these parameters (also called "host parameter names") can
** be set using the routines listed below.
**
** In every case, the first parameter is a pointer to the sqlite3_stmt
** structure returned from sqlite3_prepare().  The second parameter is the
** index of the parameter.  The first parameter as an index of 1.  For
** named parameters (":AAA" or "$VVV") you can use 
** sqlite3_bind_parameter_index() to get the correct index value given
** the parameters name.  If the same named parameter occurs more than
** once, it is assigned the same index each time.
**
** The fifth parameter to sqlite3_bind_blob(), sqlite3_bind_text(), and
** sqlite3_bind_text16() is a destructor used to dispose of the BLOB or
** text after SQLite has finished with it.  If the fifth argument is the
** special value SQLITE_STATIC, then the library assumes that the information
** is in static, unmanaged space and does not need to be freed.  If the
** fifth argument has the value SQLITE_TRANSIENT, then SQLite makes its
** own private copy of the data.
**
** The sqlite3_bind_* routine must be called before sqlite3_step() after
** an sqlite3_prepare() or sqlite3_reset().  Unbound parameterss are
** interpreted as NULL.
*/
int sqlite3_bind_blob(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
int sqlite3_bind_double(sqlite3_stmt*, int, double);
int sqlite3_bind_int(sqlite3_stmt*, int, int);
int sqlite3_bind_int64(sqlite3_stmt*, int, sqlite_int64);
int sqlite3_bind_null(sqlite3_stmt*, int);
int sqlite3_bind_text(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
int sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int, void(*)(void*));
int sqlite3_bind_value(sqlite3_stmt*, int, const sqlite3_value*);

/*
** Return the number of parameters in a compiled SQL statement.  This
** routine was added to support DBD::SQLite.
*/
int sqlite3_bind_parameter_count(sqlite3_stmt*);

/*
** Return the name of the i-th parameter.  Ordinary parameters "?" are
** nameless and a NULL is returned.  For parameters of the form :AAA or
** $VVV the complete text of the parameter name is returned, including
** the initial ":" or "$".  NULL is returned if the index is out of range.
*/
const char *sqlite3_bind_parameter_name(sqlite3_stmt*, int);

/*
** Return the index of a parameter with the given name.  The name
** must match exactly.  If no parameter with the given name is found,
** return 0.
*/
int sqlite3_bind_parameter_index(sqlite3_stmt*, const char *zName);

/*
** Set all the parameters in the compiled SQL statement to NULL.
*/
int sqlite3_clear_bindings(sqlite3_stmt*);

/*
** Return the number of columns in the result set returned by the compiled
** SQL statement. This routine returns 0 if pStmt is an SQL statement
** that does not return data (for example an UPDATE).
*/
int sqlite3_column_count(sqlite3_stmt *pStmt);

/*
** The first parameter is a compiled SQL statement. This function returns
** the column heading for the Nth column of that statement, where N is the
** second function parameter.  The string returned is UTF-8 for
** sqlite3_column_name() and UTF-16 for sqlite3_column_name16().
*/
const char *sqlite3_column_name(sqlite3_stmt*,int);
const void *sqlite3_column_name16(sqlite3_stmt*,int);

/*
** The first parameter to the following calls is a compiled SQL statement.
** These functions return information about the Nth column returned by 
** the statement, where N is the second function argument.
**
** If the Nth column returned by the statement is not a column value,
** then all of the functions return NULL. Otherwise, the return the 
** name of the attached database, table and column that the expression
** extracts a value from.
**
** As with all other SQLite APIs, those postfixed with "16" return UTF-16
** encoded strings, the other functions return UTF-8. The memory containing
** the returned strings is valid until the statement handle is finalized().
**
** These APIs are only available if the library was compiled with the 
** SQLITE_ENABLE_COLUMN_METADATA preprocessor symbol defined.
*/
const char *sqlite3_column_database_name(sqlite3_stmt*,int);
const void *sqlite3_column_database_name16(sqlite3_stmt*,int);
const char *sqlite3_column_table_name(sqlite3_stmt*,int);
const void *sqlite3_column_table_name16(sqlite3_stmt*,int);
const char *sqlite3_column_origin_name(sqlite3_stmt*,int);
const void *sqlite3_column_origin_name16(sqlite3_stmt*,int);

/*
** The first parameter is a compiled SQL statement. If this statement
** is a SELECT statement, the Nth column of the returned result set 
** of the SELECT is a table column then the declared type of the table
** column is returned. If the Nth column of the result set is not at table
** column, then a NULL pointer is returned. The returned string is always
** UTF-8 encoded. For example, in the database schema:
**
** CREATE TABLE t1(c1 VARIANT);
**
** And the following statement compiled:
**
** SELECT c1 + 1, c1 FROM t1;
**
** Then this routine would return the string "VARIANT" for the second
** result column (i==1), and a NULL pointer for the first result column
** (i==0).
*/
const char *sqlite3_column_decltype(sqlite3_stmt *, int i);

/*
** The first parameter is a compiled SQL statement. If this statement
** is a SELECT statement, the Nth column of the returned result set 
** of the SELECT is a table column then the declared type of the table
** column is returned. If the Nth column of the result set is not at table
** column, then a NULL pointer is returned. The returned string is always
** UTF-16 encoded. For example, in the database schema:
**
** CREATE TABLE t1(c1 INTEGER);
**
** And the following statement compiled:
**
** SELECT c1 + 1, c1 FROM t1;
**
** Then this routine would return the string "INTEGER" for the second
** result column (i==1), and a NULL pointer for the first result column
** (i==0).
*/
const void *sqlite3_column_decltype16(sqlite3_stmt*,int);

/* 
** After an SQL query has been compiled with a call to either
** sqlite3_prepare() or sqlite3_prepare16(), then this function must be
** called one or more times to execute the statement.
**
** The return value will be either SQLITE_BUSY, SQLITE_DONE, 
** SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE.
**
** SQLITE_BUSY means that the database engine attempted to open
** a locked database and there is no busy callback registered.
** Call sqlite3_step() again to retry the open.
**
** SQLITE_DONE means that the statement has finished executing
** successfully.  sqlite3_step() should not be called again on this virtual
** machine.
**
** If the SQL statement being executed returns any data, then 
** SQLITE_ROW is returned each time a new row of data is ready
** for processing by the caller. The values may be accessed using
** the sqlite3_column_*() functions described below. sqlite3_step()
** is called again to retrieve the next row of data.
** 
** SQLITE_ERROR means that a run-time error (such as a constraint
** violation) has occurred.  sqlite3_step() should not be called again on
** the VM. More information may be found by calling sqlite3_errmsg().
**
** SQLITE_MISUSE means that the this routine was called inappropriately.
** Perhaps it was called on a virtual machine that had already been
** finalized or on one that had previously returned SQLITE_ERROR or
** SQLITE_DONE.  Or it could be the case the the same database connection
** is being used simulataneously by two or more threads.
*/
int sqlite3_step(sqlite3_stmt*);

/*
** Return the number of values in the current row of the result set.
**
** After a call to sqlite3_step() that returns SQLITE_ROW, this routine
** will return the same value as the sqlite3_column_count() function.
** After sqlite3_step() has returned an SQLITE_DONE, SQLITE_BUSY or
** error code, or before sqlite3_step() has been called on a 
** compiled SQL statement, this routine returns zero.
*/
int sqlite3_data_count(sqlite3_stmt *pStmt);

/*
** Values are stored in the database in one of the following fundamental
** types.
*/
#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
/* #define SQLITE_TEXT  3  // See below */
#define SQLITE_BLOB     4
#define SQLITE_NULL     5

/*
** SQLite version 2 defines SQLITE_TEXT differently.  To allow both
** version 2 and version 3 to be included, undefine them both if a
** conflict is seen.  Define SQLITE3_TEXT to be the version 3 value.
*/
#ifdef SQLITE_TEXT
# undef SQLITE_TEXT
#else
# define SQLITE_TEXT     3
#endif
#define SQLITE3_TEXT     3

/*
** The next group of routines returns information about the information
** in a single column of the current result row of a query.  In every
** case the first parameter is a pointer to the SQL statement that is being
** executed (the sqlite_stmt* that was returned from sqlite3_prepare()) and
** the second argument is the index of the column for which information 
** should be returned.  iCol is zero-indexed.  The left-most column as an
** index of 0.
**
** If the SQL statement is not currently point to a valid row, or if the
** the colulmn index is out of range, the result is undefined.
**
** These routines attempt to convert the value where appropriate.  For
** example, if the internal representation is FLOAT and a text result
** is requested, sprintf() is used internally to do the conversion
** automatically.  The following table details the conversions that
** are applied:
**
**    Internal Type    Requested Type     Conversion
**    -------------    --------------    --------------------------
**       NULL             INTEGER         Result is 0
**       NULL             FLOAT           Result is 0.0
**       NULL             TEXT            Result is an empty string
**       NULL             BLOB            Result is a zero-length BLOB
**       INTEGER          FLOAT           Convert from integer to float
**       INTEGER          TEXT            ASCII rendering of the integer
**       INTEGER          BLOB            Same as for INTEGER->TEXT
**       FLOAT            INTEGER         Convert from float to integer
**       FLOAT            TEXT            ASCII rendering of the float
**       FLOAT            BLOB            Same as FLOAT->TEXT
**       TEXT             INTEGER         Use atoi()
**       TEXT             FLOAT           Use atof()
**       TEXT             BLOB            No change
**       BLOB             INTEGER         Convert to TEXT then use atoi()
**       BLOB             FLOAT           Convert to TEXT then use atof()
**       BLOB             TEXT            Add a \000 terminator if needed
**
** The following access routines are provided:
**
** _type()     Return the datatype of the result.  This is one of
**             SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB,
**             or SQLITE_NULL.
** _blob()     Return the value of a BLOB.
** _bytes()    Return the number of bytes in a BLOB value or the number
**             of bytes in a TEXT value represented as UTF-8.  The \000
**             terminator is included in the byte count for TEXT values.
** _bytes16()  Return the number of bytes in a BLOB value or the number
**             of bytes in a TEXT value represented as UTF-16.  The \u0000
**             terminator is included in the byte count for TEXT values.
** _double()   Return a FLOAT value.
** _int()      Return an INTEGER value in the host computer's native
**             integer representation.  This might be either a 32- or 64-bit
**             integer depending on the host.
** _int64()    Return an INTEGER value as a 64-bit signed integer.
** _text()     Return the value as UTF-8 text.
** _text16()   Return the value as UTF-16 text.
*/
const void *sqlite3_column_blob(sqlite3_stmt*, int iCol);
int sqlite3_column_bytes(sqlite3_stmt*, int iCol);
int sqlite3_column_bytes16(sqlite3_stmt*, int iCol);
double sqlite3_column_double(sqlite3_stmt*, int iCol);
int sqlite3_column_int(sqlite3_stmt*, int iCol);
sqlite_int64 sqlite3_column_int64(sqlite3_stmt*, int iCol);
const unsigned char *sqlite3_column_text(sqlite3_stmt*, int iCol);
const void *sqlite3_column_text16(sqlite3_stmt*, int iCol);
int sqlite3_column_type(sqlite3_stmt*, int iCol);
int sqlite3_column_numeric_type(sqlite3_stmt*, int iCol);

/*
** The sqlite3_finalize() function is called to delete a compiled
** SQL statement obtained by a previous call to sqlite3_prepare()
** or sqlite3_prepare16(). If the statement was executed successfully, or
** not executed at all, then SQLITE_OK is returned. If execution of the
** statement failed then an error code is returned. 
**
** This routine can be called at any point during the execution of the
** virtual machine.  If the virtual machine has not completed execution
** when this routine is called, that is like encountering an error or
** an interrupt.  (See sqlite3_interrupt().)  Incomplete updates may be
** rolled back and transactions cancelled,  depending on the circumstances,
** and the result code returned will be SQLITE_ABORT.
*/
int sqlite3_finalize(sqlite3_stmt *pStmt);

/*
** The sqlite3_reset() function is called to reset a compiled SQL
** statement obtained by a previous call to sqlite3_prepare() or
** sqlite3_prepare16() back to it's initial state, ready to be re-executed.
** Any SQL statement variables that had values bound to them using
** the sqlite3_bind_*() API retain their values.
*/
int sqlite3_reset(sqlite3_stmt *pStmt);

/*
** The following two functions are used to add user functions or aggregates
** implemented in C to the SQL langauge interpreted by SQLite. The
** difference only between the two is that the second parameter, the
** name of the (scalar) function or aggregate, is encoded in UTF-8 for
** sqlite3_create_function() and UTF-16 for sqlite3_create_function16().
**
** The first argument is the database handle that the new function or
** aggregate is to be added to. If a single program uses more than one
** database handle internally, then user functions or aggregates must 
** be added individually to each database handle with which they will be
** used.
**
** The third parameter is the number of arguments that the function or
** aggregate takes. If this parameter is negative, then the function or
** aggregate may take any number of arguments.
**
** The fourth parameter is one of SQLITE_UTF* values defined below,
** indicating the encoding that the function is most likely to handle
** values in.  This does not change the behaviour of the programming
** interface. However, if two versions of the same function are registered
** with different encoding values, SQLite invokes the version likely to
** minimize conversions between text encodings.
**
** The seventh, eighth and ninth parameters, xFunc, xStep and xFinal, are
** pointers to user implemented C functions that implement the user
** function or aggregate. A scalar function requires an implementation of
** the xFunc callback only, NULL pointers should be passed as the xStep
** and xFinal parameters. An aggregate function requires an implementation
** of xStep and xFinal, but NULL should be passed for xFunc. To delete an
** existing user function or aggregate, pass NULL for all three function
** callback. Specifying an inconstent set of callback values, such as an
** xFunc and an xFinal, or an xStep but no xFinal, SQLITE_ERROR is
** returned.
*/
int sqlite3_create_function(
  sqlite3 *,
  const char *zFunctionName,
  int nArg,
  int eTextRep,
  void*,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
);
int sqlite3_create_function16(
  sqlite3*,
  const void *zFunctionName,
  int nArg,
  int eTextRep,
  void*,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
);

/*
** This function is deprecated.  Do not use it.  It continues to exist
** so as not to break legacy code.  But new code should avoid using it.
*/
int sqlite3_aggregate_count(sqlite3_context*);

/*
** The next group of routines returns information about parameters to
** a user-defined function.  Function implementations use these routines
** to access their parameters.  These routines are the same as the
** sqlite3_column_* routines except that these routines take a single
** sqlite3_value* pointer instead of an sqlite3_stmt* and an integer
** column number.
*/
const void *sqlite3_value_blob(sqlite3_value*);
int sqlite3_value_bytes(sqlite3_value*);
int sqlite3_value_bytes16(sqlite3_value*);
double sqlite3_value_double(sqlite3_value*);
int sqlite3_value_int(sqlite3_value*);
sqlite_int64 sqlite3_value_int64(sqlite3_value*);
const unsigned char *sqlite3_value_text(sqlite3_value*);
const void *sqlite3_value_text16(sqlite3_value*);
const void *sqlite3_value_text16le(sqlite3_value*);
const void *sqlite3_value_text16be(sqlite3_value*);
int sqlite3_value_type(sqlite3_value*);
int sqlite3_value_numeric_type(sqlite3_value*);

/*
** Aggregate functions use the following routine to allocate
** a structure for storing their state.  The first time this routine
** is called for a particular aggregate, a new structure of size nBytes
** is allocated, zeroed, and returned.  On subsequent calls (for the
** same aggregate instance) the same buffer is returned.  The implementation
** of the aggregate can use the returned buffer to accumulate data.
**
** The buffer allocated is freed automatically by SQLite.
*/
void *sqlite3_aggregate_context(sqlite3_context*, int nBytes);

/*
** The pUserData parameter to the sqlite3_create_function()
** routine used to register user functions is available to
** the implementation of the function using this call.
*/
void *sqlite3_user_data(sqlite3_context*);

/*
** The following two functions may be used by scalar user functions to
** associate meta-data with argument values. If the same value is passed to
** multiple invocations of the user-function during query execution, under
** some circumstances the associated meta-data may be preserved. This may
** be used, for example, to add a regular-expression matching scalar
** function. The compiled version of the regular expression is stored as
** meta-data associated with the SQL value passed as the regular expression
** pattern.
**
** Calling sqlite3_get_auxdata() returns a pointer to the meta data
** associated with the Nth argument value to the current user function
** call, where N is the second parameter. If no meta-data has been set for
** that value, then a NULL pointer is returned.
**
** The sqlite3_set_auxdata() is used to associate meta data with a user
** function argument. The third parameter is a pointer to the meta data
** to be associated with the Nth user function argument value. The fourth
** parameter specifies a 'delete function' that will be called on the meta
** data pointer to release it when it is no longer required. If the delete
** function pointer is NULL, it is not invoked.
**
** In practice, meta-data is preserved between function calls for
** expressions that are constant at compile time. This includes literal
** values and SQL variables.
*/
void *sqlite3_get_auxdata(sqlite3_context*, int);
void sqlite3_set_auxdata(sqlite3_context*, int, void*, void (*)(void*));


/*
** These are special value for the destructor that is passed in as the
** final argument to routines like sqlite3_result_blob().  If the destructor
** argument is SQLITE_STATIC, it means that the content pointer is constant
** and will never change.  It does not need to be destroyed.  The 
** SQLITE_TRANSIENT value means that the content will likely change in
** the near future and that SQLite should make its own private copy of
** the content before returning.
*/
#define SQLITE_STATIC      ((void(*)(void *))0)
#define SQLITE_TRANSIENT   ((void(*)(void *))-1)

/*
** User-defined functions invoke the following routines in order to
** set their return value.
*/
void sqlite3_result_blob(sqlite3_context*, const void*, int, void(*)(void*));
void sqlite3_result_double(sqlite3_context*, double);
void sqlite3_result_error(sqlite3_context*, const char*, int);
void sqlite3_result_error16(sqlite3_context*, const void*, int);
void sqlite3_result_int(sqlite3_context*, int);
void sqlite3_result_int64(sqlite3_context*, sqlite_int64);
void sqlite3_result_null(sqlite3_context*);
void sqlite3_result_text(sqlite3_context*, const char*, int, void(*)(void*));
void sqlite3_result_text16(sqlite3_context*, const void*, int, void(*)(void*));
void sqlite3_result_text16le(sqlite3_context*, const void*, int,void(*)(void*));
void sqlite3_result_text16be(sqlite3_context*, const void*, int,void(*)(void*));
void sqlite3_result_value(sqlite3_context*, sqlite3_value*);

/*
** These are the allowed values for the eTextRep argument to
** sqlite3_create_collation and sqlite3_create_function.
*/
#define SQLITE_UTF8           1
#define SQLITE_UTF16LE        2
#define SQLITE_UTF16BE        3
#define SQLITE_UTF16          4    /* Use native byte order */
#define SQLITE_ANY            5    /* sqlite3_create_function only */
#define SQLITE_UTF16_ALIGNED  8    /* sqlite3_create_collation only */

/*
** These two functions are used to add new collation sequences to the
** sqlite3 handle specified as the first argument. 
**
** The name of the new collation sequence is specified as a UTF-8 string
** for sqlite3_create_collation() and a UTF-16 string for
** sqlite3_create_collation16(). In both cases the name is passed as the
** second function argument.
**
** The third argument must be one of the constants SQLITE_UTF8,
** SQLITE_UTF16LE or SQLITE_UTF16BE, indicating that the user-supplied
** routine expects to be passed pointers to strings encoded using UTF-8,
** UTF-16 little-endian or UTF-16 big-endian respectively.
**
** A pointer to the user supplied routine must be passed as the fifth
** argument. If it is NULL, this is the same as deleting the collation
** sequence (so that SQLite cannot call it anymore). Each time the user
** supplied function is invoked, it is passed a copy of the void* passed as
** the fourth argument to sqlite3_create_collation() or
** sqlite3_create_collation16() as its first parameter.
**
** The remaining arguments to the user-supplied routine are two strings,
** each represented by a [length, data] pair and encoded in the encoding
** that was passed as the third argument when the collation sequence was
** registered. The user routine should return negative, zero or positive if
** the first string is less than, equal to, or greater than the second
** string. i.e. (STRING1 - STRING2).
*/
int sqlite3_create_collation(
  sqlite3*, 
  const char *zName, 
  int eTextRep, 
  void*,
  int(*xCompare)(void*,int,const void*,int,const void*)
);
int sqlite3_create_collation16(
  sqlite3*, 
  const char *zName, 
  int eTextRep, 
  void*,
  int(*xCompare)(void*,int,const void*,int,const void*)
);

/*
** To avoid having to register all collation sequences before a database
** can be used, a single callback function may be registered with the
** database handle to be called whenever an undefined collation sequence is
** required.
**
** If the function is registered using the sqlite3_collation_needed() API,
** then it is passed the names of undefined collation sequences as strings
** encoded in UTF-8. If sqlite3_collation_needed16() is used, the names
** are passed as UTF-16 in machine native byte order. A call to either
** function replaces any existing callback.
**
** When the user-function is invoked, the first argument passed is a copy
** of the second argument to sqlite3_collation_needed() or
** sqlite3_collation_needed16(). The second argument is the database
** handle. The third argument is one of SQLITE_UTF8, SQLITE_UTF16BE or
** SQLITE_UTF16LE, indicating the most desirable form of the collation
** sequence function required. The fourth parameter is the name of the
** required collation sequence.
**
** The collation sequence is returned to SQLite by a collation-needed
** callback using the sqlite3_create_collation() or
** sqlite3_create_collation16() APIs, described above.
*/
int sqlite3_collation_needed(
  sqlite3*, 
  void*, 
  void(*)(void*,sqlite3*,int eTextRep,const char*)
);
int sqlite3_collation_needed16(
  sqlite3*, 
  void*,
  void(*)(void*,sqlite3*,int eTextRep,const void*)
);

/*
** Specify the key for an encrypted database.  This routine should be
** called right after sqlite3_open().
**
** The code to implement this API is not available in the public release
** of SQLite.
*/
int sqlite3_key(
  sqlite3 *db,                   /* Database to be rekeyed */
  const void *pKey, int nKey     /* The key */
);

/*
** Change the key on an open database.  If the current database is not
** encrypted, this routine will encrypt it.  If pNew==0 or nNew==0, the
** database is decrypted.
**
** The code to implement this API is not available in the public release
** of SQLite.
*/
int sqlite3_rekey(
  sqlite3 *db,                   /* Database to be rekeyed */
  const void *pKey, int nKey     /* The new key */
);

/*
** Sleep for a little while. The second parameter is the number of
** miliseconds to sleep for. 
**
** If the operating system does not support sleep requests with 
** milisecond time resolution, then the time will be rounded up to 
** the nearest second. The number of miliseconds of sleep actually 
** requested from the operating system is returned.
*/
int sqlite3_sleep(int);

/*
** Return TRUE (non-zero) if the statement supplied as an argument needs
** to be recompiled.  A statement needs to be recompiled whenever the
** execution environment changes in a way that would alter the program
** that sqlite3_prepare() generates.  For example, if new functions or
** collating sequences are registered or if an authorizer function is
** added or changed.
**
*/
int sqlite3_expired(sqlite3_stmt*);

/*
** Move all bindings from the first prepared statement over to the second.
** This routine is useful, for example, if the first prepared statement
** fails with an SQLITE_SCHEMA error.  The same SQL can be prepared into
** the second prepared statement then all of the bindings transfered over
** to the second statement before the first statement is finalized.
*/
int sqlite3_transfer_bindings(sqlite3_stmt*, sqlite3_stmt*);

/*
** If the following global variable is made to point to a
** string which is the name of a directory, then all temporary files
** created by SQLite will be placed in that directory.  If this variable
** is NULL pointer, then SQLite does a search for an appropriate temporary
** file directory.
**
** Once sqlite3_open() has been called, changing this variable will invalidate
** the current temporary database, if any.
*/
extern char *sqlite3_temp_directory;

/*
** This function is called to recover from a malloc() failure that occured
** within the SQLite library. Normally, after a single malloc() fails the 
** library refuses to function (all major calls return SQLITE_NOMEM).
** This function restores the library state so that it can be used again.
**
** All existing statements (sqlite3_stmt pointers) must be finalized or
** reset before this call is made. Otherwise, SQLITE_BUSY is returned.
** If any in-memory databases are in use, either as a main or TEMP
** database, SQLITE_ERROR is returned. In either of these cases, the 
** library is not reset and remains unusable.
**
** This function is *not* threadsafe. Calling this from within a threaded
** application when threads other than the caller have used SQLite is
** dangerous and will almost certainly result in malfunctions.
**
** This functionality can be omitted from a build by defining the 
** SQLITE_OMIT_GLOBALRECOVER at compile time.
*/
int sqlite3_global_recover(void);

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
int sqlite3_get_autocommit(sqlite3*);

/*
** Return the sqlite3* database handle to which the prepared statement given
** in the argument belongs.  This is the same database handle that was
** the first argument to the sqlite3_prepare() that was used to create
** the statement in the first place.
*/
sqlite3 *sqlite3_db_handle(sqlite3_stmt*);

/*
** Register a callback function with the database connection identified by the 
** first argument to be invoked whenever a row is updated, inserted or deleted.
** Any callback set by a previous call to this function for the same 
** database connection is overridden.
**
** The second argument is a pointer to the function to invoke when a 
** row is updated, inserted or deleted. The first argument to the callback is
** a copy of the third argument to sqlite3_update_hook. The second callback 
** argument is one of SQLITE_INSERT, SQLITE_DELETE or SQLITE_UPDATE, depending
** on the operation that caused the callback to be invoked. The third and 
** fourth arguments to the callback contain pointers to the database and 
** table name containing the affected row. The final callback parameter is 
** the rowid of the row. In the case of an update, this is the rowid after 
** the update takes place.
**
** The update hook is not invoked when internal system tables are
** modified (i.e. sqlite_master and sqlite_sequence).
**
** If another function was previously registered, its pArg value is returned.
** Otherwise NULL is returned.
*/
void *sqlite3_update_hook(
  sqlite3*, 
  void(*)(void *,int ,char const *,char const *,sqlite_int64),
  void*
);

/*
** Register a callback to be invoked whenever a transaction is rolled
** back. 
**
** The new callback function overrides any existing rollback-hook
** callback. If there was an existing callback, then it's pArg value 
** (the third argument to sqlite3_rollback_hook() when it was registered) 
** is returned. Otherwise, NULL is returned.
**
** For the purposes of this API, a transaction is said to have been 
** rolled back if an explicit "ROLLBACK" statement is executed, or
** an error or constraint causes an implicit rollback to occur. The 
** callback is not invoked if a transaction is automatically rolled
** back because the database connection is closed.
*/
void *sqlite3_rollback_hook(sqlite3*, void(*)(void *), void*);

/*
** This function is only available if the library is compiled without
** the SQLITE_OMIT_SHARED_CACHE macro defined. It is used to enable or
** disable (if the argument is true or false, respectively) the 
** "shared pager" feature.
*/
int sqlite3_enable_shared_cache(int);

/*
** Attempt to free N bytes of heap memory by deallocating non-essential
** memory allocations held by the database library (example: memory 
** used to cache database pages to improve performance).
**
** This function is not a part of standard builds.  It is only created
** if SQLite is compiled with the SQLITE_ENABLE_MEMORY_MANAGEMENT macro.
*/
int sqlite3_release_memory(int);

/*
** Place a "soft" limit on the amount of heap memory that may be allocated by
** SQLite within the current thread. If an internal allocation is requested 
** that would exceed the specified limit, sqlite3_release_memory() is invoked
** one or more times to free up some space before the allocation is made.
**
** The limit is called "soft", because if sqlite3_release_memory() cannot free
** sufficient memory to prevent the limit from being exceeded, the memory is
** allocated anyway and the current operation proceeds.
**
** This function is only available if the library was compiled with the 
** SQLITE_ENABLE_MEMORY_MANAGEMENT option set.
** memory-management has been enabled.
*/
void sqlite3_soft_heap_limit(int);

/*
** This routine makes sure that all thread-local storage has been
** deallocated for the current thread.
**
** This routine is not technically necessary.  All thread-local storage
** will be automatically deallocated once memory-management and
** shared-cache are disabled and the soft heap limit has been set
** to zero.  This routine is provided as a convenience for users who
** want to make absolutely sure they have not forgotten something
** prior to killing off a thread.
*/
void sqlite3_thread_cleanup(void);

/*
** Return meta information about a specific column of a specific database
** table accessible using the connection handle passed as the first function 
** argument.
**
** The column is identified by the second, third and fourth parameters to 
** this function. The second parameter is either the name of the database
** (i.e. "main", "temp" or an attached database) containing the specified
** table or NULL. If it is NULL, then all attached databases are searched
** for the table using the same algorithm as the database engine uses to 
** resolve unqualified table references.
**
** The third and fourth parameters to this function are the table and column 
** name of the desired column, respectively. Neither of these parameters 
** may be NULL.
**
** Meta information is returned by writing to the memory locations passed as
** the 5th and subsequent parameters to this function. Any of these 
** arguments may be NULL, in which case the corresponding element of meta 
** information is ommitted.
**
** Parameter     Output Type      Description
** -----------------------------------
**
**   5th         const char*      Data type
**   6th         const char*      Name of the default collation sequence 
**   7th         int              True if the column has a NOT NULL constraint
**   8th         int              True if the column is part of the PRIMARY KEY
**   9th         int              True if the column is AUTOINCREMENT
**
**
** The memory pointed to by the character pointers returned for the 
** declaration type and collation sequence is valid only until the next 
** call to any sqlite API function.
**
** If the specified table is actually a view, then an error is returned.
**
** If the specified column is "rowid", "oid" or "_rowid_" and an 
** INTEGER PRIMARY KEY column has been explicitly declared, then the output 
** parameters are set for the explicitly declared column. If there is no
** explicitly declared IPK column, then the output parameters are set as 
** follows:
**
**     data type: "INTEGER"
**     collation sequence: "BINARY"
**     not null: 0
**     primary key: 1
**     auto increment: 0
**
** This function may load one or more schemas from database files. If an
** error occurs during this process, or if the requested table or column
** cannot be found, an SQLITE error code is returned and an error message
** left in the database handle (to be retrieved using sqlite3_errmsg()).
**
** This API is only available if the library was compiled with the
** SQLITE_ENABLE_COLUMN_METADATA preprocessor symbol defined.
*/
int sqlite3_table_column_metadata(
  sqlite3 *db,                /* Connection handle */
  const char *zDbName,        /* Database name or NULL */
  const char *zTableName,     /* Table name */
  const char *zColumnName,    /* Column name */
  char const **pzDataType,    /* OUTPUT: Declared data type */
  char const **pzCollSeq,     /* OUTPUT: Collation sequence name */
  int *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  int *pPrimaryKey,           /* OUTPUT: True if column part of PK */
  int *pAutoinc               /* OUTPUT: True if colums is auto-increment */
);

/*
** Undo the hack that converts floating point types to integer for
** builds on processors without floating point support.
*/
#ifdef SQLITE_OMIT_FLOATING_POINT
# undef double
#endif

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif
#endif
/************************************from the file sqliteInt.h*********************************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Internal interface definitions for SQLite.
**
** @(#) $Id: sqliteInt.h,v 1.493 2006/04/04 01:54:55 drh Exp $
*/
#ifndef _SQLITEINT_H_
#define _SQLITEINT_H_

#ifdef __cplusplus
extern "C" {
#endif
/*
** Extra interface definitions for those who need them
*/
#ifdef SQLITE_EXTRA
# include "sqliteExtra.h"
#endif

/*
** Many people are failing to set -DNDEBUG=1 when compiling SQLite.
** Setting NDEBUG makes the code smaller and run faster.  So the following
** lines are added to automatically set NDEBUG unless the -DSQLITE_DEBUG=1
** option is set.  Thus NDEBUG becomes an opt-in rather than an opt-out
** feature.
*/
#if !defined(NDEBUG) && !defined(SQLITE_DEBUG) 
# define NDEBUG 1
#endif

/*
** These #defines should enable >2GB file support on Posix if the
** underlying operating system supports it.  If the OS lacks
** large file support, or if the OS is windows, these should be no-ops.
**
** Large file support can be disabled using the -DSQLITE_DISABLE_LFS switch
** on the compiler command line.  This is necessary if you are compiling
** on a recent machine (ex: RedHat 7.2) but you want your code to work
** on an older machine (ex: RedHat 6.0).  If you compile on RedHat 7.2
** without this option, LFS is enable.  But LFS does not exist in the kernel
** in RedHat 6.0, so the code won't work.  Hence, for maximum binary
** portability you should omit LFS.
**
** Similar is true for MacOS.  LFS is only supported on MacOS 9 and later.
*/
#ifndef SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
# endif
# define _LARGEFILE_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#ifdef SQLITE_OMIT_FLOATING_POINT
# define double sqlite_int64
# define LONGDOUBLE_TYPE sqlite_int64
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (0x7fffffffffffffff)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
#endif
#ifndef SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#endif

/*
** The maximum number of in-memory pages to use for the main database
** table and for temporary tables. Internally, the MAX_PAGES and 
** TEMP_PAGES macros are used. To override the default values at
** compilation time, the SQLITE_DEFAULT_CACHE_SIZE and 
** SQLITE_DEFAULT_TEMP_CACHE_SIZE macros should be set.
*/
#ifdef SQLITE_DEFAULT_CACHE_SIZE
# define MAX_PAGES SQLITE_DEFAULT_CACHE_SIZE
#else
# define MAX_PAGES   2000
#endif
#ifdef SQLITE_DEFAULT_TEMP_CACHE_SIZE
# define TEMP_PAGES SQLITE_DEFAULT_TEMP_CACHE_SIZE
#else
# define TEMP_PAGES   500
#endif

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler 
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#ifdef SQLITE_OMIT_TEMPDB
#define OMIT_TEMPDB 1
#else
#define OMIT_TEMPDB 0
#endif

/*
** If the following macro is set to 1, then NULL values are considered
** distinct when determining whether or not two entries are the same
** in a UNIQUE index.  This is the way PostgreSQL, Oracle, DB2, MySQL,
** OCELOT, and Firebird all work.  The SQL92 spec explicitly says this
** is the way things are suppose to work.
**
** If the following macro is set to 0, the NULLs are indistinct for
** a UNIQUE index.  In this mode, you can only have a single NULL entry
** for a column declared UNIQUE.  This is the way Informix and SQL Server
** work.
*/
#define NULL_DISTINCT_FOR_UNIQUE 1

/*
** The maximum number of attached databases.  This must be at least 2
** in order to support the main database file (0) and the file used to
** hold temporary tables (1).  And it must be less than 32 because
** we use a bitmask of databases with a u32 in places (for example
** the Parse.cookieMask field).
*/
#define MAX_ATTACHED 10

/*
** The maximum value of a ?nnn wildcard that the parser will accept.
*/
#define SQLITE_MAX_VARIABLE_NUMBER 999

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#define SQLITE_MAX_FILE_FORMAT 4
#ifndef SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#endif

/*
** Provide a default value for TEMP_STORE in case it is not specified
** on the command-line
*/
#ifndef TEMP_STORE
# define TEMP_STORE 1
#endif

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#ifndef offsetof
#define offsetof(STRUCTURE,FIELD) ((int)((char*)&((STRUCTURE*)0)->FIELD))
#endif

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#if 'A' == '\301'
# define SQLITE_EBCDIC 1
#else
# define SQLITE_ASCII 1
#endif

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#ifndef UINT32_TYPE
# define UINT32_TYPE unsigned int
#endif
#ifndef UINT16_TYPE
# define UINT16_TYPE unsigned short int
#endif
#ifndef INT16_TYPE
# define INT16_TYPE short int
#endif
#ifndef UINT8_TYPE
# define UINT8_TYPE unsigned char
#endif
#ifndef INT8_TYPE
# define INT8_TYPE signed char
#endif
#ifndef LONGDOUBLE_TYPE
# define LONGDOUBLE_TYPE long double
#endif
typedef sqlite_int64 i64;          /* 8-byte signed integer */
typedef sqlite_uint64 u64;         /* 8-byte unsigned integer */
typedef UINT32_TYPE u32;           /* 4-byte unsigned integer */
typedef UINT16_TYPE u16;           /* 2-byte unsigned integer */
typedef INT16_TYPE i16;            /* 2-byte signed integer */
typedef UINT8_TYPE u8;             /* 1-byte unsigned integer */
typedef UINT8_TYPE i8;             /* 1-byte signed integer */

/*
** Macros to determine whether the machine is big or little endian,
** evaluated at runtime.
*/
extern const int sqlite3one;
#define SQLITE_BIGENDIAN    (*(char *)(&sqlite3one)==0)
#define SQLITE_LITTLEENDIAN (*(char *)(&sqlite3one)==1)

/*
** An instance of the following structure is used to store the busy-handler
** callback for a given sqlite handle. 
**
** The sqlite.busyHandler member of the sqlite struct contains the busy
** callback for the database handle. Each pager opened via the sqlite
** handle is passed a pointer to sqlite.busyHandler. The busy-handler
** callback is currently invoked only from within pager.c.
*/
typedef struct BusyHandler BusyHandler;
struct BusyHandler {
  int (*xFunc)(void *,int);  /* The busy callback */
  void *pArg;                /* First arg to busy callback */
  int nBusy;                 /* Incremented with each busy call */
};

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and 
** "BusyHandler typedefs.
*/
//#include "vdbe.h"
//#include "btree.h"
//#include "pager.h"

#ifdef SQLITE_MEMDEBUG
/*
** The following global variables are used for testing and debugging
** only.  They only work if SQLITE_MEMDEBUG is defined.
*/
extern int sqlite3_nMalloc;      /* Number of sqliteMalloc() calls */
extern int sqlite3_nFree;        /* Number of sqliteFree() calls */
extern int sqlite3_iMallocFail;  /* Fail sqliteMalloc() after this many calls */
extern int sqlite3_iMallocReset; /* Set iMallocFail to this when it reaches 0 */

extern void *sqlite3_pFirst;         /* Pointer to linked list of allocations */
extern int sqlite3_nMaxAlloc;        /* High water mark of ThreadData.nAlloc */
extern int sqlite3_mallocDisallowed; /* assert() in sqlite3Malloc() if set */
extern int sqlite3_isFail;           /* True if all malloc calls should fail */
extern const char *sqlite3_zFile;    /* Filename to associate debug info with */
extern int sqlite3_iLine;            /* Line number for debug info */

#define ENTER_MALLOC (sqlite3_zFile = __FILE__, sqlite3_iLine = __LINE__)
#define sqliteMalloc(x)          (ENTER_MALLOC, sqlite3Malloc(x,1))
#define sqliteMallocRaw(x)       (ENTER_MALLOC, sqlite3MallocRaw(x,1))
#define sqliteRealloc(x,y)       (ENTER_MALLOC, sqlite3Realloc(x,y))
#define sqliteStrDup(x)          (ENTER_MALLOC, sqlite3StrDup(x))
#define sqliteStrNDup(x,y)       (ENTER_MALLOC, sqlite3StrNDup(x,y))
#define sqliteReallocOrFree(x,y) (ENTER_MALLOC, sqlite3ReallocOrFree(x,y))

#else

#define ENTER_MALLOC 0
#define sqliteMalloc(x)          sqlite3Malloc(x,1)
#define sqliteMallocRaw(x)       sqlite3MallocRaw(x,1)
#define sqliteRealloc(x,y)       sqlite3Realloc(x,y)
#define sqliteStrDup(x)          sqlite3StrDup(x)
#define sqliteStrNDup(x,y)       sqlite3StrNDup(x,y)
#define sqliteReallocOrFree(x,y) sqlite3ReallocOrFree(x,y)

#endif

#define sqliteFree(x)          sqlite3FreeX(x)
#define sqliteAllocSize(x)     sqlite3AllocSize(x)


/*
** An instance of this structure might be allocated to store information
** specific to a single thread.
*/
struct ThreadData {
  int dummy;               /* So that this structure is never empty */

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  int nSoftHeapLimit;      /* Suggested max mem allocation.  No limit if <0 */
  int nAlloc;              /* Number of bytes currently allocated */
  //Pager *pPager;           /* Linked list of all pagers in this thread */
#endif

#ifndef SQLITE_OMIT_SHARED_CACHE
  u8 useSharedData;        /* True if shared pagers and schemas are enabled */
  //BtShared *pBtree;        /* Linked list of all currently open BTrees */
#endif
};

/*
** Name of the master database table.  The master database table
** is a special table that holds the names and attributes of all
** user tables and indices.
*/
#define MASTER_NAME       "sqlite_master"
#define TEMP_MASTER_NAME  "sqlite_temp_master"

/*
** The root-page of the master database table.
*/
#define MASTER_ROOT       1

/*
** The name of the schema table.
*/
#define SCHEMA_TABLE(x)  ((!OMIT_TEMPDB)&&(x==1)?TEMP_MASTER_NAME:MASTER_NAME)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#define ArraySize(X)    (sizeof(X)/sizeof(X[0]))

/*
** Forward references to structures
*/
typedef struct AggInfo AggInfo;
typedef struct AuthContext AuthContext;
typedef struct CollSeq CollSeq;
typedef struct Column Column;
typedef struct Db Db;
typedef struct Schema Schema;
typedef struct Expr Expr;
typedef struct ExprList ExprList;
typedef struct FKey FKey;
typedef struct FuncDef FuncDef;
typedef struct IdList IdList;
typedef struct Index Index;
typedef struct KeyClass KeyClass;
typedef struct KeyInfo KeyInfo;
typedef struct NameContext NameContext;
typedef struct Parse Parse;
typedef struct Select Select;
typedef struct SrcList SrcList;
typedef struct ThreadData ThreadData;
typedef struct Table Table;
typedef struct TableLock TableLock;
typedef struct Token Token;
typedef struct TriggerStack TriggerStack;
typedef struct TriggerStep TriggerStep;
typedef struct Trigger Trigger;
typedef struct WhereInfo WhereInfo;
typedef struct WhereLevel WhereLevel;

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
/* struct Db { */
/*   char *zName;         /1* Name of this database *1/ */
/*   Btree *pBt;          /1* The B*Tree structure for this database file *1/ */
/*   u8 inTrans;          /1* 0: not writable.  1: Transaction.  2: Checkpoint *1/ */
/*   u8 safety_level;     /1* How aggressive at synching data to disk *1/ */
/*   void *pAux;               /1* Auxiliary data.  Usually NULL *1/ */
/*   void (*xFreeAux)(void*);  /1* Routine to free pAux *1/ */
/*   Schema *pSchema;     /1* Pointer to database schema (possibly shared) *1/ */
/* }; */

/*
** An instance of the following structure stores a database schema.
*/
struct Schema {
  int schema_cookie;   /* Database schema version number for this file */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash aFKey;          /* Foreign keys indexed by to-table */
  Table *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 flags;           /* Flags associated with this schema */
  int cache_size;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the 
** Db.flags field.
*/
#define DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->flags&(P))==(P))
#define DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->flags&(P))!=0)
#define DbSetProperty(D,I,P)     (D)->aDb[I].pSchema->flags|=(P)
#define DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->flags&=~(P)

/*
** Allowed values for the DB.flags field.
**
** The DB_SchemaLoaded flag is set after the database schema has been
** read into internal hash tables.
**
** DB_UnresetViews means that one or more views have column names that
** have been filled out.  If the schema changes, these column names might
** changes and so the view will need to be reset.
*/
#define DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#define DB_UnresetViews    0x0002  /* Some views have defined column names */
#define DB_Empty           0x0004  /* The file is empty (length 0 bytes) */

#define SQLITE_UTF16NATIVE (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)

/*
** Each database is an instance of the following structure.
**
** The sqlite.lastRowid records the last insert rowid generated by an
** insert statement.  Inserts on views do not affect its value.  Each
** trigger has its own context, so that lastRowid can be updated inside
** triggers as usual.  The previous value will be restored once the trigger
** exits.  Upon entering a before or instead of trigger, lastRowid is no
** longer (since after version 2.8.12) reset to -1.
**
** The sqlite.nChange does not count changes within triggers and keeps no
** context.  It is reset at start of sqlite3_exec.
** The sqlite.lsChange represents the number of changes made by the last
** insert, update, or delete statement.  It remains constant throughout the
** length of a statement and is then updated by OP_SetCounts.  It keeps a
** context stack just like lastRowid so that the count of changes
** within a trigger is not seen outside the trigger.  Changes to views do not
** affect the value of lsChange.
** The sqlite.csChange keeps track of the number of current changes (since
** the last statement) and is used to update sqlite_lsChange.
**
** The member variables sqlite.errCode, sqlite.zErrMsg and sqlite.zErrMsg16
** store the most recent error code and, if applicable, string. The
** internal function sqlite3Error() is used to set these variables
** consistently.
*/
struct sqlite3 {
  int nDb;                      /* Number of backends currently in use */
  //Db *aDb;                      /* All backends */
  int flags;                    /* Miscellanous flags. See below */
  int errCode;                  /* Most recent error code (SQLITE_*) */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 temp_store;                /* 1: file 2: memory 0: default */
  int nTable;                   /* Number of tables in the database */
  CollSeq *pDfltColl;           /* The default collating sequence (BINARY) */
  i64 lastRowid;                /* ROWID of most recent insert (see above) */
  i64 priorNewRowid;            /* Last randomly generated ROWID */
  int magic;                    /* Magic number for detect library misuse */
  int nChange;                  /* Value returned by sqlite3_changes() */
  int nTotalChange;             /* Value returned by sqlite3_total_changes() */
  struct sqlite3InitInfo {      /* Information used during initialization */
    int iDb;                    /* When back is being initialized */
    int newTnum;                /* Rootpage of table being initialized */
    u8 busy;                    /* TRUE if currently initializing */
  } init;
  //struct Vdbe *pVdbe;           /* List of active virtual machines */
  int activeVdbeCnt;            /* Number of vdbes currently executing */
  void (*xTrace)(void*,const char*);        /* Trace function */
  void *pTraceArg;                          /* Argument to the trace function */
  void (*xProfile)(void*,const char*,u64);  /* Profiling function */
  void *pProfileArg;                        /* Argument to profile function */
  void *pCommitArg;                 /* Argument to xCommitCallback() */   
  int (*xCommitCallback)(void*);    /* Invoked at every commit. */
  void *pRollbackArg;               /* Argument to xRollbackCallback() */   
  void (*xRollbackCallback)(void*); /* Invoked at every commit. */
  void *pUpdateArg;
  void (*xUpdateCallback)(void*,int, const char*,const char*,sqlite_int64);
  void(*xCollNeeded)(void*,sqlite3*,int eTextRep,const char*);
  void(*xCollNeeded16)(void*,sqlite3*,int eTextRep,const void*);
  void *pCollNeededArg;
  sqlite3_value *pErr;          /* Most recent error message */
  char *zErrMsg;                /* Most recent error message (UTF-8 encoded) */
  char *zErrMsg16;              /* Most recent error message (UTF-16 encoded) */
#ifndef SQLITE_OMIT_AUTHORIZATION
  int (*xAuth)(void*,int,const char*,const char*,const char*,const char*);
                                /* Access authorization function */
  void *pAuthArg;               /* 1st argument to the access auth function */
#endif
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  int (*xProgress)(void *);     /* The progress callback */
  void *pProgressArg;           /* Argument to the progress callback */
  int nProgressOps;             /* Number of opcodes for progress callback */
#endif
#ifndef SQLITE_OMIT_GLOBALRECOVER
  sqlite3 *pNext;               /* Linked list of open db handles. */
#endif
  Hash aFunc;                   /* All functions that can be in SQL exprs */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  int busyTimeout;             /* Busy handler timeout, in msec */
 // Db aDbStatic[2];              /* Static space for the 2 default backends */
#ifdef SQLITE_SSE
  //sqlite3_stmt *pFetch;         /* Used by SSE to fetch stored statements */
#endif
};

/*
** A macro to discover the encoding of a database.
*/
#define ENC(db) ((db)->aDb[0].pSchema->enc)

/*
** Possible values for the sqlite.flags and or Db.flags fields.
**
** On sqlite.flags, the SQLITE_InTrans value means that we have
** executed a BEGIN.  On Db.flags, SQLITE_InTrans means a statement
** transaction is active on that particular database file.
*/
#define SQLITE_VdbeTrace      0x00000001  /* True to trace VDBE execution */
#define SQLITE_Interrupt      0x00000004  /* Cancel current operation */
#define SQLITE_InTrans        0x00000008  /* True if in a transaction */
#define SQLITE_InternChanges  0x00000010  /* Uncommitted Hash table changes */
#define SQLITE_FullColNames   0x00000020  /* Show full column names on SELECT */
#define SQLITE_ShortColNames  0x00000040  /* Show short columns names */
#define SQLITE_CountRows      0x00000080  /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#define SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#define SQLITE_SqlTrace       0x00000200  /* Debug print SQL as it executes */
#define SQLITE_VdbeListing    0x00000400  /* Debug listings of VDBE programs */
#define SQLITE_WriteSchema    0x00000800  /* OK to update SQLITE_MASTER */
#define SQLITE_NoReadlock     0x00001000  /* Readlocks are omitted when 
                                          ** accessing read-only databases */
#define SQLITE_IgnoreChecks   0x00002000  /* Do not enforce check constraints */
#define SQLITE_ReadUncommitted 0x00004000  /* For shared-cache mode */
#define SQLITE_LegacyFileFmt  0x00008000  /* Create new databases in format 1 */
#define SQLITE_FullFSync      0x00010000  /* Use full fsync on the backend */

/*
** Possible values for the sqlite.magic field.
** The numbers are obtained at random and have no special meaning, other
** than being distinct from one another.
*/
#define SQLITE_MAGIC_OPEN     0xa029a697  /* Database is open */
#define SQLITE_MAGIC_CLOSED   0x9f3c2d33  /* Database is closed */
#define SQLITE_MAGIC_BUSY     0xf03b7906  /* Database currently in use */
#define SQLITE_MAGIC_ERROR    0xb5357930  /* An SQLITE_MISUSE error occurred */

/*
** Each SQL function is defined by an instance of the following
** structure.  A pointer to this structure is stored in the sqlite.aFunc
** hash table.  When multiple functions have the same name, the hash table
** points to a linked list of these structures.
*/
struct FuncDef {
  i16 nArg;            /* Number of arguments.  -1 means unlimited */
  u8 iPrefEnc;         /* Preferred text encoding (SQLITE_UTF8, 16LE, 16BE) */
  u8 needCollSeq;      /* True if sqlite3GetFuncCollSeq() might be called */
  u8 flags;            /* Some combination of SQLITE_FUNC_* */
  void *pUserData;     /* User data parameter */
  FuncDef *pNext;      /* Next function with same name */
  void (*xFunc)(sqlite3_context*,int,sqlite3_value**); /* Regular function */
  void (*xStep)(sqlite3_context*,int,sqlite3_value**); /* Aggregate step */
  void (*xFinalize)(sqlite3_context*);                /* Aggregate finializer */
  char zName[1];       /* SQL name of the function.  MUST BE LAST */
};

/*
** Possible values for FuncDef.flags
*/
#define SQLITE_FUNC_LIKE   0x01  /* Candidate for the LIKE optimization */
#define SQLITE_FUNC_CASE   0x02  /* Case-sensitive LIKE-type function */

/*
** information about each column of an SQL table is held in an instance
** of this structure.
*/
struct Column {
  char *zName;     /* Name of this column */
  Expr *pDflt;     /* Default value of this column */
  char *zType;     /* Data type for this column */
  char *zColl;     /* Collating sequence.  If NULL, use the default */
  u8 notNull;      /* True if there is a NOT NULL constraint */
  u8 isPrimKey;    /* True if this column is part of the PRIMARY KEY */
  char affinity;   /* One of the SQLITE_AFF_... values */
};

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** There may two seperate implementations of the collation function, one
** that processes text in UTF-8 encoding (CollSeq.xCmp) and another that
** processes text encoded in UTF-16 (CollSeq.xCmp16), using the machine
** native byte order. When a collation sequence is invoked, SQLite selects
** the version that will require the least expensive encoding
** translations, if any.
**
** The CollSeq.pUser member variable is an extra parameter that passed in
** as the first argument to the UTF-8 comparison function, xCmp.
** CollSeq.pUser16 is the equivalent for the UTF-16 comparison function,
** xCmp16.
**
** If both CollSeq.xCmp and CollSeq.xCmp16 are NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
struct CollSeq {
  char *zName;         /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;              /* Text encoding handled by xCmp() */
  u8 type;             /* One of the SQLITE_COLL_... values below */
  void *pUser;         /* First argument to xCmp() */
  int (*xCmp)(void*,int, const void*, int, const void*);
};

/*
** Allowed values of CollSeq flags:
*/
#define SQLITE_COLL_BINARY  1  /* The default memcmp() collating sequence */
#define SQLITE_COLL_NOCASE  2  /* The built-in NOCASE collating sequence */
#define SQLITE_COLL_REVERSE 3  /* The built-in REVERSE collating sequence */
#define SQLITE_COLL_USER    0  /* Any other user-defined collating sequence */

/*
** A sort order can be either ASC or DESC.
*/
#define SQLITE_SO_ASC       0  /* Sort in ascending order */
#define SQLITE_SO_DESC      1  /* Sort in ascending order */

/*
** Column affinity types.
**
** These used to have mnemonic name like 'i' for SQLITE_AFF_INTEGER and
** 't' for SQLITE_AFF_TEXT.  But we can save a little space and improve
** the speed a little by number the values consecutively.  
**
** But rather than start with 0 or 1, we begin with 'a'.  That way,
** when multiple affinity types are concatenated into a string and
** used as the P3 operand, they will be more readable.
**
** Note also that the numeric types are grouped together so that testing
** for a numeric type is a single comparison.
*/
#define SQLITE_AFF_TEXT     'a'
#define SQLITE_AFF_NONE     'b'
#define SQLITE_AFF_NUMERIC  'c'
#define SQLITE_AFF_INTEGER  'd'
#define SQLITE_AFF_REAL     'e'

#define sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** Each SQL table is represented in memory by an instance of the
** following structure.
**
** Table.zName is the name of the table.  The case of the original
** CREATE TABLE statement is stored, but case is not significant for
** comparisons.
**
** Table.nCol is the number of columns in this table.  Table.aCol is a
** pointer to an array of Column structures, one for each column.
**
** If the table has an INTEGER PRIMARY KEY, then Table.iPKey is the index of
** the column that is that key.   Otherwise Table.iPKey is negative.  Note
** that the datatype of the PRIMARY KEY must be INTEGER for this field to
** be set.  An INTEGER PRIMARY KEY is used as the rowid for each row of
** the table.  If a table has no INTEGER PRIMARY KEY, then a random rowid
** is generated for each row of the table.  Table.hasPrimKey is true if
** the table has any PRIMARY KEY, INTEGER or otherwise.
**
** Table.tnum is the page number for the root BTree page of the table in the
** database file.  If Table.iDb is the index of the database table backend
** in sqlite.aDb[].  0 is for the main database and 1 is for the file that
** holds temporary tables and indices.  If Table.isTransient
** is true, then the table is stored in a file that is automatically deleted
** when the VDBE cursor to the table is closed.  In this case Table.tnum 
** refers VDBE cursor number that holds the table open, not to the root
** page number.  Transient tables are used to hold the results of a
** sub-query that appears instead of a real table name in the FROM clause 
** of a SELECT statement.
*/
struct Table {
  char *zName;     /* Name of the table */
  int nCol;        /* Number of columns in this table */
  Column *aCol;    /* Information about each column */
  int iPKey;       /* If not less then 0, use aCol[iPKey] as the primary key */
  Index *pIndex;   /* List of SQL indexes on this table. */
  int tnum;        /* Root BTree node for this table (see note above) */
  Select *pSelect; /* NULL for tables.  Points to definition if a view. */
  u8 readOnly;     /* True if this table should not be written by the user */
  u8 isTransient;  /* True if automatically deleted when VDBE finishes */
  u8 hasPrimKey;   /* True if there exists a primary key */
  u8 keyConf;      /* What to do in case of uniqueness conflict on iPKey */
  u8 autoInc;      /* True if the integer primary key is autoincrement */
  int nRef;          /* Number of pointers to this Table */
  Trigger *pTrigger; /* List of SQL triggers on this table */
  FKey *pFKey;       /* Linked list of all foreign keys in this table */
  char *zColAff;     /* String defining the affinity of each column */
#ifndef SQLITE_OMIT_CHECK
  Expr *pCheck;      /* The AND of all CHECK constraints */
#endif
#ifndef SQLITE_OMIT_ALTERTABLE
  int addColOffset;  /* Offset in CREATE TABLE statement to add a new column */
#endif
  Schema *pSchema;
};

/*
** Each foreign key constraint is an instance of the following structure.
**
** A foreign key is associated with two tables.  The "from" table is
** the table that contains the REFERENCES clause that creates the foreign
** key.  The "to" table is the table that is named in the REFERENCES clause.
** Consider this example:
**
**     CREATE TABLE ex1(
**       a INTEGER PRIMARY KEY,
**       b INTEGER CONSTRAINT fk1 REFERENCES ex2(x)
**     );
**
** For foreign key "fk1", the from-table is "ex1" and the to-table is "ex2".
**
** Each REFERENCES clause generates an instance of the following structure
** which is attached to the from-table.  The to-table need not exist when
** the from-table is created.  The existance of the to-table is not checked
** until an attempt is made to insert data into the from-table.
**
** The sqlite.aFKey hash table stores pointers to this structure
** given the name of a to-table.  For each to-table, all foreign keys
** associated with that table are on a linked list using the FKey.pNextTo
** field.
*/
struct FKey {
  Table *pFrom;     /* The table that constains the REFERENCES clause */
  FKey *pNextFrom;  /* Next foreign key in pFrom */
  char *zTo;        /* Name of table that the key points to */
  FKey *pNextTo;    /* Next foreign key that points to zTo */
  int nCol;         /* Number of columns in this key */
  struct sColMap {  /* Mapping of columns in pFrom to columns in zTo */
    int iFrom;         /* Index of column in pFrom */
    char *zCol;        /* Name of column in zTo.  If 0 use PRIMARY KEY */
  } *aCol;          /* One entry for each of nCol column s */
  u8 isDeferred;    /* True if constraint checking is deferred till COMMIT */
  u8 updateConf;    /* How to resolve conflicts that occur on UPDATE */
  u8 deleteConf;    /* How to resolve conflicts that occur on DELETE */
  u8 insertConf;    /* How to resolve conflicts that occur on INSERT */
};

/*
** SQLite supports many different ways to resolve a contraint
** error.  ROLLBACK processing means that a constraint violation
** causes the operation in process to fail and for the current transaction
** to be rolled back.  ABORT processing means the operation in process
** fails and any prior changes from that one operation are backed out,
** but the transaction is not rolled back.  FAIL processing means that
** the operation in progress stops and returns an error code.  But prior
** changes due to the same operation are not backed out and no rollback
** occurs.  IGNORE means that the particular row that caused the constraint
** error is not inserted or updated.  Processing continues and no error
** is returned.  REPLACE means that preexisting database rows that caused
** a UNIQUE constraint violation are removed so that the new insert or
** update can proceed.  Processing continues and no error is reported.
**
** RESTRICT, SETNULL, and CASCADE actions apply only to foreign keys.
** RESTRICT is the same as ABORT for IMMEDIATE foreign keys and the
** same as ROLLBACK for DEFERRED keys.  SETNULL means that the foreign
** key is set to NULL.  CASCADE means that a DELETE or UPDATE of the
** referenced table row is propagated into the row that holds the
** foreign key.
** 
** The following symbolic values are used to record which type
** of action to take.
*/
#define OE_None     0   /* There is no constraint to check */
#define OE_Rollback 1   /* Fail the operation and rollback the transaction */
#define OE_Abort    2   /* Back out changes but do no rollback transaction */
#define OE_Fail     3   /* Stop the operation but leave all prior changes */
#define OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#define OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */

#define OE_Restrict 6   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#define OE_SetNull  7   /* Set the foreign key value to NULL */
#define OE_SetDflt  8   /* Set the foreign key value to its default */
#define OE_Cascade  9   /* Cascade the changes */

#define OE_Default  99  /* Do whatever the default action is */


/*
** An instance of the following structure is passed as the first
** argument to sqlite3VdbeKeyCompare and is used to control the 
** comparison of the two index keys.
**
** If the KeyInfo.incrKey value is true and the comparison would
** otherwise be equal, then return a result as if the second key
** were larger.
*/
struct KeyInfo {
  u8 enc;             /* Text encoding - one of the TEXT_Utf* values */
  u8 incrKey;         /* Increase 2nd key by epsilon before comparison */
  int nField;         /* Number of entries in aColl[] */
  u8 *aSortOrder;     /* If defined an aSortOrder[i] is true, sort DESC */
  CollSeq *aColl[1];  /* Collating sequence for each term of the key */
};

/*
** Each SQL index is represented in memory by an
** instance of the following structure.
**
** The columns of the table that are to be indexed are described
** by the aiColumn[] field of this structure.  For example, suppose
** we have the following table and index:
**
**     CREATE TABLE Ex1(c1 int, c2 int, c3 text);
**     CREATE INDEX Ex2 ON Ex1(c3,c1);
**
** In the Table structure describing Ex1, nCol==3 because there are
** three columns in the table.  In the Index structure describing
** Ex2, nColumn==2 since 2 of the 3 columns of Ex1 are indexed.
** The value of aiColumn is {2, 0}.  aiColumn[0]==2 because the 
** first column to be indexed (c3) has an index of 2 in Ex1.aCol[].
** The second column to be indexed (c1) has an index of 0 in
** Ex1.aCol[], hence Ex2.aiColumn[1]==0.
**
** The Index.onError field determines whether or not the indexed columns
** must be unique and what to do if they are not.  When Index.onError=OE_None,
** it means this is not a unique index.  Otherwise it is a unique index
** and the value of Index.onError indicate the which conflict resolution 
** algorithm to employ whenever an attempt is made to insert a non-unique
** element.
*/
struct Index {
  char *zName;     /* Name of this index */
  int nColumn;     /* Number of columns in the table used by this index */
  int *aiColumn;   /* Which columns are used by this index.  1st is 0 */
  unsigned *aiRowEst; /* Result of ANALYZE: Est. rows selected by each column */
  Table *pTable;   /* The SQL table being indexed */
  int tnum;        /* Page containing root of this index in database file */
  u8 onError;      /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  u8 autoIndex;    /* True if is automatically created (ex: by UNIQUE) */
  char *zColAff;   /* String defining the affinity of each column */
  Index *pNext;    /* The next index associated with the same table */
  Schema *pSchema; /* Schema containing this index */
  u8 *aSortOrder;  /* Array of size Index.nColumn. True==DESC, False==ASC */
  char **azColl;   /* Array of collation sequence names for index */
};

/*
** Each token coming out of the lexer is an instance of
** this structure.  Tokens are also used as part of an expression.
**
** Note if Token.z==0 then Token.dyn and Token.n are undefined and
** may contain random values.  Do not make any assuptions about Token.dyn
** and Token.n when Token.z==0.
*/
struct Token {
  const unsigned char *z; /* Text of the token.  Not NULL-terminated! */
  unsigned dyn  : 1;      /* True for malloced memory, false for static */
  unsigned n    : 31;     /* Number of characters in this token */
};

/*
** An instance of this structure contains information needed to generate
** code for a SELECT that contains aggregate functions.
**
** If Expr.op==TK_AGG_COLUMN or TK_AGG_FUNCTION then Expr.pAggInfo is a
** pointer to this structure.  The Expr.iColumn field is the index in
** AggInfo.aCol[] or AggInfo.aFunc[] of information needed to generate
** code for that node.
**
** AggInfo.pGroupBy and AggInfo.aFunc.pExpr point to fields within the
** original Select structure that describes the SELECT statement.  These
** fields do not need to be freed when deallocating the AggInfo structure.
*/
struct AggInfo {
  u8 directMode;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 useSortingIdx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  int sortingIdx;         /* Cursor number of the sorting index */
  ExprList *pGroupBy;     /* The group by clause */
  int nSortingColumn;     /* Number of columns in the sorting index */
  struct AggInfo_col {    /* For each column used in source tables */
    int iTable;              /* Cursor number of the source table */
    int iColumn;             /* Column number within the source table */
    int iSorterColumn;       /* Column number in the sorting index */
    int iMem;                /* Memory location that acts as accumulator */
    Expr *pExpr;             /* The original expression */
  } *aCol;
  int nColumn;            /* Number of used entries in aCol[] */
  int nColumnAlloc;       /* Number of slots allocated for aCol[] */
  int nAccumulator;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  struct AggInfo_func {   /* For each aggregate function */
    Expr *pExpr;             /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    int iMem;                /* Memory location that acts as accumulator */
    int iDistinct;           /* Virtual table used to enforce DISTINCT */
  } *aFunc;
  int nFunc;              /* Number of entries in aFunc[] */
  int nFuncAlloc;         /* Number of slots allocated for aFunc[] */
};

/*
** Each node of an expression in the parse tree is an instance
** of this structure.
**
** Expr.op is the opcode.  The integer parser token codes are reused
** as opcodes here.  For example, the parser defines TK_GE to be an integer
** code representing the ">=" operator.  This same integer code is reused
** to represent the greater-than-or-equal-to operator in the expression
** tree.
**
** Expr.pRight and Expr.pLeft are subexpressions.  Expr.pList is a list
** of argument if the expression is a function.
**
** Expr.token is the operator token for this node.  For some expressions
** that have subexpressions, Expr.token can be the complete text that gave
** rise to the Expr.  In the latter case, the token is marked as being
** a compound token.
**
** An expression of the form ID or ID.ID refers to a column in a table.
** For such expressions, Expr.op is set to TK_COLUMN and Expr.iTable is
** the integer cursor number of a VDBE cursor pointing to that table and
** Expr.iColumn is the column number for the specific column.  If the
** expression is used as a result in an aggregate SELECT, then the
** value is also stored in the Expr.iAgg column in the aggregate so that
** it can be accessed after all aggregates are computed.
**
** If the expression is a function, the Expr.iTable is an integer code
** representing which function.  If the expression is an unbound variable
** marker (a question mark character '?' in the original SQL) then the
** Expr.iTable holds the index number for that variable.
**
** If the expression is a subquery then Expr.iColumn holds an integer
** register number containing the result of the subquery.  If the
** subquery gives a constant result, then iTable is -1.  If the subquery
** gives a different answer at different times during statement processing
** then iTable is the address of a subroutine that computes the subquery.
**
** The Expr.pSelect field points to a SELECT statement.  The SELECT might
** be the right operand of an IN operator.  Or, if a scalar SELECT appears
** in an expression the opcode is TK_SELECT and Expr.pSelect is the only
** operand.
**
** If the Expr is of type OP_Column, and the table it is selecting from
** is a disk table or the "old.*" pseudo-table, then pTab points to the
** corresponding table definition.
*/
struct Expr {
  u8 op;                 /* Operation performed by this node */
  char affinity;         /* The affinity of the column or 0 if not a column */
  u8 flags;              /* Various flags.  See below */
  CollSeq *pColl;        /* The collation type of the column or 0 */
  Expr *pLeft, *pRight, *pParent;  /* Left and right subnodes */
  ExprList *pList;       /* A list of expressions used as function arguments
                         ** or in "<expr> IN (<expr-list)" */
  Token token;           /* An operand token */
  Token span;            /* Complete text of the expression */
  int iTable, iColumn;   /* When op==TK_COLUMN, then this expr node means the
                         ** iColumn-th field of the iTable-th table. */
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  int iAgg;              /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
  int iRightJoinTable;   /* If EP_FromJoin, the right table of the join */
  Select *pSelect;       /* When the expression is a sub-select.  Also the
                         ** right side of "<expr> IN (<select>)" */
  Table *pTab;           /* Table for OP_Column expressions. */
  Schema *pSchema;
};

/*
** The following are the meanings of bits in the Expr.flags field.
*/
#define EP_FromJoin     0x01  /* Originated in ON or USING clause of a join */
#define EP_Agg          0x02  /* Contains one or more aggregate functions */
#define EP_Resolved     0x04  /* IDs have been resolved to COLUMNs */
#define EP_Error        0x08  /* Expression contains one or more errors */
#define EP_Distinct     0x10  /* Aggregate function with DISTINCT keyword */
#define EP_VarSelect    0x20  /* pSelect is correlated, not constant */
#define EP_Dequoted     0x40  /* True if the string has been dequoted */

/*
** These macros can be used to test, set, or clear bits in the 
** Expr.flags field.
*/
#define ExprHasProperty(E,P)     (((E)->flags&(P))==(P))
#define ExprHasAnyProperty(E,P)  (((E)->flags&(P))!=0)
#define ExprSetProperty(E,P)     (E)->flags|=(P)
#define ExprClearProperty(E,P)   (E)->flags&=~(P)

/*
** A list of expressions.  Each expression may optionally have a
** name.  An expr/name combination can be used in several ways, such
** as the list of "expr AS ID" fields following a "SELECT" or in the
** list of "ID = expr" items in an UPDATE.  A list of expressions can
** also be used as the argument to a function, in which case the a.zName
** field is not used.
*/
struct ExprList {
  int nExpr;             /* Number of expressions on the list */
  int nAlloc;            /* Number of entries allocated below */
  int iECursor;          /* VDBE Cursor associated with this ExprList */
  struct ExprList_item {
    Expr *pExpr;           /* The list of expressions */
    char *zName;           /* Token associated with this expression */
    u8 sortOrder;          /* 1 for DESC or 0 for ASC */
    u8 isAgg;              /* True if this is an aggregate like count(*) */
    u8 done;               /* A flag to indicate when processing is finished */
  } *a;                  /* One entry for each expression */
};

typedef struct ValuesList {
    int nValues;    /* Number of Values to insert*/
    int nAlloc;
    ExprList** a;    /* array of Values*/
}ValuesList;

/*
** An instance of this structure can hold a simple list of identifiers,
** such as the list "a,b,c" in the following statements:
**
**      INSERT INTO t(a,b,c) VALUES ...;
**      CREATE INDEX idx ON t(a,b,c);
**      CREATE TRIGGER trig BEFORE UPDATE ON t(a,b,c) ...;
**
** The IdList.a.idx field is used when the IdList represents the list of
** column names after a table name in an INSERT statement.  In the statement
**
**     INSERT INTO t(a,b,c) ...
**
** If "a" is the k-th column of table "t", then IdList.a[0].idx==k.
*/
struct IdList {
  struct IdList_item {
    char *zName;      /* Name of the identifier */
    int idx;          /* Index in some Table.aCol[] of a column named zName */
  } *a;
  int nId;         /* Number of identifiers on the list */
  int nAlloc;      /* Number of entries allocated for a[] below */
};

/*
** The bitmask datatype defined below is used for various optimizations.
*/
typedef unsigned int Bitmask;

/*
** The following structure describes the FROM clause of a SELECT statement.
** Each table or subquery in the FROM clause is a separate element of
** the SrcList.a[] array.
**
** With the addition of multiple database support, the following structure
** can also be used to describe a particular table such as the table that
** is modified by an INSERT, DELETE, or UPDATE statement.  In standard SQL,
** such a table must be a simple name: ID.  But in SQLite, the table can
** now be identified by a database name, a dot, then the table name: ID.ID.
*/
struct SrcList {
  i16 nSrc;        /* Number of tables or subqueries in the FROM clause */
  i16 nAlloc;      /* Number of entries allocated in a[] below */
  struct SrcList_item {
    char *zDatabase;  /* Name of database holding this table */
    char *zName;      /* Name of the table */
    char *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
    Table *pTab;      /* An SQL table corresponding to zName */
    Select *pSelect;  /* A SELECT statement used in place of a table name */
    u8 isPopulated;   /* Temporary table associated with SELECT is populated */
    u8 jointype;      /* Type of join between this table and the next */
    i16 iCursor;      /* The VDBE cursor number used to access this table */
    Expr *pOn;        /* The ON clause of a join */
    IdList *pUsing;   /* The USING clause of a join */
    Bitmask colUsed;  /* Bit N (1<<N) set if column N or pTab is used */

    Token tableToken;
    Token dbToken;
  } a[1];             /* One entry for each identifier on the list */
};

/*
** Permitted values of the SrcList.a.jointype field
*/
#define JT_INNER     0x0001    /* Any kind of inner or cross join */
#define JT_CROSS     0x0002    /* Explicit use of the CROSS keyword */
#define JT_NATURAL   0x0004    /* True for a "natural" join */
#define JT_LEFT      0x0008    /* Left outer join */
#define JT_RIGHT     0x0010    /* Right outer join */
#define JT_OUTER     0x0020    /* The "OUTER" keyword is present */
#define JT_ERROR     0x0040    /* unknown or unsupported join type */
#define JT_FULL      0x0080    /* FULL JOIN, add by wink*/
/*
** For each nested loop in a WHERE clause implementation, the WhereInfo
** structure contains a single instance of this structure.  This structure
** is intended to be private the the where.c module and should not be
** access or modified by other modules.
*/
struct WhereLevel {
  int iFrom;            /* Which entry in the FROM clause */
  int flags;            /* Flags associated with this level */
  int iMem;             /* First memory cell used by this level */
  int iLeftJoin;        /* Memory cell used to implement LEFT OUTER JOIN */
  Index *pIdx;          /* Index used.  NULL if no index */
  int iTabCur;          /* The VDBE cursor used to access the table */
  int iIdxCur;          /* The VDBE cursor used to acesss pIdx */
  int brk;              /* Jump here to break out of the loop */
  int cont;             /* Jump here to continue with the next loop cycle */
  int top;              /* First instruction of interior of the loop */
  int op, p1, p2;       /* Opcode used to terminate the loop */
  int nEq;              /* Number of == or IN constraints on this loop */
  int nIn;              /* Number of IN operators constraining this loop */
  int *aInLoop;         /* Loop terminators for IN operators */
};

/*
** The WHERE clause processing routine has two halves.  The
** first part does the start of the WHERE loop and the second
** half does the tail of the WHERE loop.  An instance of
** this structure is returned by the first half and passed
** into the second half to give some continuity.
*/
struct WhereInfo {
  Parse *pParse;
  SrcList *pTabList;   /* List of tables in the join */
  int iTop;            /* The very beginning of the WHERE loop */
  int iContinue;       /* Jump here to continue with next record */
  int iBreak;          /* Jump here to break out of the loop */
  int nLevel;          /* Number of nested loop */
  WhereLevel a[1];     /* Information about each nest loop in the WHERE */
};

/*
** A NameContext defines a context in which to resolve table and column
** names.  The context consists of a list of tables (the pSrcList) field and
** a list of named expression (pEList).  The named expression list may
** be NULL.  The pSrc corresponds to the FROM clause of a SELECT or
** to the table being operated on by INSERT, UPDATE, or DELETE.  The
** pEList corresponds to the result set of a SELECT and is NULL for
** other statements.
**
** NameContexts can be nested.  When resolving names, the inner-most 
** context is searched first.  If no match is found, the next outer
** context is checked.  If there is still no match, the next context
** is checked.  This process continues until either a match is found
** or all contexts are check.  When a match is found, the nRef member of
** the context containing the match is incremented. 
**
** Each subquery gets a new NameContext.  The pNext field points to the
** NameContext in the parent query.  Thus the process of scanning the
** NameContext list corresponds to searching through successively outer
** subqueries looking for a match.
*/
struct NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  ExprList *pEList;    /* Optional list of named expressions */
  int nRef;            /* Number of names resolved by this context */
  int nErr;            /* Number of errors encountered while resolving names */
  u8 allowAgg;         /* Aggregate functions allowed here */
  u8 hasAgg;           /* True if aggregates are seen */
  u8 isCheck;          /* True if resolving names in a CHECK constraint */
  int nDepth;          /* Depth of subquery recursion. 1 for no recursion */
  AggInfo *pAggInfo;   /* Information about aggregates at this level */
  NameContext *pNext;  /* Next outer name context.  NULL for outermost */
};

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
**
** nLimit is set to -1 if there is no LIMIT clause.  nOffset is set to 0.
** If there is a LIMIT clause, the parser sets nLimit to the value of the
** limit and nOffset to the value of the offset (or 0 if there is not
** offset).  But later on, nLimit and nOffset become the memory locations
** in the VDBE that record the limit and offset counters.
**
** addrOpenVirt[] entries contain the address of OP_OpenVirtual opcodes.
** These addresses must be stored so that we can go back and fill in
** the P3_KEYINFO and P2 parameters later.  Neither the KeyInfo nor
** the number of columns in P2 can be computed at the same time
** as the OP_OpenVirtual instruction is coded because not
** enough information about the compound query is known at that point.
** The KeyInfo for addrOpenVirt[0] and [1] contains collating sequences
** for the result set.  The KeyInfo for addrOpenVirt[2] contains collating
** sequences for the ORDER BY clause.
*/
struct Select {
  ExprList *pEList;      /* The fields of the result */
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  u8 isDistinct;         /* True if the DISTINCT keyword is present */
  u8 isResolved;         /* True once sqlite3SelectResolve() has run. */
  u8 isAgg;              /* True if this is an aggregate query */
  u8 usesVirt;           /* True if uses an OpenVirtual opcode */
  u8 disallowOrderBy;    /* Do not allow an ORDER BY to be attached if TRUE */
  SrcList *pSrc;         /* The FROM clause */
  Expr *pWhere;          /* The WHERE clause */
  ExprList *pGroupBy;    /* The GROUP BY clause */
  Expr *pHaving;         /* The HAVING clause */
  ExprList *pOrderBy;    /* The ORDER BY clause */
  Select *pPrior;        /* Prior select in a compound select statement */
  Select *pRightmost;    /* Right-most select in a compound select statement */
  Expr *pLimit;          /* LIMIT expression. NULL means not used. */
  Expr *pOffset;         /* OFFSET expression. NULL means not used. */
  int iLimit, iOffset;   /* Memory registers holding LIMIT & OFFSET counters */
  int addrOpenVirt[3];   /* OP_OpenVirtual opcodes related to this select */
};

typedef struct Insert {
    SrcList *pTabList;    /* Name of table into which we are inserting */
    ExprList *pSetList;      /* List of values to be inserted */
    ValuesList *pValuesList;
    Select *pSelect;      /* A SELECT statement to use as the data source */
    IdList *pColumn;      /* Column names corresponding to IDLIST. */
    int onError;           /* How to handle constraint errors */
}Insert;

typedef struct Delete {
    SrcList *pTabList;     /* The table from which we should delete things */
    Expr *pWhere;           /* The WHERE clause.  May be null */
    Expr *pLimit;
    Expr *pOffset;
}Delete;

typedef struct Update {
    SrcList *pTabList;     /* The table in which we should change things */
    ExprList *pChanges;    /* Things to be changed */
    Expr *pWhere;          /* The WHERE clause.  May be null */
    int onError;            /* How to handle constraint errors */
    Expr *pLimit;
    Expr *pOffset;
}Update;

typedef struct SetStatement {
    ExprList *pSetList;
    Token   value;    /* value of SET NAMES or SET CHARACTER SET*/
}SetStatement;

/*
** The results of a select can be distributed in several ways.
*/
#define SRT_Union        1  /* Store result as keys in an index */
#define SRT_Except       2  /* Remove result from a UNION index */
#define SRT_Discard      3  /* Do not save the results anywhere */

/* The ORDER BY clause is ignored for all of the above */
#define IgnorableOrderby(X) (X<=SRT_Discard)

#define SRT_Callback     4  /* Invoke a callback with each row of result */
#define SRT_Mem          5  /* Store result in a memory cell */
#define SRT_Set          6  /* Store non-null results as keys in an index */
#define SRT_Table        7  /* Store result as data with an automatic rowid */
#define SRT_VirtualTab   8  /* Create virtual table and store like SRT_Table */
#define SRT_Subroutine   9  /* Call a subroutine to handle results */
#define SRT_Exists      10  /* Store 1 if the result is not empty */

typedef enum SqlType{
    SQLTYPE_UNKNOWN = 0,
    SQLTYPE_SELECT,
    SQLTYPE_UPDATE,
    SQLTYPE_INSERT,
    SQLTYPE_REPLACE,
    SQLTYPE_DELETE, 
    SQLTYPE_TRANSACTION_BEGIN,
    SQLTYPE_TRANSACTION_START,
    SQLTYPE_TRANSACTION_COMMIT,
    SQLTYPE_TRANSACTION_ROLLBACK,
    SQLTYPE_CREATE_TABLE,
    SQLTYPE_DROP_TABLE,
    SQLTYPE_SET,
    SQLTYPE_SET_NAMES,
    SQLTYPE_SET_CHARACTER_SET,
    SQLTYPE_SHOW,
}SqlType;

typedef enum ShowStatementType{
    SHOWTYPE_SHOW_DATABASES,
    SHOWTYPE_SHOW_TABLES,
    SHOWTYPE_SHOW_TABLE_STATUS,
    SHOWTYPE_SHOW_VARIABLES,
    SHOWTYPE_SHOW_COLLATION
}ShowStatementType;

typedef union ParsedResult {
    Select* selectObj;
    Insert* insertObj;
    Delete* deleteObj;
    Update* updateObj;
    SetStatement* setObj;
    ShowStatementType showType;
} ParsedResult;

typedef struct ParsedResultItem {
    ParsedResult result;
    SqlType sqltype;
}ParsedResultItem;

typedef struct ParsedResultArray {
    ParsedResultItem *array;
    u32     curSize;
    u32     allocSize;
}ParsedResultArray;

typedef struct TokenItem {
    Token token;
    int  tokenType;
}TokenItem;

typedef struct TokenArray {
    TokenItem *array;
    u32       curSize;
    u32       allocSize;
}TokenArray;

/*
** An SQL parser context.  A copy of this structure is passed through
** the parser and down into all the parser action routine in order to
** carry around information that is global to the entire parse.
**
** The structure is divided into two parts.  When the parser and code
** generate call themselves recursively, the first part of the structure
** is constant but the second part is reset at the beginning and end of
** each recursion.
**
** The nTableLock and aTableLock variables are only used if the shared-cache 
** feature is enabled (if sqlite3Tsd()->useSharedData is true). They are
** used to store the set of table-locks required by the statement being
** compiled. Function sqlite3TableLock() is used to add entries to the
** list.
*/
struct Parse {
  //sqlite3 *db;         /* The main database structure */
  int flags;                    /* Miscellanous flags. See below */
  int rc;              /* Return code from execution */
  char *zErrMsg;       /* An error message */
  //Vdbe *pVdbe;         /* An engine for executing database bytecode */
  u8 colNamesSet;      /* TRUE after OP_ColumnName has been issued to pVdbe */
  u8 nameClash;        /* A permanent table name clashes with temp table name */
  u8 checkSchema;      /* Causes schema cookie check after an error */
  u8 nested;           /* Number of nested calls to the parser/code generator */
  int nErr;            /* Number of errors seen */
  int nTab;            /* Number of previously allocated VDBE cursors */
  int nMem;            /* Number of memory cells used so far */
  int nSet;            /* Number of sets used so far */
  int ckOffset;        /* Stack offset to data used by CHECK constraints */
  u32 writeMask;       /* Start a write transaction on these databases */
  u32 cookieMask;      /* Bitmask of schema verified databases */
  int cookieGoto;      /* Address of OP_Goto to cookie verifier subroutine */
  int cookieValue[MAX_ATTACHED+2];  /* Values of cookies to verify */
#ifndef SQLITE_OMIT_SHARED_CACHE
  /* int nTableLock;        /1* Number of locks in aTableLock *1/ */
  /* TableLock *aTableLock; /1* Required table locks for shared-cache mode *1/ */
#endif

  /* Above is constant between recursions.  Below is reset before and after
  ** each recursion */

  /* int nVar;            /1* Number of '?' variables seen in the SQL so far *1/ */
  /* int nVarExpr;        /1* Number of used slots in apVarExpr[] *1/ */
  /* int nVarExprAlloc;   /1* Number of allocated slots in apVarExpr[] *1/ */
  /* Expr **apVarExpr;    /1* Pointers to :aaa and $aaaa wildcard expressions *1/ */
  u8 explain;          /* True if the EXPLAIN flag is found on the query */
  Token sErrToken;     /* The token at which the error occurred */
  Token sNameToken;    /* Token with unqualified schema object name */
  Token sLastToken;    /* The last token parsed */
  const char *zSql;    /* All SQL text */
  const char *zTail;   /* All SQL text past the last semicolon parsed */
  /* Table *pNewTable;    /1* A table being constructed by CREATE TABLE *1/ */
  /* Trigger *pNewTrigger;     /1* Trigger under construct by a CREATE TRIGGER *1/ */
  /* TriggerStack *trigStack;  /1* Trigger actions being coded *1/ */
  /* const char *zAuthContext; /1* The 6th parameter to db->xAuth callbacks *1/ */
    
  ParsedResultArray parsed;
  TokenArray tokens;
};

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
struct AuthContext {
  const char *zAuthContext;   /* Put saved Parse.zAuthContext here */
  Parse *pParse;              /* The Parse structure */
};

/*
** Bitfield flags for P2 value in OP_Insert and OP_Delete
*/
#define OPFLAG_NCHANGE   1    /* Set to update db->nChange */
#define OPFLAG_LASTROWID 2    /* Set to update db->lastRowid */
#define OPFLAG_ISUPDATE  4    /* This OP_Insert is an sql UPDATE */

/*
 * Each trigger present in the database schema is stored as an instance of
 * struct Trigger. 
 *
 * Pointers to instances of struct Trigger are stored in two ways.
 * 1. In the "trigHash" hash table (part of the sqlite3* that represents the 
 *    database). This allows Trigger structures to be retrieved by name.
 * 2. All triggers associated with a single table form a linked list, using the
 *    pNext member of struct Trigger. A pointer to the first element of the
 *    linked list is stored as the "pTrigger" member of the associated
 *    struct Table.
 *
 * The "step_list" member points to the first element of a linked list
 * containing the SQL statements specified as the trigger program.
 */
struct Trigger {
  char *name;             /* The name of the trigger                        */
  char *table;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Expr *pWhen;            /* The WHEN clause of the expresion (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  int foreach;            /* One of TK_ROW or TK_STATEMENT */
  Token nameToken;        /* Token containing zName. Use during parsing only */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  TriggerStep *step_list; /* Link list of trigger program steps             */
  Trigger *pNext;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which. 
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#define TRIGGER_BEFORE  1
#define TRIGGER_AFTER   2

/*
 * An instance of struct TriggerStep is used to store a single SQL statement
 * that is a part of a trigger-program. 
 *
 * Instances of struct TriggerStep are stored in a singly linked list (linked
 * using the "pNext" member) referenced by the "step_list" member of the 
 * associated struct Trigger instance. The first element of the linked list is
 * the first step of the trigger-program.
 * 
 * The "op" member indicates whether this is a "DELETE", "INSERT", "UPDATE" or
 * "SELECT" statement. The meanings of the other members is determined by the 
 * value of "op" as follows:
 *
 * (op == TK_INSERT)
 * orconf    -> stores the ON CONFLICT algorithm
 * pSelect   -> If this is an INSERT INTO ... SELECT ... statement, then
 *              this stores a pointer to the SELECT statement. Otherwise NULL.
 * target    -> A token holding the name of the table to insert into.
 * pExprList -> If this is an INSERT INTO ... VALUES ... statement, then
 *              this stores values to be inserted. Otherwise NULL.
 * pIdList   -> If this is an INSERT INTO ... (<column-names>) VALUES ... 
 *              statement, then this stores the column-names to be
 *              inserted into.
 *
 * (op == TK_DELETE)
 * target    -> A token holding the name of the table to delete from.
 * pWhere    -> The WHERE clause of the DELETE statement if one is specified.
 *              Otherwise NULL.
 * 
 * (op == TK_UPDATE)
 * target    -> A token holding the name of the table to update rows of.
 * pWhere    -> The WHERE clause of the UPDATE statement if one is specified.
 *              Otherwise NULL.
 * pExprList -> A list of the columns to update and the expressions to update
 *              them to. See sqlite3Update() documentation of "pChanges"
 *              argument.
 * 
 */
struct TriggerStep {
  int op;              /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT */
  int orconf;          /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */

  Select *pSelect;     /* Valid for SELECT and sometimes 
			  INSERT steps (when pExprList == 0) */
  Token target;        /* Valid for DELETE, UPDATE, INSERT steps */
  Expr *pWhere;        /* Valid for DELETE, UPDATE steps */
  ExprList *pExprList; /* Valid for UPDATE statements and sometimes 
			   INSERT steps (when pSelect == 0)         */
  IdList *pIdList;     /* Valid for INSERT statements only */
  TriggerStep *pNext;  /* Next in the link-list */
  TriggerStep *pLast;  /* Last element in link-list. Valid for 1st elem only */
};

/*
 * An instance of struct TriggerStack stores information required during code
 * generation of a single trigger program. While the trigger program is being
 * coded, its associated TriggerStack instance is pointed to by the
 * "pTriggerStack" member of the Parse structure.
 *
 * The pTab member points to the table that triggers are being coded on. The 
 * newIdx member contains the index of the vdbe cursor that points at the temp
 * table that stores the new.* references. If new.* references are not valid
 * for the trigger being coded (for example an ON DELETE trigger), then newIdx
 * is set to -1. The oldIdx member is analogous to newIdx, for old.* references.
 *
 * The ON CONFLICT policy to be used for the trigger program steps is stored 
 * as the orconf member. If this is OE_Default, then the ON CONFLICT clause 
 * specified for individual triggers steps is used.
 *
 * struct TriggerStack has a "pNext" member, to allow linked lists to be
 * constructed. When coding nested triggers (triggers fired by other triggers)
 * each nested trigger stores its parent trigger's TriggerStack as the "pNext" 
 * pointer. Once the nested trigger has been coded, the pNext value is restored
 * to the pTriggerStack member of the Parse stucture and coding of the parent
 * trigger continues.
 *
 * Before a nested trigger is coded, the linked list pointed to by the 
 * pTriggerStack is scanned to ensure that the trigger is not about to be coded
 * recursively. If this condition is detected, the nested trigger is not coded.
 */
struct TriggerStack {
  Table *pTab;         /* Table that triggers are currently being coded on */
  int newIdx;          /* Index of vdbe cursor to "new" temp table */
  int oldIdx;          /* Index of vdbe cursor to "old" temp table */
  int orconf;          /* Current orconf policy */
  int ignoreJump;      /* where to jump to for a RAISE(IGNORE) */
  Trigger *pTrigger;   /* The trigger currently being coded */
  TriggerStack *pNext; /* Next trigger down on the trigger stack */
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.  
*/
typedef struct DbFixer DbFixer;
struct DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  const char *zDb;    /* Make sure all objects are contained in this database */
  const char *zType;  /* Type of the container - used for error messages */
  const Token *pName; /* Name of the container - used for error messages */
};

/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
typedef struct {
  sqlite3 *db;        /* The database being initialized */
  char **pzErrMsg;    /* Error message stored here */
} InitData;

/*
 * This global flag is set for performance testing of triggers. When it is set
 * SQLite will perform the overhead of building new and old trigger references 
 * even when no triggers exist
 */
extern int sqlite3_always_code_trigger_setup;


/*
** The SQLITE_CORRUPT_BKPT macro can be either a constant (for production
** builds) or a function call (for debugging).  If it is a function call,
** it allows the operator to set a breakpoint at the spot where database
** corruption is first detected.
*/
#ifdef SQLITE_DEBUG
  extern int sqlite3Corrupt(void);
# define SQLITE_CORRUPT_BKPT sqlite3Corrupt()
#else
# define SQLITE_CORRUPT_BKPT SQLITE_CORRUPT
#endif

/*
** Internal function prototypes
*/
int sqlite3StrICmp(const char *, const char *);
int sqlite3StrNICmp(const char *, const char *, int);
int sqlite3HashNoCase(const char *, int);
int sqlite3IsNumber(const char*, int*, u8);
int sqlite3Compare(const char *, const char *);
int sqlite3SortCompare(const char *, const char *);
void sqlite3RealToSortable(double r, char *);

void *sqlite3Malloc(int,int);
void *sqlite3MallocRaw(int,int);
void sqlite3Free(void*);
void *sqlite3Realloc(void*,int);
char *sqlite3StrDup(const char*);
char *sqlite3StrNDup(const char*, int);
# define sqlite3CheckMemory(a,b)
void sqlite3ReallocOrFree(void**,int);
void sqlite3FreeX(void*);
void *sqlite3MallocX(int);
int sqlite3AllocSize(void *);

char *sqlite3MPrintf(const char*, ...);
char *sqlite3VMPrintf(const char*, va_list);
void sqlite3DebugPrintf(const char*, ...);
void *sqlite3TextToPtr(const char*);
void sqlite3SetString(char **, ...);
void sqlite3ErrorMsg(Parse*, const char*, ...);
void sqlite3ErrorClear(Parse*);
void sqlite3Dequote(char*);
void sqlite3DequoteExpr(Expr*);
int sqlite3KeywordCode(const unsigned char*, int);
int sqlite3RunParser(Parse*, const char*, char **);
void sqlite3FinishCoding(Parse*);
Expr *sqlite3Expr(int, Expr*, Expr*, const Token*);
Expr *sqlite3RegisterExpr(Parse*,Token*);
Expr *sqlite3ExprAnd(Expr*, Expr*);
void sqlite3ExprSpan(Expr*,Token*,Token*);
Expr *sqlite3ExprFunction(ExprList*, Token*);
void sqlite3ExprAssignVarNumber(Parse*, Expr*);
void sqlite3ExprDelete(Expr*);
ExprList *sqlite3ExprListAppend(ExprList*,Expr*,Token*);
void sqlite3ExprListDelete(ExprList*);
int sqlite3Init(sqlite3*, char**);
int sqlite3InitCallback(void*, int, char**, char**);
void sqlite3Pragma(Parse*,Token*,Token*,Token*,int);
void sqlite3ResetInternalSchema(sqlite3*, int);
void sqlite3BeginParse(Parse*,int);
void sqlite3RollbackInternalChanges(sqlite3*);
void sqlite3CommitInternalChanges(sqlite3*);
Table *sqlite3ResultSetOfSelect(Parse*,char*,Select*);
void sqlite3OpenMasterTable(Parse *, int);
void sqlite3StartTable(Parse*,Token*,Token*,int,int,int);
void sqlite3AddColumn(Parse*,Token*);
void sqlite3AddNotNull(Parse*, int);
void sqlite3AddPrimaryKey(Parse*, ExprList*, int, int, int);
void sqlite3AddCheckConstraint(Parse*, Expr*);
void sqlite3AddColumnType(Parse*,Token*);
void sqlite3AddDefaultValue(Parse*,Expr*);
void sqlite3AddCollateType(Parse*, const char*, int);
void sqlite3EndTable(Parse*,Token*,Token*,Select*);

#ifndef SQLITE_OMIT_VIEW
  void sqlite3CreateView(Parse*,Token*,Token*,Token*,Select*,int);
  int sqlite3ViewGetColumnNames(Parse*,Table*);
#else
# define sqlite3ViewGetColumnNames(A,B) 0
#endif

void sqlite3DropTable(Parse*, SrcList*, int, int);
void sqlite3DeleteTable(sqlite3*, Table*);
//void sqlite3Insert(Parse*, SrcList*, ExprList*, Select*, IdList*, int);
void sqlite3Insert(Parse*, SrcList*, ExprList*, ValuesList*, Select*, IdList*, int);
int sqlite3ArrayAllocate(void**,int,int);
IdList *sqlite3IdListAppend(IdList*, Token*);
int sqlite3IdListIndex(IdList*,const char*);
SrcList *sqlite3SrcListAppend(SrcList*, Token*, Token*);
void sqlite3SrcListAddAlias(SrcList*, Token*);
void sqlite3SrcListAssignCursors(Parse*, SrcList*);
void sqlite3IdListDelete(IdList*);
void sqlite3SrcListDelete(SrcList*);
void sqlite3CreateIndex(Parse*,Token*,Token*,SrcList*,ExprList*,int,Token*,
                        Token*, int, int);
void sqlite3DropIndex(Parse*, SrcList*, int);
//void sqlite3AddKeyType(Vdbe*, ExprList*);
//void sqlite3AddIdxKeyType(Vdbe*, Index*);
int sqlite3Select(Parse*, Select*, int, int, Select*, int, int*, char *aff);
Select *sqlite3SelectNew(ExprList*,SrcList*,Expr*,ExprList*,Expr*,ExprList*,
                        int,Expr*,Expr*);
void sqlite3SelectDelete(Select*);
void sqlite3SelectUnbind(Select*);
Table *sqlite3SrcListLookup(Parse*, SrcList*);
int sqlite3IsReadOnly(Parse*, Table*, int);
void sqlite3OpenTable(Parse*, int iCur, int iDb, Table*, int);
void sqlite3DeleteFrom(Parse*, SrcList*, Expr*, Expr*, Expr*);
void sqlite3Update(Parse*, SrcList*, ExprList*, Expr*, int, Expr*, Expr*);
WhereInfo *sqlite3WhereBegin(Parse*, SrcList*, Expr*, ExprList**);
void sqlite3WhereEnd(WhereInfo*);
void sqlite3ExprCode(Parse*, Expr*);
void sqlite3ExprCodeAndCache(Parse*, Expr*);
int sqlite3ExprCodeExprList(Parse*, ExprList*);
void sqlite3ExprIfTrue(Parse*, Expr*, int, int);
void sqlite3ExprIfFalse(Parse*, Expr*, int, int);
void sqlite3NextedParse(Parse*, const char*, ...);
Table *sqlite3FindTable(sqlite3*,const char*, const char*);
Table *sqlite3LocateTable(Parse*,const char*, const char*);
Index *sqlite3FindIndex(sqlite3*,const char*, const char*);
void sqlite3UnlinkAndDeleteTable(sqlite3*,int,const char*);
void sqlite3UnlinkAndDeleteIndex(sqlite3*,int,const char*);
void sqlite3Vacuum(Parse*);
int sqlite3RunVacuum(char**, sqlite3*);
char *sqlite3NameFromToken(Token*);
int sqlite3ExprCheck(Parse*, Expr*, int, int*);
int sqlite3ExprCompare(Expr*, Expr*);
int sqliteFuncId(Token*);
int sqlite3ExprResolveNames(NameContext *, Expr *);
int sqlite3ExprAnalyzeAggregates(NameContext*, Expr*);
int sqlite3ExprAnalyzeAggList(NameContext*,ExprList*);
//Vdbe *sqlite3GetVdbe(Parse*);
void sqlite3Randomness(int, void*);
void sqlite3RollbackAll(sqlite3*);
void sqlite3CodeVerifySchema(Parse*, int);
void sqlite3BeginTransaction(Parse*, int);
void sqlite3CommitTransaction(Parse*);
void sqlite3RollbackTransaction(Parse*);
int sqlite3ExprIsConstant(Expr*);
int sqlite3ExprIsConstantOrFunction(Expr*);
int sqlite3ExprIsInteger(Expr*, int*);
int sqlite3IsRowid(const char*);
//void sqlite3GenerateRowDelete(sqlite3*, Vdbe*, Table*, int, int);
//void sqlite3GenerateRowIndexDelete(Vdbe*, Table*, int, char*);
//void sqlite3GenerateIndexKey(Vdbe*, Index*, int);
void sqlite3GenerateConstraintChecks(Parse*,Table*,int,char*,int,int,int,int);
void sqlite3CompleteInsertion(Parse*, Table*, int, char*, int, int, int);
void sqlite3OpenTableAndIndices(Parse*, Table*, int, int);
void sqlite3BeginWriteOperation(Parse*, int, int);
Expr *sqlite3ExprDup(Expr*);
void sqlite3TokenCopy(Token*, Token*);
ExprList *sqlite3ExprListDup(ExprList*);
SrcList *sqlite3SrcListDup(SrcList*);
IdList *sqlite3IdListDup(IdList*);
Select *sqlite3SelectDup(Select*);
FuncDef *sqlite3FindFunction(sqlite3*,const char*,int,int,u8,int);
void sqlite3RegisterBuiltinFunctions(sqlite3*);
void sqlite3RegisterDateTimeFunctions(sqlite3*);
int sqlite3SafetyOn(sqlite3*);
int sqlite3SafetyOff(sqlite3*);
int sqlite3SafetyCheck(sqlite3*);
//void sqlite3ChangeCookie(sqlite3*, Vdbe*, int);

#ifndef SQLITE_OMIT_TRIGGER
  void sqlite3BeginTrigger(Parse*, Token*,Token*,int,int,IdList*,SrcList*,
                           int,Expr*,int);
  void sqlite3FinishTrigger(Parse*, TriggerStep*, Token*);
  void sqlite3DropTrigger(Parse*, SrcList*);
  void sqlite3DropTriggerPtr(Parse*, Trigger*);
  int sqlite3TriggersExist(Parse*, Table*, int, ExprList*);
  int sqlite3CodeRowTrigger(Parse*, int, ExprList*, int, Table *, int, int, 
                           int, int);
  void sqliteViewTriggers(Parse*, Table*, Expr*, int, ExprList*);
  void sqlite3DeleteTriggerStep(TriggerStep*);
  TriggerStep *sqlite3TriggerSelectStep(Select*);
  TriggerStep *sqlite3TriggerInsertStep(Token*, IdList*, ExprList*,Select*,int);
  TriggerStep *sqlite3TriggerUpdateStep(Token*, ExprList*, Expr*, int);
  TriggerStep *sqlite3TriggerDeleteStep(Token*, Expr*);
  void sqlite3DeleteTrigger(Trigger*);
  void sqlite3UnlinkAndDeleteTrigger(sqlite3*,int,const char*);
#else
# define sqlite3TriggersExist(A,B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3CodeRowTrigger(A,B,C,D,E,F,G,H,I) 0
#endif

int sqlite3JoinType(Parse*, Token*, Token*, Token*);
void sqlite3CreateForeignKey(Parse*, ExprList*, Token*, ExprList*, int);
void sqlite3DeferForeignKey(Parse*, int);
#ifndef SQLITE_OMIT_AUTHORIZATION
  void sqlite3AuthRead(Parse*,Expr*,SrcList*);
  int sqlite3AuthCheck(Parse*,int, const char*, const char*, const char*);
  void sqlite3AuthContextPush(Parse*, AuthContext*, const char*);
  void sqlite3AuthContextPop(AuthContext*);
#else
# define sqlite3AuthRead(a,b,c)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3AuthContextPush(a,b,c)
# define sqlite3AuthContextPop(a)  ((void)(a))
#endif
void sqlite3Attach(Parse*, Expr*, Expr*, Expr*);
void sqlite3Detach(Parse*, Expr*);
//int sqlite3BtreeFactory(const sqlite3 *db, const char *zFilename,
//                       int omitJournal, int nCache, Btree **ppBtree);
int sqlite3FixInit(DbFixer*, Parse*, int, const char*, const Token*);
int sqlite3FixSrcList(DbFixer*, SrcList*);
int sqlite3FixSelect(DbFixer*, Select*);
int sqlite3FixExpr(DbFixer*, Expr*);
int sqlite3FixExprList(DbFixer*, ExprList*);
int sqlite3FixTriggerStep(DbFixer*, TriggerStep*);
int sqlite3AtoF(const char *z, double*);
char *sqlite3_snprintf(int,char*,const char*,...);
int sqlite3GetInt32(const char *, int*);
int sqlite3FitsIn64Bits(const char *);
int sqlite3utf16ByteLen(const void *pData, int nChar);
int sqlite3utf8CharLen(const char *pData, int nByte);
int sqlite3ReadUtf8(const unsigned char *);
int sqlite3PutVarint(unsigned char *, u64);
int sqlite3GetVarint(const unsigned char *, u64 *);
int sqlite3GetVarint32(const unsigned char *, u32 *);
int sqlite3VarintLen(u64 v);
//void sqlite3IndexAffinityStr(Vdbe *, Index *);
//void sqlite3TableAffinityStr(Vdbe *, Table *);
char sqlite3CompareAffinity(Expr *pExpr, char aff2);
int sqlite3IndexAffinityOk(Expr *pExpr, char idx_affinity);
char sqlite3ExprAffinity(Expr *pExpr);
int sqlite3atoi64(const char*, i64*);
void sqlite3Error(sqlite3*, int, const char*,...);
void *sqlite3HexToBlob(const char *z);
int sqlite3TwoPartName(Parse *, Token *, Token *, Token **);
const char *sqlite3ErrStr(int);
int sqlite3ReadUniChar(const char *zStr, int *pOffset, u8 *pEnc, int fold);
int sqlite3ReadSchema(Parse *pParse);
CollSeq *sqlite3FindCollSeq(sqlite3*,u8 enc, const char *,int,int);
CollSeq *sqlite3LocateCollSeq(Parse *pParse, const char *zName, int nName);
CollSeq *sqlite3ExprCollSeq(Parse *pParse, Expr *pExpr);
int sqlite3CheckCollSeq(Parse *, CollSeq *);
int sqlite3CheckIndexCollSeq(Parse *, Index *);
int sqlite3CheckObjectName(Parse *, const char *);
void sqlite3VdbeSetChanges(sqlite3 *, int);
void sqlite3utf16Substr(sqlite3_context *,int,sqlite3_value **);

const void *sqlite3ValueText(sqlite3_value*, u8);
int sqlite3ValueBytes(sqlite3_value*, u8);
void sqlite3ValueSetStr(sqlite3_value*, int, const void *,u8, void(*)(void*));
void sqlite3ValueFree(sqlite3_value*);
sqlite3_value *sqlite3ValueNew(void);
char *sqlite3utf16to8(const void*, int);
int sqlite3ValueFromExpr(Expr *, u8, u8, sqlite3_value **);
void sqlite3ValueApplyAffinity(sqlite3_value *, u8, u8);
extern const unsigned char sqlite3UpperToLower[];
void sqlite3RootPageMoved(Db*, int, int);
void sqlite3Reindex(Parse*, Token*, Token*);
void sqlite3AlterFunctions(sqlite3*);
void sqlite3AlterRenameTable(Parse*, SrcList*, Token*);
int sqlite3GetToken(const unsigned char *, int *);
void sqlite3NestedParse(Parse*, const char*, ...);
void sqlite3ExpirePreparedStatements(sqlite3*);
void sqlite3CodeSubselect(Parse *, Expr *);
int sqlite3SelectResolve(Parse *, Select *, NameContext *);
//void sqlite3ColumnDefault(Vdbe *, Table *, int);
void sqlite3AlterFinishAddColumn(Parse *, Token *);
void sqlite3AlterBeginAddColumn(Parse *, SrcList *);
const char *sqlite3TestErrorName(int);
CollSeq *sqlite3GetCollSeq(sqlite3*, CollSeq *, const char *, int);
char sqlite3AffinityType(const Token*);
void sqlite3Analyze(Parse*, Token*, Token*);
int sqlite3InvokeBusyHandler(BusyHandler*);
int sqlite3FindDb(sqlite3*, Token*);
void sqlite3AnalysisLoad(sqlite3*,int iDB);
void sqlite3DefaultRowEst(Index*);
void sqlite3RegisterLikeFunctions(sqlite3*, int);
int sqlite3IsLikeFunction(sqlite3*,Expr*,int*,char*);
ThreadData *sqlite3ThreadData(void);
const ThreadData *sqlite3ThreadDataReadOnly(void);
void sqlite3ReleaseThreadData(void);
void sqlite3AttachFunctions(sqlite3 *);
void sqlite3MinimumFileFormat(Parse*, int, int);
void sqlite3SchemaFree(void *);
//Schema *sqlite3SchemaGet(Btree *);
int sqlite3SchemaToIndex(sqlite3 *db, Schema *);
KeyInfo *sqlite3IndexKeyinfo(Parse *, Index *);
int sqlite3CreateFunc(sqlite3 *, const char *, int, int, void *, 
  void (*)(sqlite3_context*,int,sqlite3_value **),
  void (*)(sqlite3_context*,int,sqlite3_value **), void (*)(sqlite3_context*));
int sqlite3ApiExit(sqlite3 *db, int);
int sqlite3MallocFailed(void);
void sqlite3FailedMalloc(void);
//void sqlite3AbortOtherActiveVdbes(sqlite3 *, Vdbe *);
int sqlite3OpenTempDatabase(Parse *);

Expr *sqlite3ExprLikeOp(ExprList *pList, Token *pToken);

int sqlite3RunParser(Parse *pParse, const char *zSql, char **pzErrMsg);
int sqlite3RunParser1(Parse *pParse, const char *zSql, int sqlLen, char **pzErrMsg);

void resetParseObject(Parse *parseObj);

Parse* sqlite3ParseNew();
Parse* sqlite3ParseInit(Parse *parseObj);
void sqlite3ParseReset(Parse *parseObj);
void   sqlite3ParseDelete(Parse *parseObj);

#define sqlite3ParseClean(X) sqlite3ParseReset(X)


TokenArray* sqlite3TokenArrayAppend(TokenArray *tokenArray, TokenItem *item);
ParsedResultArray* sqlite3ParsedResultArrayAppend(ParsedResultArray *resultArray, ParsedResultItem *result);
void sqlite3ParsedResultArrayClean(ParsedResultArray *resultArray);

Insert* sqlite3InsertNew(SrcList *pTabList, ExprList *pSetList, ValuesList *pValuesList, Select *pSelect, IdList *pColumn, int onError);
void sqlite3InsertDelete(Insert *insertObj);

Delete* sqlite3DeleteNew(SrcList *pTabList, Expr *pWhere, Expr *pLimit, Expr *pOffset);
void sqlite3DeleteFree(Delete *deleteObj);

Update* sqlite3UpdateNew(SrcList *pTabList, ExprList *pChanges, Expr *pWhere, int onError, Expr *pLimit, Expr *pOffset);
void sqlite3UpdateDelete(Update* updateObj);

SetStatement* sqlite3SetStatementNew(ExprList *pList, Token *pValue);
void sqlite3SetStatementDelete(SetStatement* setObj);

ValuesList *sqlite3ValuesListAppend(ValuesList *valueList, ExprList *exprList);
void sqlite3ValuesListDelete(ValuesList *valuesList);

void sqlite3SetStatement(Parse *pParse, ExprList *pExprList, Token *pToken, SqlType sqltype);
void sqlite3CheckSetScope(Parse *pParse, Token *pScope);

void sqlite3ShowStatement(Parse *pParse, ShowStatementType showtype);

#ifndef SQLITE_OMIT_SHARED_CACHE
  void sqlite3TableLock(Parse *, int, int, u8, const char *);
#else
  #define sqlite3TableLock(v,w,x,y,z)
#endif

#ifdef SQLITE_MEMDEBUG
  void sqlite3MallocDisallow(void);
  void sqlite3MallocAllow(void);
  int sqlite3TestMallocFail(void);
#else
  #define sqlite3TestMallocFail() 0
  #define sqlite3MallocDisallow()
  #define sqlite3MallocAllow()
#endif

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
  void *sqlite3ThreadSafeMalloc(int);
  void sqlite3ThreadSafeFree(void *);
#else
  #define sqlite3ThreadSafeMalloc sqlite3MallocX
  #define sqlite3ThreadSafeFree sqlite3FreeX
#endif

#ifdef SQLITE_SSE
#include "sseInt.h"
#endif

#ifdef __cplusplus
}
#endif

#endif

/*********************************************file from os.h*****************************************/
/*
** 2001 September 16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This header file (together with is companion C source-code file
** "os.c") attempt to abstract the underlying operating system so that
** the SQLite library will work on both POSIX and windows systems.
*/
#ifndef _SQLITE_OS_H_
#define _SQLITE_OS_H_

/*
** Figure out if we are dealing with Unix, Windows, or some other
** operating system.
*/
#if !defined(OS_UNIX) && !defined(OS_OTHER)
# define OS_OTHER 0
# ifndef OS_WIN
#   if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#     define OS_WIN 1
#     define OS_UNIX 0
#   else
#     define OS_WIN 0
#     define OS_UNIX 1
#  endif
# else
#  define OS_UNIX 0
# endif
#else
# ifndef OS_WIN
#  define OS_WIN 0
# endif
#endif


/*
** Define the maximum size of a temporary filename
*/
#if OS_WIN
# include <windows.h>
# define SQLITE_TEMPNAME_SIZE (MAX_PATH+50)
#else
# define SQLITE_TEMPNAME_SIZE 200
#endif

/* If the SET_FULLSYNC macro is not defined above, then make it
** a no-op
*/
#ifndef SET_FULLSYNC
# define SET_FULLSYNC(x,y)
#endif

/*
** Temporary files are named starting with this prefix followed by 16 random
** alphanumeric characters, and no file extension. They are stored in the
** OS's standard temporary file directory, and are deleted prior to exit.
** If sqlite is being embedded in another program, you may wish to change the
** prefix to reflect your program's name, so that if your program exits
** prematurely, old temporary files can be easily identified. This can be done
** using -DTEMP_FILE_PREFIX=myprefix_ on the compiler command line.
*/
#ifndef TEMP_FILE_PREFIX
# define TEMP_FILE_PREFIX "sqlite_"
#endif

/*
** Define the interfaces for Unix and for Windows.
*/
#if OS_UNIX
#define sqlite3OsOpenReadWrite      sqlite3UnixOpenReadWrite
#define sqlite3OsOpenExclusive      sqlite3UnixOpenExclusive
#define sqlite3OsOpenReadOnly       sqlite3UnixOpenReadOnly
#define sqlite3OsDelete             sqlite3UnixDelete
#define sqlite3OsFileExists         sqlite3UnixFileExists
#define sqlite3OsFullPathname       sqlite3UnixFullPathname
#define sqlite3OsIsDirWritable      sqlite3UnixIsDirWritable
#define sqlite3OsSyncDirectory      sqlite3UnixSyncDirectory
#define sqlite3OsTempFileName       sqlite3UnixTempFileName
#define sqlite3OsRandomSeed         sqlite3UnixRandomSeed
#define sqlite3OsSleep              sqlite3UnixSleep
#define sqlite3OsCurrentTime        sqlite3UnixCurrentTime
#define sqlite3OsEnterMutex         sqlite3UnixEnterMutex
#define sqlite3OsLeaveMutex         sqlite3UnixLeaveMutex
#define sqlite3OsInMutex            sqlite3UnixInMutex
#define sqlite3OsThreadSpecificData sqlite3UnixThreadSpecificData
#define sqlite3OsMalloc             sqlite3GenericMalloc
#define sqlite3OsRealloc            sqlite3GenericRealloc
#define sqlite3OsFree               sqlite3GenericFree
#define sqlite3OsAllocationSize     sqlite3GenericAllocationSize
#endif
#if OS_WIN
#define sqlite3OsOpenReadWrite      sqlite3WinOpenReadWrite
#define sqlite3OsOpenExclusive      sqlite3WinOpenExclusive
#define sqlite3OsOpenReadOnly       sqlite3WinOpenReadOnly
#define sqlite3OsDelete             sqlite3WinDelete
#define sqlite3OsFileExists         sqlite3WinFileExists
#define sqlite3OsFullPathname       sqlite3WinFullPathname
#define sqlite3OsIsDirWritable      sqlite3WinIsDirWritable
#define sqlite3OsSyncDirectory      sqlite3WinSyncDirectory
#define sqlite3OsTempFileName       sqlite3WinTempFileName
#define sqlite3OsRandomSeed         sqlite3WinRandomSeed
#define sqlite3OsSleep              sqlite3WinSleep
#define sqlite3OsCurrentTime        sqlite3WinCurrentTime
#define sqlite3OsEnterMutex         sqlite3WinEnterMutex
#define sqlite3OsLeaveMutex         sqlite3WinLeaveMutex
#define sqlite3OsInMutex            sqlite3WinInMutex
#define sqlite3OsThreadSpecificData sqlite3WinThreadSpecificData
#define sqlite3OsMalloc             sqlite3GenericMalloc
#define sqlite3OsRealloc            sqlite3GenericRealloc
#define sqlite3OsFree               sqlite3GenericFree
#define sqlite3OsAllocationSize     sqlite3GenericAllocationSize
#endif

/*
** If using an alternative OS interface, then we must have an "os_other.h"
** header file available for that interface.  Presumably the "os_other.h"
** header file contains #defines similar to those above.
*/
#if OS_OTHER
# include "os_other.h"
#endif



/*
** Forward declarations
*/
typedef struct OsFile OsFile;
typedef struct IoMethod IoMethod;

/*
** An instance of the following structure contains pointers to all
** methods on an OsFile object.
*/
struct IoMethod {
  int (*xClose)(OsFile**);
  int (*xOpenDirectory)(OsFile*, const char*);
  int (*xRead)(OsFile*, void*, int amt);
  int (*xWrite)(OsFile*, const void*, int amt);
  int (*xSeek)(OsFile*, i64 offset);
  int (*xTruncate)(OsFile*, i64 size);
  int (*xSync)(OsFile*, int);
  void (*xSetFullSync)(OsFile *id, int setting);
  int (*xFileHandle)(OsFile *id);
  int (*xFileSize)(OsFile*, i64 *pSize);
  int (*xLock)(OsFile*, int);
  int (*xUnlock)(OsFile*, int);
  int (*xLockState)(OsFile *id);
  int (*xCheckReservedLock)(OsFile *id);
};

/*
** The OsFile object describes an open disk file in an OS-dependent way.
** The version of OsFile defined here is a generic version.  Each OS
** implementation defines its own subclass of this structure that contains
** additional information needed to handle file I/O.  But the pMethod
** entry (pointing to the virtual function table) always occurs first
** so that we can always find the appropriate methods.
*/
struct OsFile {
  IoMethod const *pMethod;
};

/*
** The following values may be passed as the second argument to
** sqlite3OsLock(). The various locks exhibit the following semantics:
**
** SHARED:    Any number of processes may hold a SHARED lock simultaneously.
** RESERVED:  A single process may hold a RESERVED lock on a file at
**            any time. Other processes may hold and obtain new SHARED locks.
** PENDING:   A single process may hold a PENDING lock on a file at
**            any one time. Existing SHARED locks may persist, but no new
**            SHARED locks may be obtained by other processes.
** EXCLUSIVE: An EXCLUSIVE lock precludes all other locks.
**
** PENDING_LOCK may not be passed directly to sqlite3OsLock(). Instead, a
** process that requests an EXCLUSIVE lock may actually obtain a PENDING
** lock. This can be upgraded to an EXCLUSIVE lock by a subsequent call to
** sqlite3OsLock().
*/
#define NO_LOCK         0
#define SHARED_LOCK     1
#define RESERVED_LOCK   2
#define PENDING_LOCK    3
#define EXCLUSIVE_LOCK  4

/*
** File Locking Notes:  (Mostly about windows but also some info for Unix)
**
** We cannot use LockFileEx() or UnlockFileEx() on Win95/98/ME because
** those functions are not available.  So we use only LockFile() and
** UnlockFile().
**
** LockFile() prevents not just writing but also reading by other processes.
** A SHARED_LOCK is obtained by locking a single randomly-chosen 
** byte out of a specific range of bytes. The lock byte is obtained at 
** random so two separate readers can probably access the file at the 
** same time, unless they are unlucky and choose the same lock byte.
** An EXCLUSIVE_LOCK is obtained by locking all bytes in the range.
** There can only be one writer.  A RESERVED_LOCK is obtained by locking
** a single byte of the file that is designated as the reserved lock byte.
** A PENDING_LOCK is obtained by locking a designated byte different from
** the RESERVED_LOCK byte.
**
** On WinNT/2K/XP systems, LockFileEx() and UnlockFileEx() are available,
** which means we can use reader/writer locks.  When reader/writer locks
** are used, the lock is placed on the same range of bytes that is used
** for probabilistic locking in Win95/98/ME.  Hence, the locking scheme
** will support two or more Win95 readers or two or more WinNT readers.
** But a single Win95 reader will lock out all WinNT readers and a single
** WinNT reader will lock out all other Win95 readers.
**
** The following #defines specify the range of bytes used for locking.
** SHARED_SIZE is the number of bytes available in the pool from which
** a random byte is selected for a shared lock.  The pool of bytes for
** shared locks begins at SHARED_FIRST. 
**
** These #defines are available in sqlite_aux.h so that adaptors for
** connecting SQLite to other operating systems can use the same byte
** ranges for locking.  In particular, the same locking strategy and
** byte ranges are used for Unix.  This leaves open the possiblity of having
** clients on win95, winNT, and unix all talking to the same shared file
** and all locking correctly.  To do so would require that samba (or whatever
** tool is being used for file sharing) implements locks correctly between
** windows and unix.  I'm guessing that isn't likely to happen, but by
** using the same locking range we are at least open to the possibility.
**
** Locking in windows is manditory.  For this reason, we cannot store
** actual data in the bytes used for locking.  The pager never allocates
** the pages involved in locking therefore.  SHARED_SIZE is selected so
** that all locks will fit on a single page even at the minimum page size.
** PENDING_BYTE defines the beginning of the locks.  By default PENDING_BYTE
** is set high so that we don't have to allocate an unused page except
** for very large databases.  But one should test the page skipping logic 
** by setting PENDING_BYTE low and running the entire regression suite.
**
** Changing the value of PENDING_BYTE results in a subtly incompatible
** file format.  Depending on how it is changed, you might not notice
** the incompatibility right away, even running a full regression test.
** The default location of PENDING_BYTE is the first byte past the
** 1GB boundary.
**
*/
#ifndef SQLITE_TEST
#define PENDING_BYTE      0x40000000  /* First byte past the 1GB boundary */
#else
extern unsigned int sqlite3_pending_byte;
#define PENDING_BYTE sqlite3_pending_byte
#endif

#define RESERVED_BYTE     (PENDING_BYTE+1)
#define SHARED_FIRST      (PENDING_BYTE+2)
#define SHARED_SIZE       510

/*
** Prototypes for operating system interface routines.
*/
int sqlite3OsClose(OsFile**);
int sqlite3OsOpenDirectory(OsFile*, const char*);
int sqlite3OsRead(OsFile*, void*, int amt);
int sqlite3OsWrite(OsFile*, const void*, int amt);
int sqlite3OsSeek(OsFile*, i64 offset);
int sqlite3OsTruncate(OsFile*, i64 size);
int sqlite3OsSync(OsFile*, int);
void sqlite3OsSetFullSync(OsFile *id, int setting);
int sqlite3OsFileHandle(OsFile *id);
int sqlite3OsFileSize(OsFile*, i64 *pSize);
int sqlite3OsLock(OsFile*, int);
int sqlite3OsUnlock(OsFile*, int);
int sqlite3OsLockState(OsFile *id);
int sqlite3OsCheckReservedLock(OsFile *id);
int sqlite3OsOpenReadWrite(const char*, OsFile**, int*);
int sqlite3OsOpenExclusive(const char*, OsFile**, int);
int sqlite3OsOpenReadOnly(const char*, OsFile**);
int sqlite3OsDelete(const char*);
int sqlite3OsFileExists(const char*);
char *sqlite3OsFullPathname(const char*);
int sqlite3OsIsDirWritable(char*);
int sqlite3OsSyncDirectory(const char*);
int sqlite3OsTempFileName(char*);
int sqlite3OsRandomSeed(char*);
int sqlite3OsSleep(int ms);
int sqlite3OsCurrentTime(double*);
void sqlite3OsEnterMutex(void);
void sqlite3OsLeaveMutex(void);
int sqlite3OsInMutex(int);
ThreadData *sqlite3OsThreadSpecificData(int);
void *sqlite3OsMalloc(int);
void *sqlite3OsRealloc(void *, int);
void sqlite3OsFree(void *);
int sqlite3OsAllocationSize(void *);

/*
** If the SQLITE_ENABLE_REDEF_IO macro is defined, then the OS-layer
** interface routines are not called directly but are invoked using
** pointers to functions.  This allows the implementation of various
** OS-layer interface routines to be modified at run-time.  There are
** obscure but legitimate reasons for wanting to do this.  But for
** most users, a direct call to the underlying interface is preferable
** so the the redefinable I/O interface is turned off by default.
*/
#ifdef SQLITE_ENABLE_REDEF_IO

/*
** When redefinable I/O is enabled, a single global instance of the
** following structure holds pointers to the routines that SQLite 
** uses to talk with the underlying operating system.  Modify this
** structure (before using any SQLite API!) to accomodate perculiar
** operating system interfaces or behaviors.
*/
struct sqlite3OsVtbl {
  int (*xOpenReadWrite)(const char*, OsFile**, int*);
  int (*xOpenExclusive)(const char*, OsFile**, int);
  int (*xOpenReadOnly)(const char*, OsFile**);

  int (*xDelete)(const char*);
  int (*xFileExists)(const char*);
  char *(*xFullPathname)(const char*);
  int (*xIsDirWritable)(char*);
  int (*xSyncDirectory)(const char*);
  int (*xTempFileName)(char*);

  int (*xRandomSeed)(char*);
  int (*xSleep)(int ms);
  int (*xCurrentTime)(double*);

  void (*xEnterMutex)(void);
  void (*xLeaveMutex)(void);
  int (*xInMutex)(int);
  ThreadData *(*xThreadSpecificData)(int);

  void *(*xMalloc)(int);
  void *(*xRealloc)(void *, int);
  void (*xFree)(void *);
  int (*xAllocationSize)(void *);
};

/* Macro used to comment out routines that do not exists when there is
** no disk I/O 
*/
#ifdef SQLITE_OMIT_DISKIO
# define IF_DISKIO(X)  0
#else
# define IF_DISKIO(X)  X
#endif

#ifdef _SQLITE_OS_C_
  /*
  ** The os.c file implements the global virtual function table.
  */
  struct sqlite3OsVtbl sqlite3Os = {
    IF_DISKIO( sqlite3OsOpenReadWrite ),
    IF_DISKIO( sqlite3OsOpenExclusive ),
    IF_DISKIO( sqlite3OsOpenReadOnly ),
    IF_DISKIO( sqlite3OsDelete ),
    IF_DISKIO( sqlite3OsFileExists ),
    IF_DISKIO( sqlite3OsFullPathname ),
    IF_DISKIO( sqlite3OsIsDirWritable ),
    IF_DISKIO( sqlite3OsSyncDirectory ),
    IF_DISKIO( sqlite3OsTempFileName ),
    sqlite3OsRandomSeed,
    sqlite3OsSleep,
    sqlite3OsCurrentTime,
    sqlite3OsEnterMutex,
    sqlite3OsLeaveMutex,
    sqlite3OsInMutex,
    sqlite3OsThreadSpecificData,
    sqlite3OsMalloc,
    sqlite3OsRealloc,
    sqlite3OsFree,
    sqlite3OsAllocationSize
  };
#else
  /*
  ** Files other than os.c just reference the global virtual function table. 
  */
  extern struct sqlite3OsVtbl sqlite3Os;
#endif /* _SQLITE_OS_C_ */


/* This additional API routine is available with redefinable I/O */
struct sqlite3OsVtbl *sqlite3_os_switch(void);


/*
** Redefine the OS interface to go through the virtual function table
** rather than calling routines directly.
*/
#undef sqlite3OsOpenReadWrite
#undef sqlite3OsOpenExclusive
#undef sqlite3OsOpenReadOnly
#undef sqlite3OsDelete
#undef sqlite3OsFileExists
#undef sqlite3OsFullPathname
#undef sqlite3OsIsDirWritable
#undef sqlite3OsSyncDirectory
#undef sqlite3OsTempFileName
#undef sqlite3OsRandomSeed
#undef sqlite3OsSleep
#undef sqlite3OsCurrentTime
#undef sqlite3OsEnterMutex
#undef sqlite3OsLeaveMutex
#undef sqlite3OsInMutex
#undef sqlite3OsThreadSpecificData
#undef sqlite3OsMalloc
#undef sqlite3OsRealloc
#undef sqlite3OsFree
#undef sqlite3OsAllocationSize
#define sqlite3OsOpenReadWrite      sqlite3Os.xOpenReadWrite
#define sqlite3OsOpenExclusive      sqlite3Os.xOpenExclusive
#define sqlite3OsOpenReadOnly       sqlite3Os.xOpenReadOnly
#define sqlite3OsDelete             sqlite3Os.xDelete
#define sqlite3OsFileExists         sqlite3Os.xFileExists
#define sqlite3OsFullPathname       sqlite3Os.xFullPathname
#define sqlite3OsIsDirWritable      sqlite3Os.xIsDirWritable
#define sqlite3OsSyncDirectory      sqlite3Os.xSyncDirectory
#define sqlite3OsTempFileName       sqlite3Os.xTempFileName
#define sqlite3OsRandomSeed         sqlite3Os.xRandomSeed
#define sqlite3OsSleep              sqlite3Os.xSleep
#define sqlite3OsCurrentTime        sqlite3Os.xCurrentTime
#define sqlite3OsEnterMutex         sqlite3Os.xEnterMutex
#define sqlite3OsLeaveMutex         sqlite3Os.xLeaveMutex
#define sqlite3OsInMutex            sqlite3Os.xInMutex
#define sqlite3OsThreadSpecificData sqlite3Os.xThreadSpecificData
#define sqlite3OsMalloc             sqlite3Os.xMalloc
#define sqlite3OsRealloc            sqlite3Os.xRealloc
#define sqlite3OsFree               sqlite3Os.xFree
#define sqlite3OsAllocationSize     sqlite3Os.xAllocationSize

#endif /* SQLITE_ENABLE_REDEF_IO */

#endif /* _SQLITE_OS_H_ */

/**************************************file from parse.h**********************************/
#define TK_LP                              1
#define TK_RP                              2
#define TK_SEMI                            3
#define TK_EXPLAIN                         4
#define TK_QUERY                           5
#define TK_PLAN                            6
#define TK_BEGIN                           7
#define TK_START                           8
#define TK_TRANSACTION                     9
#define TK_WORK                           10
#define TK_COMMIT                         11
#define TK_ROLLBACK                       12
#define TK_CREATE                         13
#define TK_TABLE                          14
#define TK_IF                             15
#define TK_NOT                            16
#define TK_EXISTS                         17
#define TK_TEMP                           18
#define TK_AS                             19
#define TK_COMMA                          20
#define TK_ID                             21
#define TK_EQ                             22
#define TK_DEFAULT                        23
#define TK_CHARSET                        24
#define TK_SET                            25
#define TK_COLLATE                        26
#define TK_ABORT                          27
#define TK_AFTER                          28
#define TK_ANALYZE                        29
#define TK_ASC                            30
#define TK_ATTACH                         31
#define TK_BEFORE                         32
#define TK_CASCADE                        33
#define TK_CAST                           34
#define TK_CONFLICT                       35
#define TK_DATABASE                       36
#define TK_DEFERRED                       37
#define TK_DESC                           38
#define TK_DETACH                         39
#define TK_EACH                           40
#define TK_END                            41
#define TK_EXCLUSIVE                      42
#define TK_FAIL                           43
#define TK_FOR                            44
#define TK_IGNORE                         45
#define TK_IMMEDIATE                      46
#define TK_INITIALLY                      47
#define TK_INSTEAD                        48
#define TK_LIKE_KW                        49
#define TK_MATCH                          50
#define TK_KEY                            51
#define TK_OF                             52
#define TK_OFFSET                         53
#define TK_PRAGMA                         54
#define TK_RAISE                          55
#define TK_REPLACE                        56
#define TK_RESTRICT                       57
#define TK_ROW                            58
#define TK_STATEMENT                      59
#define TK_TRIGGER                        60
#define TK_VACUUM                         61
#define TK_VIEW                           62
#define TK_REINDEX                        63
#define TK_RENAME                         64
#define TK_CTIME_KW                       65
#define TK_OR                             66
#define TK_AND                            67
#define TK_IS                             68
#define TK_BETWEEN                        69
#define TK_IN                             70
#define TK_ISNULL                         71
#define TK_NOTNULL                        72
#define TK_NE                             73
#define TK_GT                             74
#define TK_LE                             75
#define TK_LT                             76
#define TK_GE                             77
#define TK_ESCAPE                         78
#define TK_BITAND                         79
#define TK_BITOR                          80
#define TK_LSHIFT                         81
#define TK_RSHIFT                         82
#define TK_PLUS                           83
#define TK_MINUS                          84
#define TK_STAR                           85
#define TK_SLASH                          86
#define TK_REM                            87
#define TK_CONCAT                         88
#define TK_UMINUS                         89
#define TK_UPLUS                          90
#define TK_BITNOT                         91
#define TK_STRING                         92
#define TK_JOIN_KW                        93
#define TK_CONSTRAINT                     94
#define TK_AUTOINCR                       95
#define TK_NULL                           96
#define TK_PRIMARY                        97
#define TK_UNIQUE                         98
#define TK_CHECK                          99
#define TK_REFERENCES                     100
#define TK_ON                             101
#define TK_DELETE                         102
#define TK_UPDATE                         103
#define TK_INSERT                         104
#define TK_DEFERRABLE                     105
#define TK_FOREIGN                        106
#define TK_DROP                           107
#define TK_UNION                          108
#define TK_ALL                            109
#define TK_EXCEPT                         110
#define TK_INTERSECT                      111
#define TK_SELECT                         112
#define TK_DISTINCT                       113
#define TK_DOT                            114
#define TK_FROM                           115
#define TK_JOIN                           116
#define TK_USING                          117
#define TK_ORDER                          118
#define TK_BY                             119
#define TK_GROUP                          120
#define TK_HAVING                         121
#define TK_LIMIT                          122
#define TK_WHERE                          123
#define TK_INTO                           124
#define TK_VALUES                         125
#define TK_INTEGER                        126
#define TK_FLOAT                          127
#define TK_BLOB                           128
#define TK_REGISTER                       129
#define TK_VARIABLE                       130
#define TK_VARIABLE1                      131
#define TK_CASE                           132
#define TK_WHEN                           133
#define TK_THEN                           134
#define TK_ELSE                           135
#define TK_NAMES                          136
#define TK_CHARACTER                      137
#define TK_GLOBAL                         138
#define TK_LOCAL                          139
#define TK_SESSION                        140
#define TK_SHOW                           141
#define TK_DATABASES                      142
#define TK_SCHEMAS                        143
#define TK_TABLES                         144
#define TK_STATUS                         145
#define TK_VARIABLES                      146
#define TK_COLLATION                      147
#define TK_TO_TEXT                        148
#define TK_TO_BLOB                        149
#define TK_TO_NUMERIC                     150
#define TK_TO_INT                         151
#define TK_TO_REAL                        152
#define TK_END_OF_FILE                    153
#define TK_ILLEGAL                        154
#define TK_SPACE                          155
#define TK_UNCLOSED_STRING                156
#define TK_COMMENT                        157
#define TK_FUNCTION                       158
#define TK_COLUMN                         159
#define TK_AGG_FUNCTION                   160
#define TK_AGG_COLUMN                     161
#define TK_CONST_FUNC                     162
/*********************************************** file from os_common.h************************/
/*
** 2004 May 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains macros and a little bit of code that is common to
** all of the platform-specific files (os_*.c) and is #included into those
** files.
**
** This file should be #included by the os_*.c files only.  It is not a
** general purpose header file.
*/

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#ifdef MEMORY_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#endif


/*
 * When testing, this global variable stores the location of the
 * pending-byte in the database file.
 */
#ifdef SQLITE_TEST
unsigned int sqlite3_pending_byte = 0x40000000;
#endif

int sqlite3_os_trace = 0;
#ifdef SQLITE_DEBUG
static int last_page = 0;
#define SEEK(X)           last_page=(X)
#define TRACE1(X)         if( sqlite3_os_trace ) sqlite3DebugPrintf(X)
#define TRACE2(X,Y)       if( sqlite3_os_trace ) sqlite3DebugPrintf(X,Y)
#define TRACE3(X,Y,Z)     if( sqlite3_os_trace ) sqlite3DebugPrintf(X,Y,Z)
#define TRACE4(X,Y,Z,A)   if( sqlite3_os_trace ) sqlite3DebugPrintf(X,Y,Z,A)
#define TRACE5(X,Y,Z,A,B) if( sqlite3_os_trace ) sqlite3DebugPrintf(X,Y,Z,A,B)
#define TRACE6(X,Y,Z,A,B,C) if(sqlite3_os_trace) sqlite3DebugPrintf(X,Y,Z,A,B,C)
#define TRACE7(X,Y,Z,A,B,C,D) \
    if(sqlite3_os_trace) sqlite3DebugPrintf(X,Y,Z,A,B,C,D)
#else
#define SEEK(X)
#define TRACE1(X)
#define TRACE2(X,Y)
#define TRACE3(X,Y,Z)
#define TRACE4(X,Y,Z,A)
#define TRACE5(X,Y,Z,A,B)
#define TRACE6(X,Y,Z,A,B,C)
#define TRACE7(X,Y,Z,A,B,C,D)
#endif

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#ifdef SQLITE_PERFORMANCE_TRACE
__inline__ unsigned long long int hwtime(void){
  unsigned long long int x;
  __asm__("rdtsc\n\t"
          "mov %%edx, %%ecx\n\t"
          :"=A" (x));
  return x;
}
static unsigned long long int g_start;
static unsigned int elapse;
#define TIMER_START       g_start=hwtime()
#define TIMER_END         elapse=hwtime()-g_start
#define TIMER_ELAPSED     elapse
#else
#define TIMER_START
#define TIMER_END
#define TIMER_ELAPSED     0
#endif

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ifdef SQLITE_TEST
int sqlite3_io_error_hit = 0;
int sqlite3_io_error_pending = 0;
int sqlite3_diskfull_pending = 0;
int sqlite3_diskfull = 0;
#define SimulateIOError(A)  \
   if( sqlite3_io_error_pending ) \
     if( sqlite3_io_error_pending-- == 1 ){ local_ioerr(); return A; }
static void local_ioerr(){
  sqlite3_io_error_hit = 1;  /* Really just a place to set a breakpoint */
}
#define SimulateDiskfullError \
   if( sqlite3_diskfull_pending ){ \
     if( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       return SQLITE_FULL; \
     }else{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#else
#define SimulateIOError(A)
#define SimulateDiskfullError
#endif

/*
** When testing, keep a count of the number of open files.
*/
#ifdef SQLITE_TEST
int sqlite3_open_file_count = 0;
#define OpenCounter(X)  sqlite3_open_file_count+=(X)
#else
#define OpenCounter(X)
#endif

/*
** sqlite3GenericMalloc
** sqlite3GenericRealloc
** sqlite3GenericOsFree
** sqlite3GenericAllocationSize
**
** Implementation of the os level dynamic memory allocation interface in terms
** of the standard malloc(), realloc() and free() found in many operating
** systems. No rocket science here.
**
** There are two versions of these four functions here. The version
** implemented here is only used if memory-management or memory-debugging is
** enabled. This version allocates an extra 8-bytes at the beginning of each
** block and stores the size of the allocation there.
**
** If neither memory-management or debugging is enabled, the second
** set of implementations is used instead.
*/
#if defined(SQLITE_ENABLE_MEMORY_MANAGEMENT) || defined (SQLITE_MEMDEBUG)
void *sqlite3GenericMalloc(int n){
  char *p = (char *)malloc(n+8);
  assert(n>0);
  assert(sizeof(int)<=8);
  if( p ){
    *(int *)p = n;
    p += 8;
  }
  return (void *)p;
}
void *sqlite3GenericRealloc(void *p, int n){
  char *p2 = ((char *)p - 8);
  assert(n>0);
  p2 = (char*)realloc(p2, n+8);
  if( p2 ){
    *(int *)p2 = n;
    p2 += 8;
  }
  return (void *)p2;
}
void sqlite3GenericFree(void *p){
  assert(p);
  free((void *)((char *)p - 8));
}
int sqlite3GenericAllocationSize(void *p){
  return p ? *(int *)((char *)p - 8) : 0;
}
#else
void *sqlite3GenericMalloc(int n){
  char *p = (char *)malloc(n);
  return (void *)p;
}
void *sqlite3GenericRealloc(void *p, int n){
  assert(n>0);
  p = realloc(p, n);
  return p;
}
void sqlite3GenericFree(void *p){
  assert(p);
  free(p);
}
/* Never actually used, but needed for the linker */
int sqlite3GenericAllocationSize(void *p){ return 0; }
#endif
/********************************file from keywordhash.h******************************************/
