include $(top_srcdir)/common.mk

SUBDIRS = compute_star yaml

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
	spacing_count.c

taiko_ranking_LDADD = ../../libosux.la \
		../../util/libosux_util.la \
		compute_star/libtr_star.la

taiko_ranking_CFLAGS = -I $(srcdir)/compute_star $(AM_CFLAGS)
taiko_ranking_LDFLAGS = -pthread -lmysqlclient -lm $(AM_LDFLAGS)

