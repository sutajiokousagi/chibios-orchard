#!/usr/bin/python

def read_code(src_file):
    code = open(src_file).read().split("\n")
    return code

def gen_lifter(src_file, dst_file, il_file):
    code = read_code(src_file)
     
    ml_lift = open(dst_file, "w")
    ml_lift.write('loadPath := "/NOBACKUP/workspaces/robertog/holtoil" :: !loadPath;\n')
    ml_lift.write('load "new_lifter";\n')
    ml_lift.write('working_directory := "%s";' % il_file)
     
    idx = 0
    while True:
        line = code[idx]
        idx+=1
        blocks = line.split(":")
        if len(blocks) == 1:
            continue
        try:
            b0 = blocks[0]
            b0 = b0.strip()
            b0 = "0x%s" % b0
            addr = int(b0, 16)
            ml_lift.write("cmd_cntr  := %s;\n" % b0)
            rest = blocks[1].split()
            inst = rest[0]
            comm = " ".join(rest[1:])
            ml_lift.write("lift_instruction \"%s\"; " % inst)
            ml_lift.write("(* %s *)\n" % comm)
        except:
            if line.find("Disassembly of section ") >= 0:
                section = line.replace("Disassembly of section ", "")
                if section != ".text:":
                    break
            ml_lift.write("\n")
            ml_lift.write("(* %s *)\n" % blocks[0])
            ml_lift.write("""append_to_file "\\n// %s\\n";""" % blocks[0])
            continue
    ml_lift.close()
    return

def gen_static_mem(src_file, dst_file):
    code = read_code(src_file)
    while code[0].find("Disassembly of section .text:") < 0:
        code = code[1:]
    code = code[1:]
    all_req_memory_loc = set([])
    for line in code:
        blocks = line.split(":")
        if blocks[0].find("Disassembly of section") >= 0:
            break
        if len(blocks) == 1 or len(blocks[1]) == 0:
            continue

        if line.find(";") < 0:
            continue
        rest = line.split(";")[-1]
        rest = rest.strip()
        if rest.find("<") <= 0:
            continue
        req_mem_loc = int("0x%s"%rest.split(" ")[0], 16)
        all_req_memory_loc.add(req_mem_loc)

    static_mem = open(dst_file, "w")
    static_mem.write("loc_code_memory:bool = \n")

    for line in code:
        blocks = line.split(":")
        if blocks[0].find("Disassembly of section") >= 0:
            break
        if len(blocks) == 1 or len(blocks[1]) == 0:
            continue
        m_addr = blocks[0]
        m_addr = m_addr.strip()
        m_addr = "0x%s" % m_addr
        try:
            addr = int(m_addr, 16)
            if not addr in all_req_memory_loc:
                continue
            m_value = blocks[1].strip().split()[0]
            static_mem.write("  (old_mem[%s:u32, e_little]:u32 == 0x%s:u32) &\n" % (m_addr, m_value))
        except:
            pass
    static_mem.write("  true \n")
    static_mem.close()

def get_static_symbols(src_file):
    symbols = {}
    code = read_code(src_file)
    section = ""
    for line in code[2:]:
        blocks = line.split(":")
        if blocks[0].find("Disassembly of section") >= 0:
            section = blocks[0].split(".")[1]
        if section == "text":
            continue
        if section.find("debug") >= 0:
            continue
        if section.find("comment") >= 0:
            continue
        if section.find("ARM") >= 0:
            continue
        if len(blocks) != 2:
            continue
        if blocks[1] != "":
            continue
        blocks = blocks[0].split(" ")
        if len(blocks) != 2:
            continue
        m_addr = blocks[0]
        m_addr = "0x%s" % m_addr
        name = blocks[1][1:-1].replace(".", "")
        symbols[name] = m_addr
    return symbols


def gen_static_locs(src_file, dst_file):
    symbols = get_static_symbols(src_file)
    static_mem = open(dst_file, "w")
    for symb in symbols.keys():
        static_mem.write("#set global_%s_add=[[%s:u32]]\n" % (symb, symbols[symb]))
    static_mem.close()

def get_f_symbols(src_file):
    code = read_code(src_file)
    functions = {}
    section = ""
    for line in code[2:]:
        blocks = line.split(":")
        if blocks[0].find("Disassembly of section") >= 0:
            section = blocks[0].split(".")[1]
        if section == "":
            continue
        if section != "text":
            break
        if len(blocks) != 2:
            continue
        if blocks[1] != "":
            continue
        blocks = blocks[0].split(" ")
        if len(blocks) != 2:
            continue
        m_addr = blocks[0]
        m_addr = "0x%s" % m_addr
        name = blocks[1][1:-1].replace(".", "")
        functions[name] = m_addr
    return functions

def gen_f_symbols(src_file, dst_file):
    functions = get_f_symbols(src_file)
    static_mem = open(dst_file, "w")
    for fun in functions.keys():
        static_mem.write("#set f_%s_add=[[%s:u32]]\n" % (fun, functions[fun]))
    static_mem.close()

def gen_static_mem_data(symbol_name, size, src_file, dst_file):
    symbols = get_static_symbols(src_file)
    symbol_loc = int(symbols[symbol_name], 16)

    code = read_code(src_file)
    values = {}
    section = ""
    for line in code[2:]:
        blocks = line.split(":")
        if blocks[0].find("Disassembly of section") >= 0:
            section = blocks[0].split(".")[1]
        if section != "data" and section != "rodata":
            continue
        if section == "debug_abbrev":
            break
        if len(blocks) != 2:
            continue
        if blocks[1] == "":
            continue
        m_addr = blocks[0].strip()
        m_addr = "0x%s" % m_addr
        if int(m_addr, 16) < symbol_loc:
            continue
        if int(m_addr, 16)  >= symbol_loc + int(size, 16):
            continue
        value = blocks[1].strip().split()[0]
        value = "0x%s" % value
        values[m_addr] = value

    res = " &\n".join(
        ["(mem[%s:u32, e_little]:u32 == %s:u32)" % (add, values[add])
         for add in values.keys()
         ])
    
    static_mem = open(dst_file, "w")
    static_mem.write("(\n%s\n)" % (res))
    static_mem.close()

def print_usage():
    print "usage asmutils.py"
    print "  --gen_lifter src.asm dst.sml dst.dir: generate dst.ml file that lift the code to the file dst.dir/arm_to_ir.il"
    print "  --gen_static_mem src.asm dst.il: generate precondition for code momory referenced by the hypervisor"
    print "  --gen_static_locs src.asm dst.il: generate precondition for symbol location"
    print "  --gen_f_symbols src.asm dst.il: generate precondition for function location"
    exit(1)

def main():
    import sys
    if len(sys.argv) <= 2:
        print_usage()
    if sys.argv[1] == "--gen_lifter":
        if len(sys.argv) != 5:
            print_usage()
        gen_lifter(sys.argv[2], sys.argv[3], sys.argv[4])
        exit(0)
    if sys.argv[1] == "--gen_static_mem":
        if len(sys.argv) != 4:
            print_usage()
        gen_static_mem(sys.argv[2], sys.argv[3])
        exit(0)
    if sys.argv[1] == "--gen_static_locs":
        if len(sys.argv) != 4:
            print_usage()
        gen_static_locs(sys.argv[2], sys.argv[3])
        exit(0)
    if sys.argv[1] == "--gen_f_symbols":
        if len(sys.argv) != 4:
            print_usage()
        gen_f_symbols(sys.argv[2], sys.argv[3])
        exit(0)
    if sys.argv[1] == "--gen_static_mem_data":
        if len(sys.argv) != 6:
            print_usage()
        gen_static_mem_data(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
        exit(0)
    print_usage()

if __name__ == "__main__":
    main()

