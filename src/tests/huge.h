
#ifndef libcfx2_huge_h_included
#define libcfx2_huge_h_included

#ifndef tests_DEBUG
#define huge_filename   "huge.cfx2"
#define huge_node_count (2000 * 1000)
#else
/* debug mode is (particularly on Windows) AWFULLY slow */

#define huge_filename   "huge_Debug.cfx2"
#define huge_node_count (8 * 1000)
#endif

#endif
