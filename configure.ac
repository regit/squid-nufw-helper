# autoconf
AC_PREREQ(2.57)
AC_INIT(squid_nufw_helper, 1.1, nufw-devel@nongnu.org)
AM_INIT_AUTOMAKE(1.8)

AC_CONFIG_SRCDIR([main.c])
AC_PROG_CC
AC_SUBST(CC)

AC_ARG_WITH(mysql,   [--with-mysql Use Mysql as backend (PostgreSQL is selected if not Mysql)],db_mysql=$withval, db_mysql="no")
AC_ARG_WITH(squid-group,
   [--with-squid-group=GROUP    Specify UNIX group of Squid process (default: "proxy")],
   [SQUID_GROUP="$withval"],[SQUID_GROUP="proxy"])
AC_SUBST([SQUID_GROUP])

if test "${db_mysql}" = "yes"; then
  AC_CHECK_LIB([mysqlclient],[mysql_real_connect],AC_DEFUN([HAVE_LIBRARY_MYSQL],[1]),AC_MSG_ERROR([mysqlclient library needed if selected]),[$(mysql_config --libs)])
else
  AC_CHECK_LIB([pq],[PQconnectdb],AC_DEFUN([HAVE_LIBRARY_PGSQL],[1]),AC_MSG_ERROR([libpq library needed if PostgreSQL support selected (ie, if mysql is not selected)]),[$(pg_config --libs)])
fi
if test "${db_mysql}" = "yes"; then
    AC_DEFINE_UNQUOTED([DB_TYPE_MYSQL],[1],[Use MySQL database])
fi

if test "${db_mysql}" = "yes"; then
 CFLAGS="$(mysql_config --cflags) $CFLAGS"
else
 CFLAGS="-I/usr/include/postgresql $CFLAGS"
fi


CFLAGS="-Wall $CFLAGS"

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strdup strerror)

AM_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([Makefile])

AC_OUTPUT

