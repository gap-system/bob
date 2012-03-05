#include <bob.h>

static const char *GAP_dependencies
  = { NULL };

static BOBstatus GAP_prerequisites()
{
    return BOB_OK;
}

static BOBstatus GAP_get(string targetdir)
{
    return BOB_getindirectly(targetdir,
           "http://www-groups.mcs.st-and.ac.uk/~neunhoef/for/BOB/GAP.link");
}

static BOBstatus GAP_build(string targetdir)
{
    return BOB_OK;
}

Component GAP(1,GAP_dependencies,GAP_prerequisites,GAP_get,GAP_build);

