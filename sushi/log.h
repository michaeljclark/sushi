//
//  log.h
//

#ifndef log_h
#define log_h

/* logging */

std::string format_string(const char* fmt, ...);
void log_prefix(const char* prefix, const char* fmt, va_list arg);
void log_fatal_exit(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_debug(const char* fmt, ...);

#endif
