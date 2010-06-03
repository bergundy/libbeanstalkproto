TESTS = commands.t responses.t stats.t
check_PROGRAMS = commands.t responses.t stats.t

srcdir = $(top_builddir)/src
AM_CFLAGS = -I$(srcdir) 

commands_t_SOURCES = test_commands.c beanstalkclient.h
commands_t_CFLAGS  = @CHECK_CFLAGS@ $(AM_CFLAGS)
commands_t_LDADD   =  @CHECK_LIBS@ $(srcdir)/beanstalkclient.o

responses_t_SOURCES = test_responses.c beanstalkclient.h
responses_t_CFLAGS  = @CHECK_CFLAGS@ $(AM_CFLAGS)
responses_t_LDADD   =  @CHECK_LIBS@ $(srcdir)/beanstalkclient.o

stats_t_SOURCES = test_stats.c beanstalkclient.h
stats_t_CFLAGS  = @CHECK_CFLAGS@ $(AM_CFLAGS)
stats_t_LDADD   =  @CHECK_LIBS@ $(srcdir)/beanstalkclient.o
