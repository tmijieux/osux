include $(top_srcdir)/common.mk

SUBDIRS = yaml

bin_PROGRAMS = taiko_ranking
taiko_ranking_SOURCES = \
	main.c \
	check_osu_file.c check_osu_file.h \
	tr_mods.c tr_mods.h \
	print.c print.h \
	taiko_ranking_map.c taiko_ranking_map.h \
	taiko_ranking_object.c taiko_ranking_object.h \
	taiko_ranking_score.c taiko_ranking_score.h \
	treatment.c treatment.h \
	tr_sort.c tr_sort.h \
	tr_db.c tr_db.h \
	tr_gts.c tr_gts.h \
	config.c config.h \
	options.c options.h \
	cst_yaml.c cst_yaml.h \
	vector.c vector.h \
	linear_fun.c linear_fun.h\
	spacing_count.c spacing_count.h \
	freq_counter.c freq_counter.h \
	compute_stars.c compute_stars.h \
	density.c density.h \
	reading.c reading.h \
	pattern.c pattern.h \
	accuracy.c accuracy.h \
	final_star.c final_star.h

taiko_ranking_LDADD = ../lib/libosux.la

taiko_ranking_CFLAGS = \
	$(MYSQL_CFLAGS) -DMYSQL_TR_DB \
	-fopenmp $(GTS_CFLAGS) $(AM_CFLAGS)
	-rpath $(pkglibdir)

taiko_ranking_LDFLAGS = \
	$(MYSQL_LIBS) -lm \
	$(GTS_LIBS) $(AM_LDFLAGS)
