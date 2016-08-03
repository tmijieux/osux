#ifndef OSUX_H
#define OSUX_H

#include "./osux/timingpoint.h"
#include "./osux/keys.h"
#include "./osux/replay.h"
#include "./osux/mods.h"
#include "./osux/beatmap.h"
#include "./osux/database.h"
#include "./osux/beatmap_database.h"
#include "./osux/error.h"
#include "./osux/string2.h"
#include "./osux/read.h"
#include "./osux/cfetch.h"
#include "./osux/beatmap_set.h"
#include "./osux/data.h"
#include "./osux/hash_table.h"
#include "./osux/pyfetch.h"
#include "./osux/cparse.h"
#include "./osux/list.h"
#include "./osux/split.h"
#include "./osux/uleb128.h"
#include "./osux/list_node.h"
#include "./osux/stack.h"
#include "./osux/heap.h"
#include "./osux/taiko_autoconvert.h"
#include "./osux/yaml2.h"
#include "./osux/xz_decomp.h"
#include "./osux/sum.h"
#include "./osux/md5.h"
#include "./osux/util.h"
#include "./osux/hitsound.h"
#include "./osux/color.h"
#include "./osux/hitobject.h"
#include "./osux/parser.h"
#include "./osux/game_mode.h"
#include "./osux/table.h"
#include "./osux/compiler.h"

#ifdef _WIN32
# include "./osux/windows.h"
#else
# include <unistd.h>
#endif

#endif //OSUX_H
