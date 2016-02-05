/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>

static int app_dir = -1;
void set_app_dir(const char * argv0)
{
  char * tmp = strdup(argv0);
  tmp = dirname(tmp);
  
  int (* open_fun)(const char *, int, int) = NULL;
  open_fun = dlsym(RTLD_NEXT, "open");
  if(open_fun != NULL)
    app_dir = open_fun(tmp, O_DIRECTORY);
  
  free(tmp);
}

int open(const char *pathname, int flags, ...);
{
  int r = openat(app_dir, pathname, flags, mods);
  if(r < 0)
    {
      int (* open_fun)(const char *, int, int) = NULL;
      open_fun = dlsym(RTLD_NEXT, "open");
      if(open_fun != NULL)
	r = open_fun(pathname, flags, mode);
      else 
	r = -1;
    }
  return r;
}
