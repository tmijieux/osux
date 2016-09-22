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
	tr_sort.c \
	tr_db.c \
	tr_gts.c \
	config.c \
	options.c \
	cst_yaml.c \
	vector.c \
	linear_fun.c \
	spacing_count.c \
	freq_counter.c \
	compute_stars.c \
	density.c \
	reading.c \
	pattern.c \
	accuracy.c \
	final_star.c

taiko_ranking_LDADD = ../lib/libosux.la

taiko_ranking_CFLAGS = \
	$(MYSQL_CFLAGS) -DUSE_TR_MYSQL_DB \
	-fopenmp $(GTS_CFLAGS) $(AM_CFLAGS)
	-rpath $(pkglibdir)

taiko_ranking_LDFLAGS = \
	$(MYSQL_LIBS) -lm \
	$(GTS_LIBS) $(AM_LDFLAGS)
