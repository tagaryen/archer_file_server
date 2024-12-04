#ifndef _LMDB_H_
#define _LMDB_H_

#include <sys/types.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
typedef	int	mdb_mode_t;
#else
typedef	mode_t	mdb_mode_t;
#endif

#ifdef _WIN32
typedef	void *mdb_filehandle_t;
#else
typedef int mdb_filehandle_t;
#endif

/** Library major version */
#define MDB_VERSION_MAJOR	0
/** Library minor version */
#define MDB_VERSION_MINOR	9
/** Library patch version */
#define MDB_VERSION_PATCH	31

/** Combine args a,b,c into a single integer for easy version comparisons */
#define MDB_VERINT(a,b,c)	(((a) << 24) | ((b) << 16) | (c))

/** The full library version as a single integer */
#define MDB_VERSION_FULL	\
	MDB_VERINT(MDB_VERSION_MAJOR,MDB_VERSION_MINOR,MDB_VERSION_PATCH)

/** The release date of this library version */
#define MDB_VERSION_DATE	"July 10, 2023"

/** A stringifier for the version info */
#define MDB_VERSTR(a,b,c,d)	"LMDB " #a "." #b "." #c ": (" d ")"

/** A helper for the stringifier macro */
#define MDB_VERFOO(a,b,c,d)	MDB_VERSTR(a,b,c,d)

/** The full library version as a C string */
#define	MDB_VERSION_STRING	\
	MDB_VERFOO(MDB_VERSION_MAJOR,MDB_VERSION_MINOR,MDB_VERSION_PATCH,MDB_VERSION_DATE)
/**	@} */

/** @brief Opaque structure for a database environment.
 *
 * A DB environment supports multiple databases, all residing in the same
 * shared-memory map.
 */
typedef struct MDB_env MDB_env;

/** @brief Opaque structure for a transaction handle.
 *
 * All database operations require a transaction handle. Transactions may be
 * read-only or read-write.
 */
typedef struct MDB_txn MDB_txn;

/** @brief A handle for an individual database in the DB environment. */
typedef unsigned int	MDB_dbi;

/** @brief Opaque structure for navigating through a database */
typedef struct MDB_cursor MDB_cursor;

typedef struct MDB_val {
	size_t		 mv_size;	/**< size of the data item */
	void		*mv_data;	/**< address of the data item */
} MDB_val;

/** @brief A callback function used to compare two keys in a database */
typedef int  (MDB_cmp_func)(const MDB_val *a, const MDB_val *b);

typedef void (MDB_rel_func)(MDB_val *item, void *oldptr, void *newptr, void *relctx);

/** @defgroup	mdb_env	Environment Flags
 *	@{
 */
	/** mmap at a fixed address (experimental) */
#define MDB_FIXEDMAP	0x01
	/** no environment directory */
#define MDB_NOSUBDIR	0x4000
	/** don't fsync after commit */
#define MDB_NOSYNC		0x10000
	/** read only */
#define MDB_RDONLY		0x20000
	/** don't fsync metapage after commit */
#define MDB_NOMETASYNC		0x40000
	/** use writable mmap */
#define MDB_WRITEMAP		0x80000
	/** use asynchronous msync when #MDB_WRITEMAP is used */
#define MDB_MAPASYNC		0x100000
	/** tie reader locktable slots to #MDB_txn objects instead of to threads */
#define MDB_NOTLS		0x200000
	/** don't do any locking, caller must manage their own locks */
#define MDB_NOLOCK		0x400000
	/** don't do readahead (no effect on Windows) */
#define MDB_NORDAHEAD	0x800000
	/** don't initialize malloc'd memory before writing to datafile */
#define MDB_NOMEMINIT	0x1000000
/** @} */

/**	@defgroup	mdb_dbi_open	Database Flags
 *	@{
 */
	/** use reverse string keys */
#define MDB_REVERSEKEY	0x02
	/** use sorted duplicates */
#define MDB_DUPSORT		0x04
	/** numeric keys in native byte order: either unsigned int or size_t.
	 *  The keys must all be of the same size. */
#define MDB_INTEGERKEY	0x08
	/** with #MDB_DUPSORT, sorted dup items have fixed size */
#define MDB_DUPFIXED	0x10
	/** with #MDB_DUPSORT, dups are #MDB_INTEGERKEY-style integers */
#define MDB_INTEGERDUP	0x20
	/** with #MDB_DUPSORT, use reverse string dups */
#define MDB_REVERSEDUP	0x40
	/** create DB if not already existing */
#define MDB_CREATE		0x40000
/** @} */

/**	@defgroup mdb_put	Write Flags
 *	@{
 */
/** For put: Don't write if the key already exists. */
#define MDB_NOOVERWRITE	0x10
/** Only for #MDB_DUPSORT<br>
 * For put: don't write if the key and data pair already exist.<br>
 * For mdb_cursor_del: remove all duplicate data items.
 */
#define MDB_NODUPDATA	0x20
/** For mdb_cursor_put: overwrite the current key/data pair */
#define MDB_CURRENT	0x40
/** For put: Just reserve space for data, don't copy it. Return a
 * pointer to the reserved space.
 */
#define MDB_RESERVE	0x10000
/** Data is being appended, don't split full pages. */
#define MDB_APPEND	0x20000
/** Duplicate data is being appended, don't split full pages. */
#define MDB_APPENDDUP	0x40000
/** Store multiple data items in one call. Only for #MDB_DUPFIXED. */
#define MDB_MULTIPLE	0x80000
/*	@} */

/**	@defgroup mdb_copy	Copy Flags
 *	@{
 */
/** Compacting copy: Omit free space from copy, and renumber all
 * pages sequentially.
 */
#define MDB_CP_COMPACT	0x01
/*	@} */

/** @brief Cursor Get operations.
 *
 *	This is the set of all operations for retrieving data
 *	using a cursor.
 */
typedef enum MDB_cursor_op {
	MDB_FIRST,				/**< Position at first key/data item */
	MDB_FIRST_DUP,			/**< Position at first data item of current key.
								Only for #MDB_DUPSORT */
	MDB_GET_BOTH,			/**< Position at key/data pair. Only for #MDB_DUPSORT */
	MDB_GET_BOTH_RANGE,		/**< position at key, nearest data. Only for #MDB_DUPSORT */
	MDB_GET_CURRENT,		/**< Return key/data at current cursor position */
	MDB_GET_MULTIPLE,		/**< Return up to a page of duplicate data items
								from current cursor position. Move cursor to prepare
								for #MDB_NEXT_MULTIPLE. Only for #MDB_DUPFIXED */
	MDB_LAST,				/**< Position at last key/data item */
	MDB_LAST_DUP,			/**< Position at last data item of current key.
								Only for #MDB_DUPSORT */
	MDB_NEXT,				/**< Position at next data item */
	MDB_NEXT_DUP,			/**< Position at next data item of current key.
								Only for #MDB_DUPSORT */
	MDB_NEXT_MULTIPLE,		/**< Return up to a page of duplicate data items
								from next cursor position. Move cursor to prepare
								for #MDB_NEXT_MULTIPLE. Only for #MDB_DUPFIXED */
	MDB_NEXT_NODUP,			/**< Position at first data item of next key */
	MDB_PREV,				/**< Position at previous data item */
	MDB_PREV_DUP,			/**< Position at previous data item of current key.
								Only for #MDB_DUPSORT */
	MDB_PREV_NODUP,			/**< Position at last data item of previous key */
	MDB_SET,				/**< Position at specified key */
	MDB_SET_KEY,			/**< Position at specified key, return key + data */
	MDB_SET_RANGE,			/**< Position at first key greater than or equal to specified key. */
	MDB_PREV_MULTIPLE		/**< Position at previous page and return up to
								a page of duplicate data items. Only for #MDB_DUPFIXED */
} MDB_cursor_op;

/** @defgroup  errors	Return Codes
 *
 *	BerkeleyDB uses -30800 to -30999, we'll go under them
 *	@{
 */
	/**	Successful result */
#define MDB_SUCCESS	 0
	/** key/data pair already exists */
#define MDB_KEYEXIST	(-30799)
	/** key/data pair not found (EOF) */
#define MDB_NOTFOUND	(-30798)
	/** Requested page not found - this usually indicates corruption */
#define MDB_PAGE_NOTFOUND	(-30797)
	/** Located page was wrong type */
#define MDB_CORRUPTED	(-30796)
	/** Update of meta page failed or environment had fatal error */
#define MDB_PANIC		(-30795)
	/** Environment version mismatch */
#define MDB_VERSION_MISMATCH	(-30794)
	/** File is not a valid LMDB file */
#define MDB_INVALID	(-30793)
	/** Environment mapsize reached */
#define MDB_MAP_FULL	(-30792)
	/** Environment maxdbs reached */
#define MDB_DBS_FULL	(-30791)
	/** Environment maxreaders reached */
#define MDB_READERS_FULL	(-30790)
	/** Too many TLS keys in use - Windows only */
#define MDB_TLS_FULL	(-30789)
	/** Txn has too many dirty pages */
#define MDB_TXN_FULL	(-30788)
	/** Cursor stack too deep - internal error */
#define MDB_CURSOR_FULL	(-30787)
	/** Page has not enough space - internal error */
#define MDB_PAGE_FULL	(-30786)
	/** Database contents grew beyond environment mapsize */
#define MDB_MAP_RESIZED	(-30785)
	/** Operation and DB incompatible, or DB type changed. This can mean:
	 *	<ul>
	 *	<li>The operation expects an #MDB_DUPSORT / #MDB_DUPFIXED database.
	 *	<li>Opening a named DB when the unnamed DB has #MDB_DUPSORT / #MDB_INTEGERKEY.
	 *	<li>Accessing a data record as a database, or vice versa.
	 *	<li>The database was dropped and recreated with different flags.
	 *	</ul>
	 */
#define MDB_INCOMPATIBLE	(-30784)
	/** Invalid reuse of reader locktable slot */
#define MDB_BAD_RSLOT		(-30783)
	/** Transaction must abort, has a child, or is invalid */
#define MDB_BAD_TXN			(-30782)
	/** Unsupported size of key/DB name/data, or wrong DUPFIXED size */
#define MDB_BAD_VALSIZE		(-30781)
	/** The specified DBI was changed unexpectedly */
#define MDB_BAD_DBI		(-30780)
	/** The last defined error code */
#define MDB_LAST_ERRCODE	MDB_BAD_DBI
/** @} */

/** @brief Statistics for a database in the environment */
typedef struct MDB_stat {
	unsigned int	ms_psize;			/**< Size of a database page.
											This is currently the same for all databases. */
	unsigned int	ms_depth;			/**< Depth (height) of the B-tree */
	size_t		ms_branch_pages;	/**< Number of internal (non-leaf) pages */
	size_t		ms_leaf_pages;		/**< Number of leaf pages */
	size_t		ms_overflow_pages;	/**< Number of overflow pages */
	size_t		ms_entries;			/**< Number of data items */
} MDB_stat;

/** @brief Information about the environment */
typedef struct MDB_envinfo {
	void	*me_mapaddr;			/**< Address of map, if fixed */
	size_t	me_mapsize;				/**< Size of the data memory map */
	size_t	me_last_pgno;			/**< ID of the last used page */
	size_t	me_last_txnid;			/**< ID of the last committed transaction */
	unsigned int me_maxreaders;		/**< max reader slots in the environment */
	unsigned int me_numreaders;		/**< max reader slots used in the environment */
} MDB_envinfo;

char *mdb_version(int *major, int *minor, int *patch);

char *mdb_strerror(int err);

int  mdb_env_create(MDB_env **env);

int  mdb_env_open(MDB_env *env, const char *path, unsigned int flags, mdb_mode_t mode);

int  mdb_env_copy(MDB_env *env, const char *path);

int  mdb_env_copyfd(MDB_env *env, mdb_filehandle_t fd);

int  mdb_env_copy2(MDB_env *env, const char *path, unsigned int flags);

int  mdb_env_copyfd2(MDB_env *env, mdb_filehandle_t fd, unsigned int flags);

int  mdb_env_stat(MDB_env *env, MDB_stat *stat);

int  mdb_env_info(MDB_env *env, MDB_envinfo *stat);

int  mdb_env_sync(MDB_env *env, int force);

void mdb_env_close(MDB_env *env);

int  mdb_env_set_flags(MDB_env *env, unsigned int flags, int onoff);

int  mdb_env_get_flags(MDB_env *env, unsigned int *flags);

int  mdb_env_get_path(MDB_env *env, const char **path);

int  mdb_env_get_fd(MDB_env *env, mdb_filehandle_t *fd);

int  mdb_env_set_mapsize(MDB_env *env, size_t size);

int  mdb_env_set_maxreaders(MDB_env *env, unsigned int readers);

int  mdb_env_get_maxreaders(MDB_env *env, unsigned int *readers);

int  mdb_env_set_maxdbs(MDB_env *env, MDB_dbi dbs);

int  mdb_env_get_maxkeysize(MDB_env *env);

int  mdb_env_set_userctx(MDB_env *env, void *ctx);

void *mdb_env_get_userctx(MDB_env *env);

typedef void MDB_assert_func(MDB_env *env, const char *msg);

int  mdb_env_set_assert(MDB_env *env, MDB_assert_func *func);

int  mdb_txn_begin(MDB_env *env, MDB_txn *parent, unsigned int flags, MDB_txn **txn);

MDB_env *mdb_txn_env(MDB_txn *txn);

size_t mdb_txn_id(MDB_txn *txn);

int  mdb_txn_commit(MDB_txn *txn);

void mdb_txn_abort(MDB_txn *txn);

void mdb_txn_reset(MDB_txn *txn);

int  mdb_txn_renew(MDB_txn *txn);

/** Compat with version <= 0.9.4, avoid clash with libmdb from MDB Tools project */
#define mdb_open(txn,name,flags,dbi)	mdb_dbi_open(txn,name,flags,dbi)
/** Compat with version <= 0.9.4, avoid clash with libmdb from MDB Tools project */
#define mdb_close(env,dbi)				mdb_dbi_close(env,dbi)

int  mdb_dbi_open(MDB_txn *txn, const char *name, unsigned int flags, MDB_dbi *dbi);

int  mdb_stat(MDB_txn *txn, MDB_dbi dbi, MDB_stat *stat);

int mdb_dbi_flags(MDB_txn *txn, MDB_dbi dbi, unsigned int *flags);

void mdb_dbi_close(MDB_env *env, MDB_dbi dbi);

int  mdb_drop(MDB_txn *txn, MDB_dbi dbi, int del);

int  mdb_set_compare(MDB_txn *txn, MDB_dbi dbi, MDB_cmp_func *cmp);

int  mdb_set_dupsort(MDB_txn *txn, MDB_dbi dbi, MDB_cmp_func *cmp);

int  mdb_set_relfunc(MDB_txn *txn, MDB_dbi dbi, MDB_rel_func *rel);

int  mdb_set_relctx(MDB_txn *txn, MDB_dbi dbi, void *ctx);

int  mdb_get(MDB_txn *txn, MDB_dbi dbi, MDB_val *key, MDB_val *data);

int  mdb_put(MDB_txn *txn, MDB_dbi dbi, MDB_val *key, MDB_val *data,
			    unsigned int flags);

int  mdb_del(MDB_txn *txn, MDB_dbi dbi, MDB_val *key, MDB_val *data);

int  mdb_cursor_open(MDB_txn *txn, MDB_dbi dbi, MDB_cursor **cursor);

void mdb_cursor_close(MDB_cursor *cursor);

int  mdb_cursor_renew(MDB_txn *txn, MDB_cursor *cursor);

MDB_txn *mdb_cursor_txn(MDB_cursor *cursor);

MDB_dbi mdb_cursor_dbi(MDB_cursor *cursor);

int  mdb_cursor_get(MDB_cursor *cursor, MDB_val *key, MDB_val *data,
			    MDB_cursor_op op);

int  mdb_cursor_put(MDB_cursor *cursor, MDB_val *key, MDB_val *data,
				unsigned int flags);

int  mdb_cursor_del(MDB_cursor *cursor, unsigned int flags);

int  mdb_cursor_count(MDB_cursor *cursor, size_t *countp);

int  mdb_cmp(MDB_txn *txn, MDB_dbi dbi, const MDB_val *a, const MDB_val *b);

int  mdb_dcmp(MDB_txn *txn, MDB_dbi dbi, const MDB_val *a, const MDB_val *b);

typedef int (MDB_msg_func)(const char *msg, void *ctx);

int	mdb_reader_list(MDB_env *env, MDB_msg_func *func, void *ctx);

int	mdb_reader_check(MDB_env *env, int *dead);

#ifdef __cplusplus
}
#endif

#endif /* _LMDB_H_ */

