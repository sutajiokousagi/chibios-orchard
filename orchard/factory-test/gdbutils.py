#!/usr/bin/python

import logging

import sys
sys.path.append("./python-gdb-mi") 

import gdbmi
        

def sync_exec(session, query):
    lines = []
    def handler(token, obj):
        return True
    def console_handler(token, obj):
        lines.append(obj.what)
        return True

    token = session.send(query, handler, console_handler)
    while not session.wait_for(token): pass

    res = "".join(lines)
    res = res.replace('"', "")
    res = res.replace('\\n', "\n")
    return res
    

def list_types(session):
    all_types = set()
    res = sync_exec(session, "info types")
    files = res.split("\nFile ")[1:]
    for f in files:
        parts = f.split(":\n")
        f_name = parts[0]
        f_types = parts[1]
        f_types = f_types.split(";\n")
        f_types = [r for r in f_types if r.find("typedef") >= 0]
        f_types = [r.split()[-1] for r in f_types]
        f_types = [r.split("[")[0] for r in f_types]
        all_types = all_types.union(f_types)
    return all_types


class EnumType:
    def __init__(self, name, size, values):
        self.name = name
        self.size = size
        self.values = values
    def __str__(self):
        return "ENUM<%s>"%(",".join(self.values))
    @classmethod
    def parse(cls, name, txt, size):
        t = EnumType(name, size, [])
        return t

class PrimitiveType:
    def __init__(self, name, size):
        self.name = name
        self.size = size
    def __str__(self):
        return "PRIMITIVE"
    @classmethod
    def parse(cls, name, txt, size):
        t = PrimitiveType(name, size)
        return t

class StructType:
    def __init__(self, name, size, fields):
        self.name = name
        self.size = size
        self.fields = fields
    def __str__(self):
        return "PRIMITIVE"
    @classmethod
    def parse(cls, name, txt, size, s):
        txt = txt.split("{")[-1].split("}")[0]
        fields = txt.split(";")
        fields = [f.strip() for f in fields if f.strip() != '']
        fields = [f.split()[-1] for f in fields]
        # removes array annotation
        fields = [f.split("[")[0] for f in fields]
        # removes fpoint annotations
        fields = [f.split(")")[0].split("(")[-1] for f in fields]
        # removes pointers annotations
        fields = [f.split("*")[-1] for f in fields]
        curr_type = {}
        for field_name in fields:
            curr_field = {}
            txt = sync_exec(s, "print (int)&(((%s*)0)->%s)" % (name, field_name))
            curr_field["offset"] = int(txt.split("=")[-1].strip())
            curr_type[field_name] = curr_field

        return StructType(name, size, curr_type)

def dump_type(session, type_name):
    curr_type = {}
    txt = sync_exec(session, "ptype %s" % type_name)
    txt = txt[txt.find("type = ") + len("type = "):]
    txt_size = sync_exec(session, "print sizeof(%s)" % type_name)
    if (txt_size.strip() == ""):
        return None
    txt_size = txt_size.split("=")[-1].strip().split()[0]
    size = int(txt_size)
    if txt.find("enum") == 0:
        return EnumType.parse(type_name, txt, size)
    if txt.find("(*)") >= 0:
        return None
    if txt.find("{") < 0:
        return EnumType.parse(type_name, txt, size)
    if txt.strip() == type_name:
        return PrimitiveType.parse(type_name, txt, size)
    return StructType.parse(type_name, txt, size, session)

def get_all_types(s):
    types = list_types(s)
    st_types = {}
    for type_name in types:
        curr_type = dump_type(s, type_name)
        st_types[type_name] = curr_type
    return st_types

def gen_types(s, dst_file):
    st_types = get_all_types(s)

    f = open(dst_file, "w")
    for type_name in st_types.keys():
        type = st_types[type_name]
        if type is None:
            continue
        f.write("#set t_%s_len=[[%s:u32]]\n"%(type_name, type.size))
        if not isinstance(type, StructType):
            continue
        for f_name in type.fields.keys():
            field = type.fields[f_name]
            f.write("#set o_%s:%s=[[%s:u32]]\n"%(type_name, f_name, field["offset"]))
    f.close()
    return

def open_session(elf_file):
    logging.basicConfig(
        #level=logging.INFO,
        #level=logging.DEBUG,
        level=logging.ERROR,
        format='%(asctime)s '\
            '%(levelname)s '\
            '%(pathname)s:%(lineno)s '\
            '%(message)s')

    s = gdbmi.Session(debuggee = elf_file,
                      gdb = "arm-none-eabi-gdb")

    sync_exec(s, 'info version')
    return s

def print_usage():
    print "usage gdbutils.py"
    print "  --gen-struct src.elf dst.il: generate offset and len of types"
    exit(1)

def main():
    import sys
    if len(sys.argv) != 4:
        print_usage()

    s = open_session(sys.argv[2])
    gen_types(s, sys.argv[3])

if __name__ == "__main__":
    main()
