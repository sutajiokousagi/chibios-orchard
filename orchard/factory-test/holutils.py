#!/usr/bin/python

import asmutils
import gdbutils

asm_file = "../../verification/out/sth_ovp_arm9.asm"
elf_file = "../../verification/out/sth_ovp_arm9.elf"
constant_def = "../../out/hypervisor_constants.sml"
constant_value = "../../out/hypervisor_values.sml"

symbols = asmutils.get_static_symbols(asm_file)
functions = asmutils.get_f_symbols(asm_file)

s = gdbutils.open_session(elf_file)
types = gdbutils.get_all_types(s)

constantf = open(constant_def, "w")
constantf.write("""
(**************************************)
(*           KTH constants            *)
(**************************************)

(* global symbol locations *)
""")
for symb in symbols.keys():
    constantf.write('new_constant ("global_%s_add", ``:word32``);\n' % symb)

constantf.write("(* function symbols *)\n")
for fun in functions.keys():
    constantf.write('new_constant ("f_%s_add", ``:word32``);\n' % fun)

constantf.write("(* type sizes and offsets *)\n")
for type_name in types.keys():
    type = types[type_name]
    if type is None:
        continue
    constantf.write('new_constant ("t_%s_len", ``:word32``);\n' % type_name)
    if not isinstance(type, gdbutils.StructType):
        continue
    for f_name in type.fields.keys():
        field = type.fields[f_name]
        constantf.write('new_constant ("o_%s__%s", ``:word32``);\n' % (type_name, f_name))


constantf.close()


axioms = []
constantv = open(constant_value, "w")
constantv.write("""
(**************************************)
(*           KTH constants values     *)
(**************************************)

(* global symbols *)
""")
for symb in symbols.keys():
    try:
        symb_value = int(symbols[symb], 16)
        cname = "global_%s_add" % symb
        constantv.write("""
val %s_axiom = new_axiom("%s_axiom",
   ``%s = %sw``);
""" % (cname, cname, cname, symbols[symb]))
        axioms.append(cname)
    except:
        pass

constantv.write("(* function symbols *)\n")
for fun in functions.keys():
    cname = "f_%s_add" % fun
    constantv.write("""
val %s_axiom = new_axiom("%s_axiom",
   ``%s = %sw``);
""" % (cname, cname, cname, functions[fun]))
    axioms.append(cname)


constantv.write("(* type sizes and offsets *)\n")
for type_name in types.keys():
    type = types[type_name]
    if type is None:
        continue

    cname = "t_%s_len" % type_name
    constantv.write("""
val %s_axiom = new_axiom("%s_axiom",
   ``%s = %sw``);
""" % (cname, cname, cname, type.size))
    axioms.append(cname)

    if not isinstance(type, gdbutils.StructType):
        continue
    for f_name in type.fields.keys():
        field = type.fields[f_name]
        cname = "o_%s__%s" % (type_name, f_name)
        constantv.write("""
val %s_axiom = new_axiom("%s_axiom",
   ``%s = %sw``);
""" % (cname, cname, cname, field["offset"]))
        axioms.append(cname)

constantv.write("""
(* All axioms *)
val hypervisor_constants_axioms = [
%s
];
""" % ",\n".join(["%s_axiom"%a for a in axioms]))

constantv.close()
