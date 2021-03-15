import sys
sys.path.extend(['/home/admin1/Desktop/MScThesis', '/home/admin1/anaconda3/lib/python37.zip', '/home/admin1/anaconda3/lib/python3.7', '/home/admin1/anaconda3/lib/python3.7/lib-dynload', '/home/admin1/.local/lib/python3.7/site-packages', '/home/admin1/anaconda3/lib/python3.7/site-packages', '/home/admin1/anaconda3/lib/python3.7/site-packages/pyelftools-0.25-py3.7.egg', '/home/admin1/anaconda3/lib/python3.7/site-packages/netifaces-0.10.4-py3.7-linux-x86_64.egg', '/home/admin1/anaconda3/lib/python3.7/site-packages/IPython/extensions', '/home/admin1/.ipython'])
sys.path.append('.')
import matplotlib.pyplot
matplotlib.pyplot._IP_REGISTERED = True # Hack
#import fuzzingbook_utils
import fuzzingbook
from fuzzingbook.GrammarMiner import CallStack
import jsonpickle
import os, subprocess
import gdb
import re, json


CALL = 'callq'
RETURN = 'retq'
LINE = 'line'
ARG_REGISTERS = ['rdi', 'rsi', 'rdx', 'rcx', 'r8', 'r9', 'rbx']
REGISTERS = ARG_REGISTERS + ['rax', 'rsp', 'rbp', 'al']

class Instruction(object):
    def _parse(self, instr):
        instr_list = instr.split()
        instr_list.pop(0)

        self.current_address = instr_list[0]
        if "<" in instr_list[1]:
            instr_list.pop(1)
        self.instr_type = instr_list[1]

        if self.instr_type == CALL:
            self.pointed_address = self.get_pointed_value(instr_list[2])
            if len(instr_list) > 3:
                self.symbol_name = instr_list[-1]

        elif self.instr_type.startswith('mov') or self.instr_type == 'push' or \
            self.instr_type.startswith('cmp'):
            self.dest_reg = self.resolve_addressing_mode(instr_list[2])
        
        elif self.instr_type == 'add' or self.instr_type =='sub':
            self.dest_reg = self.resolve_binary_operations(instr_list[2], self.instr_type)

    def resolve_binary_operations(self, instr, instr_type):
        instr = instr.split(',')
        if len(instr) == 2:
            src, dest = instr
            if '(' not in src: 
                src = '$%s' % (src[1:]) if src.startswith('%') else src[1:]
            else: 
                return  

            if '(' not in dest: 
                dest = '$%s' % (dest[1:]) if dest.startswith('%') else dest[1:]
            else: 
                return

            if instr_type == 'add':
                return '%s+%s' % (src, dest)
            elif instr_type == 'sub':
                return '%s+%s' % (src, dest)

    def resolve_addressing_mode(self, instr):
        str0 = instr.split(',')
        if len(str0) > 2:
            if instr.startswith('%'):
                return '$%s' % (str0[0][1:])
            else:
                if instr.startswith('-') or instr.startswith('0x'):
                    d, rb = tuple(str0[0].split('(%'))
                    ri = str0[1][1:]
                    s = str0[2].strip(')')
                    return '$%s+%s+%s*$%s' % (rb, d, s, ri)

        src = str0[-1]
        if '(' not in src:
            if src[1:] in REGISTERS:
                return '$%s' % src[1:]
        else:
            if src.startswith('-'):
                displacement, rest = tuple(src.split('(%'))
                return '$%s%s' % (rest[:-1], displacement)
            elif src.startswith('(') and src.endswith(')'):
                return '$%s' % src[2: -1]
            else:
                displacement, rest = tuple(src.split('(%'))
                return '$%s+%s' % (rest[:-1], displacement)
            

    def get_pointed_value(self, val):
        val = val.strip('%*')
        if val in REGISTERS:
            ptr_addr = gdb.execute('x/s $%s' % (val),
                                to_string=True).split(':')
            return ptr_addr[0]
        return val

    def __init__(self, instr):
        self.symbol_name = None
        self.pointed_address = None
        self.dest_reg = None
        self.instr_type = None
        self._parse(instr)


INP_ARR = []
VAL_TUPLE = []

def reset_helper():
    global INP_ARR
    global VAL_TUPLE
    INP_ARR.clear()
    VAL_TUPLE.clear()

def get_names_from_symbols(objfile):
    names = []
    for name in objfile:
        name = name.split()
        name = name[-1].decode('utf-8')
        if '@@' in name:
            names.append(name.split('@@')[0])
            continue
        names.append(name)
    return names

def list_objfile_symbols():
    proc = subprocess.Popen(['nm', 'a.out'], stdout=subprocess.PIPE)
    output = proc.stdout.read()
    output = output.splitlines()
    return output

def get_function_names(inp, binary):
    fn_dict = {}
    fn_names = []

    symbols = list_objfile_symbols()
    functions = get_names_from_symbols(symbols)

    gdb.execute("set args '%s'" % inp)
    gdb.execute("file %s" % binary)
    gdb.execute('set confirm off')
    gdb.execute('run')
    for k in functions:
        try:
            s = gdb.execute('info address %s' % k,
            to_string=True).split(' ')
            if s[4].startswith('0x'):
                v = s[4].rstrip()
                u = v.strip('.')
                fn_dict[v] = k
            else:
                u = s[-1].rstrip()
                u = u.strip('.')
                fn_dict[u] = k
        except gdb.error:
            continue
    return fn_dict

class BinaryDebugger(object):
    def event_loop(self):
        main = self._get_main_address()
        mname = self._lookup_address(main, None)
        cs = CallStack()
        cs.enter(mname)
        x, self.mid = cs.method_id

        self._init_methodMap_mtdStack(mname)
        self._init_result(self.inp, arg1)
        self.break_at(main)
        self.resume()
        addr_range = self._get_address_range()
        t = ['main']

        while True:
            try:
                nexti = self.get_instruction()
                if self._in_scope(nexti, addr_range):
                    h = Instruction(nexti)
                    if h.instr_type == CALL:
                        name = self._lookup_address(h.pointed_address, h.symbol_name)
                        if not name or 'exit' in name:
                            self.step()
                            self.finish()
                        else:
                            self.step()
                            cs.enter(name)
                            t.append(name)
                            x, self.mid = cs.method_id
                            self.method_map[self.m_stack[-1]][-1].append(self.mid)
                            self.method_map[str(self.mid)] = [self.mid, name, []]
                            self.m_stack.append(str(self.mid))
                    elif h.instr_type == RETURN:
                        self.step()
                        c = [i for i, s in enumerate(t) if s in nexti]
                        if c:
                            for x in range(c[-1], len(t)):
                                cs.leave()
                                t.pop()
                                if len(self.m_stack) > 1: self.m_stack.pop()
                        else:
                            cs.leave()
                            t.pop()
                            if len(self.m_stack) > 1: self.m_stack.pop()
                        self.mid = cs.method_id[1]
                    else:
                        self.step()
                        val = read_register_val(h.dest_reg, self.inp)
                        if val:
                            # print(val, self.mid)
                            comparisons = self.result['comparisons']
                            self.result['comparisons'] = process_value(val, self.mid, \
                                self.inp, comparisons, self.result['original'])
                else:
                    self.finish()
            except gdb.error:
                break
        self.result['method_map'] = self.method_map
        self.result['comparisons'].sort(key= lambda x: x[0])
        with open('tree', 'w+') as f:
            obj = jsonpickle.encode(self.result)
            f.write(obj)

    def _init_result(self, inp, arg1=None):
        self.result = {'inputstr': inp,
                'arg': inp,
                'original': arg1,
                'comparisons': []}

    def _init_methodMap_mtdStack(self, mname):
        self.method_map = {'0':[0, None, [1]], '1': [1, mname, []]}
        self.m_stack = ['0', '1']

    def _lookup_address(self, addr, symbol):
        addr = addr.rstrip("\n")
        if addr in self.functions.keys():
            return self.functions[addr]
        else:
            if symbol:
                s0 = symbol[1:-1].split('@')[0]
                return s0
            return None

    def _get_main_address(self):
        entry = self._get_entry_address()
        self.break_at(entry)
        gdb.execute('run')

        instr = []
        while True:
            next_i = self.get_instruction()
            if CALL in next_i:
                break
            instr.append(next_i)
            self.step()

        instr = instr[-1].split()
        if len(instr) == 6:
            s = instr[3]
        else:
            s = instr[4]

        reg = s[-3:]
        main_addr = gdb.execute('p/x $%s' % reg, to_string=True)
        main_addr = main_addr.partition("= ")
        main_addr = main_addr[-1]

        return main_addr

    def _get_address_range(self):
        s1 = s2 = None
        e1 = e2 = None
        mappings = gdb.execute('info proc mappings', to_string=True)

        for i, line in enumerate(mappings.splitlines()):
            if i == 4:
                s1 = line.split()[0]
            elif i == 6:
                e1 = line.split()[1]
            elif i == 7:
                s2 = line.split()[0]
            elif i == 10:
                e2 = line.split()[1]
        return (s1, e1, s2, e2)

    def _set_logger(self):
        gdb.execute('set logging overwrite on')
        gdb.execute('set logging redirect on')
        gdb.execute('set logging on')

    def _get_entry_address(self):
        self.start_program()
        self.run()

        info_file = gdb.execute('info file', to_string=True)
        entry = None

        for line in info_file.splitlines():
            if 'Entry point' in line:
                entry = line.split(':')[1]
                break
        return entry

    def _in_scope(self, instr, addr_range):
        s1, e1, s2, e2 = addr_range
        instr = instr.split()
        instr.pop(0)

        current_addr = instr[0].strip(':')
        hex_val = int(current_addr, 16)
        if hex_val in range(int(s1, 16), int(e1, 16)) or \
            hex_val in range(int(s2, 16), int(e2, 16)):
            return True
        else:
            return False

    def start_program(self):
        gdb.execute("set args '%s'" % self.inp)
        gdb.execute("file %s" % self.binary)

    def break_at(self, address):
        gdb.execute("break *%s" % address)

    def finish(self):
        gdb.execute('finish')

    def get_instruction(self):
        return gdb.execute('x/i $rip', to_string=True)

    def nexti(self):
        gdb.execute('nexti')

    def resume(self):
        gdb.execute('continue')

    def run(self):
        gdb.execute('run')

    def step(self):
        gdb.execute('stepi')

    def __init__(self, inp, binary, fn_list):
        self.inp = inp
        self.binary = binary
        self.functions = fn_list
        self._set_logger()
        self.tree = {}
        self.mid = None
        self.method_map, self.m_stack = {}, []

def read_register_val(reg, original):
    if not reg:
        return None

    if reg == '$al':
        x = gdb.execute('p/c %s' % (reg), to_string=True)
        x = x.split()
        x = x[-1]
        return x[1:-1]

    val = read_as_string(reg)
    if not val or 'error' in val:
        return None

    elif val in original:
        return val
    else:
        if val[1:-1] in original:
            return val[1:-1]
        a = read_ptr_addr(reg)
        v = read_register_val(a, original)
        return v

def read_ptr_addr(reg):
    try:
        str1 = gdb.execute('x/a %s' % (reg), to_string=True)
        for idx, char in enumerate(str1):
            if str1[idx] == ':':
                addr_val = str1[idx + 1:]
                addr_val = addr_val.strip()
                break
        return addr_val
    except Exception:
        return
def read_as_string(reg):
    try:
        str0 = gdb.execute('x/s %s' % (reg), to_string=True)
        for idx, char in enumerate(str0):
            if str0[idx] == ':':
                str_val = str0[idx + 1:]
                str_val = str_val.strip()
                break
        return str_val
    except Exception:
        return

def process_value(val, mid, inputstr, comparisons, module):
    result = []

    if val and val != inputstr and mid:
        if len(val) > 1 and inputstr.count(val) == 1:
            idx = inputstr.find(val)
            if idx != -1:
                for idx in range(idx, idx + len(val)):
                    c = [idx, inputstr[idx], mid]
                    if c not in comparisons:
                        comparisons.append(c)
        elif len(val) > 1 and inputstr.count(val) > 1:
            for c in val:
                comparisons = verify_substr_path(inputstr, c, mid, module, comparisons)
        else:
            indexes_of_duplicates = [i for i, c in enumerate(inputstr) if val == c]
            if len(indexes_of_duplicates) == 1:
                idx = inputstr.find(val)
                comparisons.append([idx, val, mid])
            else:
                comparisons = verify_substr_path(inputstr, val, mid, module, comparisons)
    return comparisons

def choose_alt_char_from_list(alt_list, seen):
    alt = None
    for c in alt_list:
        if c not in seen:
            seen.append(c)
            alt = c
            break
    return alt

def  verify_substr_path(inputstr, val, mid, module, comparisons):
    new_str = None
    duplicate_idxes = [i for i, c in enumerate(inputstr) if val == c]
    # alt_chars = get_alt_chars(inputstr)
    # get_alternate_chars(val)
    alt_chars = get_alternate_chars_to(val)

    for i in duplicate_idxes:
        new_str = inputstr
        for j in duplicate_idxes:
            if i != j:
                c = select_unseen_alt_char(alt_chars);
                new_str = replace_char_at_index(c, new_str, j)

        print(new_str)
        comp = check(new_str, [val, mid], module)
        # print(new_str)
        # print('-----------------------------')
        # print(comp)
        # exit()
        if comp and comp not in comparisons:
            comparisons.append(comp)
            break
    return comparisons

    # for i in duplicate_idxes:
    #     new_str = inputstr
    #     seen = []
    #     for j in duplicate_idxes:
    #         if i != j:
    #             c = choose_alt_char_from_list(list_of_alt, seen)
    #             new_str = new_str[:j] + c + new_str[j + 1:]

    #     comp = check(new_str, [val, mid], module)
        # if comp and comp not in comparisons:
        #     comparisons.append(comp)
        #     break
    # return comparisons


    # for idx in duplicate_idxes:
    #     new_str = inputstr
    #     rest_idexes = list(duplicate_idxes)
    #     rest_idexes.remove(idx)
    #     new_str = get_mutated_string(rest_idexes, alt_chars, new_str)

    #     comp = check(new_str, [idx, val, mid], module, inputstr)
    #     if comp in comparisons:
    #         return comparisons
    #     elif comp not in comparisons:
    #         comparisons.append(comp)
    #         return comparisons
    # return comparisons

import string
import random
def get_alternate_chars_to(s):
    if s.isdigit():
        return list(string.digits.strip(s))
    elif s.isalpha():
        return list(string.ascii_letters.strip(s))
    elif s in '()/-+*':
        return '()/-+*'.strip(s)
    else:
        return list(string.ascii_letters.strip(s))


def select_unseen_alt_char(alt_chars):
    return random.choice(alt_chars)

def replace_char_at_index(c, inputstr, j):
    return inputstr[:j] + c + inputstr[j + 1:]


def get_alt_chars(oldstr):
    choosen = set()
    if oldstr.isdigit() or '(' in oldstr or ')' in oldstr or '+' in oldstr \
        or '-' in oldstr or '*' in oldstr or '/' in oldstr:
        choosen = set(list(string.digits)) - set(list(oldstr))
    else:
        choosen = set(list(string.ascii_letters)) - set(list(oldstr))
    return list(choosen)


def check(new_str, comparison, module):
    with open(f'inp.1.txt', 'w+') as f:
        print(new_str, file=f)

    subprocess.call(['gcc', '-g', module])
    subprocess.call(['gdb', '--batch-silent', "-x", "check.py"])

    result = []
    with open(f'updated_comparisons', 'rb') as f:
        result = jsonpickle.decode(f.read())

    for item in result:
        a, b, c = item
        if b == comparison[0] and c == comparison[1]:
            return item

reset_helper()
arg_0 = None
with open(f'inp.0.txt', 'r+') as f:
    arg_0 = f.read().strip()

fnames = get_function_names(arg_0, "a.out")
subprocess.call(['strip', '-s', "a.out"])

debugger = BinaryDebugger(arg_0, 'a.out', fnames)
debugger.event_loop()



# for i in duplicate_idxes:
#     new_str = inputstr
#     for j in duplicate_idxes:
#         if i != j:
#             c = select_unseen_alt_char(alt_chars);
#             new_str = replace_char_at_index(c, new_str, j)

#     comp = check(new_str, [i, val, mid], module, inputstr)
#     if comp and comp not in comparisons:
#         comparisons.append(comp)
#         return comparisons
#     print()
    # elif comp and comp in comparisons:
    #     return comparisons