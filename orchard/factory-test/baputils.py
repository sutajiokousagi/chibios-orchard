#!/usr/bin/python
import sys
import os.path, time

def process_include(il):
    pos = il.find("\n#include")
    if pos < 0:
        return (il, set())
    pos1 = il.find('"', pos) + 1
    pos2 = il.find('"', pos1)

    f_name = il[pos1:pos2]
    inc = open(f_name).read()
    il = il[:pos] + inc + il[pos2+1:]
    (il, deps) = process_include(il)
    return (il, set([f_name]).union(deps))

def process_comment(il):
    il = il.split("\n")
    il = [l for l in il if l.strip()[:2] != "//"]
    return "\n".join(il)

def process_vars(il):
    vars = {}
    while True:
        pos = il.find("#set")
        if pos < 0:
            break
        pos1 = il.find('=', pos)
        pos2 = il.find('[[', pos1) + 2
        pos3 = il.find(']]', pos2)
        vname = il[pos+4:pos1].strip()
        vvalue = il[pos2:pos3].strip()
        vars[vname] = vvalue
        il = il[:pos] + il[pos3+2:]
        
    return (il, vars)

def process_asserts(il):
    while True:
        pos = il.find("#assert_stop")
        if pos < 0:
            break
        pos1 = il.find('[[', pos) + 2
        pos2 = il.find(']]', pos1)
        aaddr = il[pos+len("#assert_stop"):pos1-2].strip()
        aexp = il[pos1:pos2].strip()
        il = il[:pos] + il[pos2+2:]
        pos = il.find("label pc_%s" % aaddr)
        pos = il.find("jmp", pos)
        pos = il[:pos].rfind("\n")
        il = il[:pos] + '\ntmp_cnd:bool = (%s)\ncjmp ~(tmp_cnd), "lift_error", "success"\n'%aexp +il[pos:]
        
    return il

def process_ind_jmp(il):
    while True:
        pos = il.find("#ind_jmp")
        if pos < 0:
            break
        pos1 = il.find('[[', pos) + 2
        pos2 = il.find(']]', pos1)
        pos3 = il.find('\n', pos2)
        addr = il[pos+len("#ind_jmp"):pos1-2].strip()
        exp = il[pos1:pos2].strip()
        dest = il[pos2+2:pos3].strip()
        il = il[:pos] + il[pos3:]

        pos_start = il.find("label %s" % addr)
        pos_cjmp = il.find("cjmp", pos_start)
        pos_jmp = il.find("jmp", pos_start)

        if pos_cjmp >= 0 and pos_cjmp < pos_jmp:
            pos_jmp = pos_cjmp

        pos = il[:pos_jmp].rfind("\n")

        if pos_cjmp <> pos_jmp:
            if dest == "None":
                il = il[:pos] + '\njmp "lift_error"\n' +il[pos:]
            else:
                lbl = int(dest.split(":")[0], 16)
                lbl = hex(lbl)
                lbl = lbl.upper()[2:]
                lbl = "pc_0x%s" % lbl
                il = il[:pos] + '\ncjmp (%s<>%s), "lift_error", "%s"\n'%(exp, dest, lbl) +il[pos:]
        else:
            block = il[pos+5:].split("\n")[0].split(",")
            cnd = block[0]
            lbl = block[2]
            if dest == "None":
                il = il[:pos] + '\ncjmp (%s), "lift_error", %s\n'%(cnd, lbl) +il[pos:]
            else:
                il = il[:pos] + "ERROR" + il[pos:]
    return il

def process_subst(il, vars):
    while True:
        old_il = il
        for vname in vars.keys():
            il = il.replace("$%s"%vname, vars[vname])
        print "end subst"
        if old_il == il:
            break
    return il

def bap_add_to_int(add):
    return int(add.split(":")[0], 16)
def add_to_bap_label(add):
    return "pc_%s" % add_to_bap_no_type(add)
def add_to_bap_no_type(add):
    return "0x%s" % (hex(add).upper()[2:])

def preprocess(src_file, dst_file):
    il = "\n"+open(src_file).read()
    (il,deps) = process_include(il)
    deps.add(src_file)

    il = process_comment(il)
    (il, vars) = process_vars(il)

    if vars.has_key("debug"):
        kw = {}
        kw["il"] = il
        kw["vars"] = vars
        kw["bap_add_to_int"] = bap_add_to_int
        kw["add_to_bap_label"] = add_to_bap_label
        kw["add_to_bap_no_type"] = add_to_bap_no_type
        
        exec vars["debug"] in kw
        il = kw["il"]
        vars = kw["vars"]

    il = process_specials(il, vars)

    il = process_subst(il, vars)

    il = process_ind_jmp(il)
    il = process_asserts(il)

    il = preprocess_inst_count(il)

    f = open(dst_file, "w")
    f.write(il)
    f.close()

def preprocess_inst_count(il):
    pos = 0
    while True:
        pos1 = il.find("label pc_", pos)
        if pos1 < 0:
            break
        pos1 = il.find("\n", pos1+1)
        il = il[:pos1]+"\nint_count=int_count+1:u64"+il[pos1:]
        pos = pos1
    return il

def process_specials(il, vars):
    il = process_last_pc(il, vars)
    il = process_geflag(il)
    il = process_unsupported(il)
    il = process_print(il, vars)
    il = process_eflag(il, vars)
    il = process_movs(il, vars)
    il = process_mcr_p15(il, vars)
    # no more requirements after fixing by hamed
    #il = process_pop_pc(il, vars)
    il = process_msr(il, vars)
    il = process_patches(il, vars)
    return il

def process_last_pc(il, vars):
    if not vars.has_key("last_pc"):
        return il
    last_label = "label pc_%s\n"%vars["last_pc"]
    pos = il.find(last_label)
    if pos < 0:
        last_label = "label %s\n"%vars["last_pc"]
        pos = il.find(last_label)
    pos2 = il.find("jmp", pos)
    il = il[:pos2-1] + "\n\njmp \"success\"\n\n" + il[pos2-1:]
    return il

def process_geflag(il):
    lines = il.split("\n")
    lines = [l for l in lines if l.upper().find("GE_FLAG") < 0]
    return "\n".join(lines)

def process_print(il, vars):
    if not vars.has_key("print_add"):
        return il
    il = il.replace("jmp  \"pc_%s\""%vars["print_add"], "")
    return il

def process_eflag(il, vars):
    il = il.replace("~E_FLAG:bool &", "")
    return il

def process_movs(il, vars):
    pos = il.find('@asm  "movs pc, lr"')
    pos2 = il.find("jmp", pos)
    il = il[:pos2] + "\nPC:u32=LR_SVC:u32\n" + il[pos2:]
    return il

def process_mcr_p15(il, vars):
    il = process_mcr_p15_inner(il, vars, "c3", "CR3", "r0")
    il = process_mcr_p15_inner(il, vars, "c2", "CR2", "r0")
    il = process_mcr_p15_inner(il, vars, "c1", "CR1", "r1")

    # debug to elimate problems in tlb_invalidate
    il = process_mcr_p15_inner(il, vars, "c8", "CR6", "r0", "c6")
    il = process_mcr_p15_inner(il, vars, "c8", "CR6", "r3", "c5")
    il = process_mcr_p15_inner(il, vars, "c8", "CR6", "r3", "c7")

    il = process_mrc_p15_inner(il, vars, "c1", "CR1", "r1")
    il = process_mrc_p15_inner(il, vars, "c2", "CR2", "r2")
    il = process_mrc_p15_inner(il, vars, "c3", "CR3", "r3")
    il = process_mrc_p15_inner(il, vars, "c5", "CR5", "r1")
    il = process_mrc_p15_inner(il, vars, "c6", "CR6", "r0")

    return il

def process_mcr_p15_inner(il, vars, cname, bapname, regname, other_cname="c0"):
    print "processing MCR"
    pos = 0
    while True:
        pos = il.find('@asm  "mcr p15, #0, %s, %s, %s"'%(regname, cname, other_cname), pos)
        print cname, pos
        if (pos < 0):
            return il
        pos1 = il.find("label pc_", pos)
        pos2 = il.find("\n", pos1)
        add = il[pos1+len("label pc_"):pos2].strip()
        add = int(add, 16)
        old_add = add
        add = add+4
        il = il[:pos2] + """
PC:u32 = 0x%s:u32
%s:u32 = R0:u32
CR0:u32 = 0:u32
jmp "pc_0x%s"
"""%(hex(old_add).upper()[2:], bapname, hex(add).upper()[2:])  + il[pos2:]
        pos = pos+1
    return il

def process_mrc_p15_inner(il, vars, cname, bapname, regname):
    print "processing MRC"
    pos = 0
    while True:
        pos = il.find('@asm  "mrc p15, #0, %s, %s, c0"'%(regname, cname), pos)
        print cname, pos
        if (pos < 0):
            return il
        pos1 = il.find("label pc_", pos)
        pos2 = il.find("\n", pos1)
        add = il[pos1+len("label pc_"):pos2].strip()
        add = int(add, 16)
        old_add = add
        add = add+4
        il = il[:pos2] + """
PC:u32 = 0x%s:u32
%s:u32 = %s:u32
jmp "pc_0x%s"
"""%(hex(old_add).upper()[2:], regname.upper(), bapname, hex(add).upper()[2:])  + il[pos2:]
        pos = pos+1
    return il

def process_msr(il, vars):
    print "processing MSR"
    pos = il.find('@asm  "msr spsr_fc, #16"')
    if (pos < 0):
        return il
    pos1 = il.find("label pc_", pos)
    pos2 = il.find("\n", pos1)
    add = il[pos1+len("label pc_"):pos2].strip()
    add = int(add, 16)
    old_add = add
    add = add+4
    il = il[:pos2] + """
PC:u32 = 0x%s:u32
SVC_N_FLAG:bool = false
SVC_C_FLAG:bool = false
SVC_Z_FLAG:bool = false
SVC_V_FLAG:bool = false
SVC_Q_FLAG:bool = false
jmp "pc_0x%s"
"""%(hex(old_add).upper()[2:], hex(add).upper()[2:])  + il[pos2:]
    return il


def process_pop_pc(il, vars):
    pos = il.find('@asm  "pop {r4-r6, pc}"')
    if (pos < 0):
        return il
    pos1 = il.find("label pc_", pos)
    pos2 = il.find("\n", pos1)
    add = il[pos1+len("label pc_"):pos2].strip()
    add = int(add, 16)
    old_add = add
    add = add+4
    il = il[:pos2] + """
PC:u32 = 0x%s:u32
tmp_sp_svc:u32 = (SP_SVC:u32 + low:u32(0x10:u64))
tmp_pc:u32 = mem:?u32[(SP_SVC:u32 + low:u32(0xC:u64)),e_little]:u32
tmp_r6:u32 = mem:?u32[(SP_SVC:u32 + low:u32(0x8:u64)),e_little]:u32
tmp_r5:u32 = mem:?u32[(SP_SVC:u32 + low:u32(0x4:u64)),e_little]:u32
tmp_r4:u32 = mem:?u32[(SP_SVC:u32 + low:u32(0x0:u64)),e_little]:u32
R4:u32 = tmp_r4:u32
R5:u32 = tmp_r5:u32
R6:u32 = tmp_r6:u32
PC:u32 = tmp_pc:u32
SP_SVC:u32 = tmp_sp_svc:u32
jmp "lift_error"
"""%(hex(old_add).upper()[2:])  + il[pos2:]
    return il



def process_patches(il, vars):
    print "processing patches"
    il1 = []
    for l in il.split("\n"):
        if l.find("PC:u32 = \"pc_") < 0:
            il1 += [l]
    il = "\n".join(il1)
    for err in ["strne r0, [r3, #33]", "streq r0, [r3, #37]", "movhi pc, lr"]:
        pos = 0
        while pos >= 0:
            pos = il.find('@asm  "%s"'%err, pos)
            if pos < 0:
                break
            pos1 = il.find("label pc_", pos)
            pos2 = il.find("\n", pos1)
            add = il[pos1+len("label pc_"):pos2].strip()
            add = int(add, 16)
            old_add = add
            add = add+4
            il = il[:pos2] + """
PC:u32 = 0x%s:u32
jmp "pc_0x%s"
"""%(hex(old_add).upper()[2:], hex(add).upper()[2:])  + il[pos2:]
            pos = pos1
    return il


def process_unsupported(il):
    lines = il.split("\n")
    def change_unsupported(l):
        if l.find("UNSUPPORTED") >= 0:
            return "jmp \"lift_error\""
        else:
            return l
    lines = map(change_unsupported, lines)
    return "\n".join(lines)

def change_stp(fname):
    stp = open(fname).read()
    stp = stp.replace("ASSERT(", "QUERY(")
    stp = stp.replace("QUERY(FALSE);", "")
     
    o = open(fname, "w")
    o.write(stp)
    o.close()

def print_usage():
    print "usage baputils.py"
    print "  --preprocess src.il dst.il: preprocess an il file"
    print "  --change-stp src.stp: change STP file"
    exit(1)

def main():
    import sys
    if len(sys.argv) <= 2:
        print_usage()
    if sys.argv[1] == "--preprocess":
        if len(sys.argv) != 4:
            print_usage()
        preprocess(sys.argv[2], sys.argv[3])
        exit(0)
    if sys.argv[1] == "--change-stp":
        if len(sys.argv) != 3:
            print_usage()
        change_stp(sys.argv[2])
        exit(0)

if __name__ == "__main__":
    main()


