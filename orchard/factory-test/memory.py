#!/usr/bin/python
import sys

err = open(sys.argv[1]).read()
m1 = int(sys.argv[2], 16)
m2 = int(sys.argv[3])

# err = """
# ASSERT( mem_322[0x0000000C] = 0x80 );
# ASSERT( mem_322[0xFFFFFFFD] = 0x0C );
# ASSERT( mem_322[0x0000000B] = 0x0B );
# ASSERT( mem_322[0xFFFFFFFA] = 0x62 );
# ASSERT( mem_322[0xFFFFFFFE] = 0xB5 );
# ASSERT( mem_322[0xFFFFFFFB] = 0x0B );
# ASSERT( mem_322[0xFFFFFFF9] = 0xA9 );
# ASSERT( mem_322[0x00000009] = 0xC4 );
# ASSERT( mem_322[0x0000000A] = 0x70 );
# ASSERT( mem_322[0xFFFFFFFF] = 0x00 );
# ASSERT( mem_322[0x00000000] = 0x00 );
# ASSERT( mem_322[0xFFFFFFFC] = 0x80 );
# ASSERT( R_EBP_0 = 0x00000001 );
# """

ls = err.split("\n")
ls = [l[len("ASSERT(") + 1: -2] for l in ls]
ls = [l for l in ls if l != '']

ls_r = [l.split() for l in ls if l[:3] != "mem"]
ls_r = [(l[0], int(l[2], 0)) for l in ls_r]
ls_m = [l for l in ls if l[:3] == "mem"]
ls_m = [l.split() for l in ls_m]
ls_m = [(int(l[0][-11:-1], 0), l[2]) for l in ls_m]
ls_m = [(x - (1 << 32 if x > 0x7fffffff else 0), y) for (x,y) in ls_m]
ls_m = dict(ls_m)
pos = ls_m.keys()
pos.sort()

for (r,v) in ls_r:
    print r,v,hex(v)
print "================"
c = 0

#min(pos), max(pos) + 1

for p in range(m1, m1+m2):
    for (r,v) in ls_r:
        if (v == p):
            print "<---- " , r
    v = ls_m[p] if ls_m.has_key(p) else "_"
    i = int(ls_m[p], 0) + \
        (int(ls_m[p+1], 0)<<8) + \
        (int(ls_m[p+2], 0)<<16) + \
        (int(ls_m[p+3], 0)<<24) \
        if ls_m.has_key(p) and \
        ls_m.has_key(p+1) and \
        ls_m.has_key(p+2) and \
        ls_m.has_key(p+3) and \
        c % 4 == 0 else ""
    h = hex(i) if i <> "" else ""
    m = "mem[%s:u32, e_little]:u32 == %s:u32" % (hex(p), h) if h <> "" else ""
    print hex(p), v, i, h, m
    c+=1

print err.split("\n")[-2]
