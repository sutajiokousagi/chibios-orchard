#!/usr/bin/python

import os

import sys

dest_path = "../out/indirect/"
main_file = sys.argv[1]

def get_out_path(fname):
    return dest_path + fname

def get_master_file(fname):
    return "src/from_hol/%s.il" % fname
def get_dot_file(fname, iter):
    return get_out_path("%s_%s.dot" % (fname, iter))
def get_pdf_file(fname, iter):
    return get_out_path("%s_%s.pdf" % (fname, iter))
def get_input_file(fname, iter):
    return get_out_path("%s_input_%s.il" % (fname, iter))
def get_processing_file(fname, iter):
    return get_out_path("%s_processing_%s.il" % (fname, iter))
def get_solving_file(fname, iter):
    return get_out_path("%s_solving_%s.il" % (fname, iter))
def get_stp_file(fname, iter):
    return get_out_path("%s_solving_%s.stp" % (fname, iter))
def get_stp_out_file(fname, iter):
    return get_out_path("%s_solving_%s.out" % (fname, iter))
def get_end_file(fname):
    return get_out_path("%s_end.il" % (fname))

def pdf_cfg(iter):
    (fin, fout) = os.popen2(" dot -Tpdf  " + get_dot_file(main_file, iter) + " -o " + get_pdf_file(main_file, iter))
    fout.read()

def compute_cfg(iter):
    (fin, fout) = os.popen2("../../utils/scripts/baputils.py --preprocess " + get_input_file(main_file, iter) + " " + get_processing_file(main_file, iter))
    fout.read()
    (fin, fout, fex) = os.popen3("iltrans -il "+get_processing_file(main_file, iter)+" -to-cfg -to-ssa -pp-ssa " + get_dot_file(main_file, iter))
    fex.read()
    fout.read()

def parse_cfg(iter):
    cfg = open(get_dot_file(main_file, iter)).read()
    pos1 = cfg.find("{")
    pos2 = cfg.rfind("}")
    cfg = cfg[pos1+1:pos2-1]
    cfg = cfg.split(";\n")
         
    nodes = []
    nodes_add = {}
    for n in [c for c in cfg if c.find("->") < 0 and c.strip() <> ""]:
        nname = n.split()[0]
        nodes.append(nname)
        if (n.find("label ") >= 0):
            naddr = n.split("label ")[1].split("\\n")[0]
            nodes_add[nname] = naddr
         
    edges = {}
    for node in nodes:
        edges[node] = []
         
    for edge in [c.split() for c in cfg if c.find("->") >= 0]:
        edges[edge[0]] = edges[edge[0]]+[edge[2]]

    return (nodes, nodes_add, edges)

def select_ind_jmp(cfg):
    frontier = ["BB_Entry"]
    ind = []
    visited = []
    (nodes, nodes_add, edges) = cfg
    while len(frontier) > 0:
        node = frontier[0]
        visited.append(node)
        for target in edges[node]:
            if target == "BB_Indirect":
                ind.append(node)
            elif visited.count(target) == 0:
                frontier = frontier + [target]
        frontier = frontier[1:]
     
    #print [nodes_add[n] for n in ind]
    #print [nodes_add[n] for n in visited if nodes_add.has_key(n)]

    if len(ind) == 0:
        return (None, [], [])

    ind = list(set(ind))

    ind_jmp = nodes_add[ind[0]]
     
    other_ind = []
    for node in nodes:
        if not nodes_add.has_key(node):
            continue
        if nodes_add[node] == ind_jmp:
            continue
        if edges[node].count("BB_Indirect") > 0:
            other_ind.append(nodes_add[node])

    return (ind_jmp, other_ind, [nodes_add[n] for n in ind[1:]])

def suppress_ind_jmps(content, jmps):
    for no_jmp in jmps:
        content = content.replace("label %s\n"%no_jmp, "label %s\njmp \"success\"\n"%no_jmp)
    return content

def extract_exp(content, ind_jmp):
    pos = content.find("label %s\n"%ind_jmp)
    pos1 = content.find("jmp", pos)
    pos1a = content.find("cjmp", pos)
    cnd = "true"
    if pos1a >=0 and pos1a < pos1:
        pos1 = pos1a
    pos2 = content.find(" ", pos1)
    pos3 = content.find("\n", pos2)
    if pos1a == pos1:
        res = content[pos2:pos3].split(",")
        return (res[0], res[1])
        
    return (cnd, content[pos2:pos3])

def query_ind_jmp(content, ind_jmp):
    (cnd, exp) = extract_exp(content, ind_jmp)

    pos = content.find("label %s\n"%ind_jmp)
    pos1 = content.find("jmp", pos)
    pos1a = content.find("cjmp", pos)
    if pos1a >=0 and pos1a < pos1:
        pos1 = pos1a
    pos2 = content.find("\n", pos1)
     
    content = content[:pos1] + """
    resolve_%d:bool = (~%s) | (%s <> free_resolve_%s:u32)
    cjmp resolve_%d, "success", "lift_error"
    """%(iteration, cnd, exp, iteration, iteration) + content[pos2:]
     
    content = content.replace(
        "jmp \"end\"\nlabel end\n",
        "goal:bool = ((~precondition)|true)\njmp \"end\"\nlabel end\n")
    return content

def solve_ind_jump(content, iteration):
    out = open(get_solving_file(main_file, iteration), "w")
    out.write(content)
    out.close()

    (fin, fout) = os.popen2("/opt/trunk/utils/topredicate -validity -il "+get_solving_file(main_file, iteration)+"  -post goal -stp-out " + get_stp_file(main_file, iteration))
    fout.read()

    (fin, fout) = os.popen2("stp " + get_stp_file(main_file, iteration))
    res = fout.read()
    fd = open(get_stp_out_file(main_file, iteration), "w")
    fd.write(res)
    fd.close()
    res = res.split("\n")
    if res[0] == "Valid.":
        return None
    #print "\n".join(res)
    res = [r for r in res if r.find("free_resolve_%s"%iteration) >= 0]

    add = res[0].split("=")[1].split()[0]
    return add

def patch_solved_ind(iteration, ind_jmp, fix_add):
    content_exp = open(get_processing_file(main_file, iteration)).read()
    (cnd, exp) = extract_exp(content_exp, ind_jmp)

    content = open(get_input_file(main_file, iteration)).read()
    if fix_add is None:
        content = """
#ind_jmp %s [[%s]] None
""" %(ind_jmp, exp) + content
    else:
        content = """
#ind_jmp %s [[%s]] %s:u32
""" %(ind_jmp, exp, fix_add) + content

    fout = open(get_input_file(main_file, iteration+1), "w")
    fout.write(content)
    fout.close


iteration = 1
fout = open(get_input_file(main_file, iteration), "w")
fout.write(open(get_master_file(main_file)).read())
fout.close()

while True:
    compute_cfg(iteration)
    cfg = parse_cfg(iteration)
    (ind_jmp, other_ind, other_act_ind) = select_ind_jmp(cfg)

    print "SOLVING", ind_jmp, other_act_ind

    if ind_jmp is None:
        break

    content = open(get_processing_file(main_file, iteration)).read()
    content = suppress_ind_jmps(content, other_ind)
    content = query_ind_jmp(content, ind_jmp)

    fix_add = solve_ind_jump(content, iteration)
    print ind_jmp, fix_add

    # fix when selected a dead path and other indirect jump are active
    while fix_add is None and len(other_act_ind) > 0:
        iteration += 1
        old_imp = ind_jmp
        ind_jmp = other_act_ind.pop()
        print "DEAD CODE and other IND JMP", ind_jmp, other_act_ind
        other_ind.append(old_imp)
        other_ind.remove(ind_jmp)

        # really bad path
        fout = open(get_input_file(main_file, iteration), "w")
        fout.write(open(get_input_file(main_file, iteration-1)).read())
        fout.close()
        compute_cfg(iteration)

        content = open(get_processing_file(main_file, iteration-1)).read()
        content = suppress_ind_jmps(content, other_ind)
        content = query_ind_jmp(content, ind_jmp)

        fix_add = solve_ind_jump(content, iteration)
        print ind_jmp, fix_add
        
    patch_solved_ind(iteration, ind_jmp, fix_add)
    iteration += 1

fout = open(get_end_file(main_file), "w")
fout.write(open(get_input_file(main_file, iteration)).read())
fout.close()
