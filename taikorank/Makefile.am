include $(top_srcdir)/common.mk

SUBDIRS = yaml

bin_PROGRAMS = taiko_ranking
taiko_ranking_SOURCES = \
	main.c \
	check_osu_file.c \
	tr_mods.c \
	print.c \
	taiko_ranking_map.c \
	taiko_ranking_object.c \
	taiko_ranking_score.c \
	treatment.c \
	stats.c \
	tr_db.c \
	config.c \
	options.c \
	cst_yaml.c \
	vector.c \
	linear_fun.c \
	spacing_count.c \
	freq_counter.c \
	density.c \
	reading.c \
	pattern.c \
	accuracy.c \
	final_star.c

taiko_ranking_LDADD = ../libosux.la \
		../util/libosux_util.la

taiko_ranking_CFLAGS = -DMYSQL_TR_DB -fopenmp -O3 $(AM_CFLAGS)
taiko_ranking_LDFLAGS = -lmysqlclient -pthread -lm $(AM_LDFLAGS)

