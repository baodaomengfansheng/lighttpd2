#ifndef _LIGHTTPD_LOG_H_
#define _LIGHTTPD_LOG_H_

#ifndef _LIGHTTPD_BASE_H_
#error Please include <lighttpd/base.h> instead of this file
#endif

/* #include <lighttpd/valgrind/valgrind.h> */

#define REMOVE_PATH_FROM_FILE 1
#if REMOVE_PATH_FROM_FILE
LI_API const char *remove_path(const char *path);
#define REMOVE_PATH(file) remove_path(file)
#else
#define REMOVE_PATH(file) file
#endif

#define VR_ERROR(vr, fmt, ...) CON_ERROR(vr->con, fmt, __VA_ARGS__)
#define VR_TRACE(vr, fmt, ...) CON_TRACE(vr->con, fmt, __VA_ARGS__)
#define VR_SEGFAULT(vr, fmt, ...) CON_SEGFAULT(vr->con, fmt, __VA_ARGS__)

#define ERROR(srv, fmt, ...) \
	log_write_(srv, NULL, LOG_LEVEL_ERROR, LOG_FLAG_TIMETAMP, "(error) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define WARNING(srv, fmt, ...) \
	log_write_(srv, NULL, LOG_LEVEL_WARNING, LOG_FLAG_TIMETAMP, "(warning) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define INFO(srv, fmt, ...) \
	log_write_(srv, NULL, LOG_LEVEL_INFO, LOG_FLAG_TIMETAMP, "(info) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define TRACE(srv, fmt, ...) \
	log_write_(srv, NULL, LOG_LEVEL_INFO, LOG_FLAG_TIMETAMP, "(debug) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)


#define CON_ERROR(con, fmt, ...) \
	log_write_(con->srv, con, LOG_LEVEL_ERROR, LOG_FLAG_TIMETAMP, "(error) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define CON_WARNING(con, fmt, ...) \
	log_write_(con->srv, con, LOG_LEVEL_WARNING, LOG_FLAG_TIMETAMP, "(warning) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define CON_INFO(con, fmt, ...) \
	log_write_(con->srv, con, LOG_LEVEL_INFO, LOG_FLAG_TIMETAMP, "(info) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define CON_TRACE(con, fmt, ...) \
	log_write_(con->srv, con, LOG_LEVEL_DEBUG, LOG_FLAG_TIMETAMP, "(debug) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)


/* TODO: perhaps make portable (detect if cc supports) */
#define	__ATTRIBUTE_PRINTF_FORMAT(fmt, arg) __attribute__ ((__format__ (__printf__, fmt, arg)))

/*LI_API int log_write(server *srv, connection *con, const char *fmt, ...) __ATTRIBUTE_PRINTF_FORMAT(3, 4);*/



/* convenience makros */
#define log_error(srv, con, fmt, ...) \
	log_write_(srv, con, LOG_LEVEL_ERROR, LOG_FLAG_TIMETAMP, "(error) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define log_warning(srv, con, fmt, ...) \
	log_write_(srv, con, LOG_LEVEL_WARNING, LOG_FLAG_TIMETAMP, "(warning) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define log_info(srv, con, fmt, ...) \
	log_write_(srv, con, LOG_LEVEL_INFO, LOG_FLAG_TIMETAMP, "(info) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define log_message(srv, con, fmt, ...) \
	log_write_(srv, con, LOG_LEVEL_MESSAGE, LOG_FLAG_TIMETAMP, "(message) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)

#define log_debug(srv, con, fmt, ...) \
	log_write_(srv, con, LOG_LEVEL_DEBUG, LOG_FLAG_TIMETAMP, "(debug) %s.%d: "fmt, REMOVE_PATH(__FILE__), __LINE__, __VA_ARGS__)



struct log_t;
typedef struct log_t log_t;

struct log_entry_t;
typedef struct log_entry_t log_entry_t;

struct log_timestamp_t;
typedef struct log_timestamp_t log_timestamp_t;

typedef enum {
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_ABORT
} log_level_t;

typedef enum {
	LOG_TYPE_STDERR,
	LOG_TYPE_FILE,
	LOG_TYPE_PIPE,
	LOG_TYPE_SYSLOG,
	LOG_TYPE_NONE
} log_type_t;

/* flags for log_write */
#define LOG_FLAG_NONE         (0x0)      /* default flag */
#define LOG_FLAG_TIMETAMP     (0x1)      /* prepend a timestamp to the log message */
#define LOG_FLAG_NOLOCK       (0x1 << 1) /* for internal use only */
#define LOG_FLAG_ALLOW_REPEAT (0x1 << 2) /* allow writing of multiple equal entries after each other */

struct log_t {
	log_type_t type;
	GString *path;
	gint refcount;
	gint fd;

	GString *lastmsg;
	guint lastmsg_count;

	GMutex *mutex;
};

struct log_timestamp_t {
	gint refcount;
	ev_tstamp last_ts;
	GString *format;
	GString *cached;
};

struct log_entry_t {
	log_t *log;
	log_level_t level;
	GString *msg;
};

/* determines the type of a log target by the path given. /absolute/path = file; |app = pipe; stderr = stderr; syslog = syslog */
log_type_t log_type_from_path(GString *path);

log_level_t log_level_from_string(GString *str);
gchar* log_level_str(log_level_t log_level);

/* log_new is used to create a new log target, if a log with the same path already exists, it is referenced instead */
log_t *log_new(server *srv, log_type_t type, GString *path);
/* avoid calling log_free directly. instead use log_unref which calls log_free if refcount has reached zero */
void log_free(server *srv, log_t *log);
void log_free_unlocked(server *srv, log_t *log);

void log_ref(server *srv, log_t *log);
void log_unref(server *srv, log_t *log);

void log_lock(log_t *log);
void log_unlock(log_t *log);

/* do not call directly, use log_rotate_logs instead */
void log_rotate(gchar *path, log_t *log, server *srv);

void log_rotate_logs(server *srv);

gpointer log_thread(server *srv);
void log_thread_start(server *srv);
void log_thread_stop(server *srv);
void log_thread_finish(server *srv);
void log_thread_wakeup(server *srv);

void log_init(server *srv);
void log_cleanup(server *srv);

/* log_write is used to directly write a message to a log target */
LI_API void log_write(server *srv, log_t *log, GString *msg);
/* log_write_ is used to write to the errorlog */
LI_API gboolean log_write_(server *srv, connection *con, log_level_t log_level, guint flags, const gchar *fmt, ...) __ATTRIBUTE_PRINTF_FORMAT(5, 6);

log_timestamp_t *log_timestamp_new(server *srv, GString *format);
gboolean log_timestamp_free(server *srv, log_timestamp_t *ts);

#endif
