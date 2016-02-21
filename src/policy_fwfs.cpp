/*
  Copyright (c) 2016, Antonio SJ Musumeci <trapexit@spawn.link>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <errno.h>
#include <sys/statvfs.h>

#include <string>
#include <vector>

#include "fs.hpp"
#include "fs_path.hpp"
#include "policy.hpp"
#include "statvfs_util.hpp"

using std::string;
using std::vector;
using std::size_t;
using mergerfs::Policy;
using mergerfs::Category;

static
int
_fwfs(const vector<string>  &basepaths,
      const char            *fusepath,
      const size_t           minfreespace,
      const bool             needswritablefs,
      vector<const string*> &paths)
{
  string fullpath;
  struct statvfs st;

  for(size_t i = 0, size = basepaths.size(); i != size; i++)
    {
      const string *basepath = &basepaths[i];

      fs::path::make(basepath,fusepath,fullpath);

      if(!fs::available(fullpath,needswritablefs,st))
        continue;

      if(StatVFS::spaceavail(st) < minfreespace)
        continue;

      paths.push_back(basepath);

      return POLICY_SUCCESS;
    }

  return (errno=ENOENT,POLICY_FAIL);
}

namespace mergerfs
{
  int
  Policy::Func::fwfs(const Category::Enum::Type  type,
                     const vector<string>       &basepaths,
                     const char                 *fusepath,
                     const size_t                minfreespace,
                     vector<const string*>      &paths)
  {
    int rv;
    const char *fp =
      ((type == Category::Enum::create) ? "" : fusepath);
    const bool needswritablefs =
      (type == Category::Enum::create);

    rv = _fwfs(basepaths,fp,minfreespace,needswritablefs,paths);
    if(POLICY_FAILED(rv))
      rv = Policy::Func::mfs(type,basepaths,fusepath,minfreespace,paths);

    return rv;
  }
}
