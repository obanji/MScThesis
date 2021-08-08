import sys
sys.path.extend(['/home/admin1/Desktop/MScThesis', '/home/admin1/anaconda3/lib/python37.zip', '/home/admin1/anaconda3/lib/python3.7', '/home/admin1/anaconda3/lib/python3.7/lib-dynload', '/home/admin1/.local/lib/python3.7/site-packages', '/home/admin1/anaconda3/lib/python3.7/site-packages', '/home/admin1/anaconda3/lib/python3.7/site-packages/pyelftools-0.25-py3.7.egg', '/home/admin1/anaconda3/lib/python3.7/site-packages/netifaces-0.10.4-py3.7-linux-x86_64.egg', '/home/admin1/anaconda3/lib/python3.7/site-packages/IPython/extensions', '/home/admin1/.ipython'])
sys.path.append('.')
import matplotlib.pyplot
matplotlib.pyplot._IP_REGISTERED = True
#import fuzzingbook_utils
import fuzzingbook
from fuzzingbook.GrammarMiner import CallStack
import jsonpickle
import os, subprocess
import gdb
import re, json, random


CALL = 'callq'
RETURN = 'retq'

REGISTERS = ['rdi', 'rdx', 'rcx', 'r8', 'r9', 'rbx', 'rax', 'rsp', 'rbp', 'al']

class Instruction(object):
    def _parse(self, instr):
        instr_list = instr.split()
        instr_list.pop(0)

        self.current_address = instr_list[0]
        if "<" in instr_list[1]:
            instr_list.pop(1)
        self.instr_type = instr_list[1]

        if self.instr_type == CALL:
            self.pointed_address = self.get_pointed_address(instr_list[2])
            if len(instr_list) > 3:
                self.symbol_name = instr_list[-1]

        elif self.instr_type.startswith('mov') or self.instr_type == 'push' or \
            self.instr_type == 'pop' or self.instr_type == 'cmp':
            self.dest_reg = self.resolve_addressing_mode(instr_list[2])

    def resolve_addressing_mode(self, instruction):
        splited_instruction = instruction.split(',')
        if len(splited_instruction) > 2:
            if instruction.startswith('%'):
                return '$%s' % (splited_instruction[0][1:])
            else:
                if instruction.startswith('-') or instruction.startswith('0x'):
                    displacement, base = tuple(
                        splited_instruction[0].split('(%'))
                    index = splited_instruction[1][1:]
                    scale = splited_instruction[2].strip(')')
                    return '$%s+%s+%s*$%s' % (base, displacement, scale, index)

        src = splited_instruction[-1]
        if '(' not in src:
            if src[1:] in REGISTERS:
                return '$%s' % src[1:]
        else:
            if src.startswith('-'):
                displacement, rest = tuple(src.split('(%'))
                return '$%s%s' % (rest[:-1], displacement)
            elif src.startswith('(') and src.endswith(')'):
                return '$%s' % src[2:-1]
            else:
                displacement, rest = tuple(src.split('(%'))
                return '$%s+%s' % (rest[:-1], displacement)

    def get_pointed_address(self, inp_value):
        inp_value = inp_value.strip('%*')
        if inp_value in REGISTERS:
            pointed_address = gdb.execute('x/s $%s' % (inp_value),to_string=True)
            pointed_address = pointed_address.split(':')
            return pointed_address[0]
        return inp_value

    def __init__(self, instr):
        self.symbol_name = None
        self.pointed_address = None
        self.dest_reg = None
        self.instr_type = None
        self._parse(instr)


def list_objfile_symbols():
    proc = subprocess.Popen(['nm', 'a.out'], stdout=subprocess.PIPE)
    output = proc.stdout.read()
    output = output.splitlines()
    return output

    
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


def get_function_names(inp, binary):
    function_dict = {}
    function_names = []

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
                function_dict[v] = k
            else:
                u = s[-1].rstrip()
                u = u.strip('.')
                function_dict[u] = k
        except gdb.error:
            continue
    return function_dict


def get_curr_address(instr):
    instr = instr.split()
    instr.pop(0)
    return instr[0].strip(':')

class BinaryDebugger(object):
    def event_loop(self):
        main = self._get_main_address()
        mname = self._lookup_address(main, None)
        cs = CallStack()
        cs.enter(mname)
        _, self.mid = cs.method_id
        
        self._init_methodMap_methodStack(mname)
        self._init_result(self.inp, arg1)
        self.break_at(main)
        self.resume()
        addr_range = self._get_address_range()
        instr_count = 0
        
        while True:
            try:
                nexti = self.get_instruction()
                if self._in_scope(nexti, addr_range):
                    instr = Instruction(nexti)
                    if instr.instr_type == CALL:
                        name = self._lookup_address(instr.pointed_address, instr.symbol_name)
                        if not name or 'exit' in name or 'alloc' in name:
                            self.step()
                            self.finish()
                        else:
                            self.step()
                            cs.enter(name)
                            _, self.mid = cs.method_id
                            self.method_map[self.method_stack[-1]][-1].append(self.mid)
                            self.method_map[str(self.mid)] = [self.mid, name, []]
                            self.method_stack.append(str(self.mid))
                    elif instr.instr_type == RETURN:
                        self.step()
                        instr_count += 1
                        cs.leave()
                        if len(self.method_stack) > 1: 
                            self.method_stack.pop()
                        self.mid = cs.method_id[1]
                    else:
                        self.step()
                        instr_count += 1
                        value = read_register_value(instr.dest_reg, self.inp, instr.instr_type)
                        if value:
                            comparisons = self.result['comparisons']
                            self.result['comparisons'] = add_value_to_comparisons(value, self.mid, \
                                self.inp, comparisons, self.result['original'], instr_count)                           
                else:
                    self.finish()
            except gdb.error:
                break
        self.result['method_map'] = self.method_map
        self.result['comparisons'].sort(key= lambda x: x[0])
        with open('tree', 'w+') as f:
            result = jsonpickle.encode(self.result)
            f.write(result)

    def _init_result(self, inp, arg1=None):
        self.result = {
            'inputstr': inp,
            'arg': inp,
            'original': arg1,
            'methodMap': {},
            'comparisons': []
        }

    def _init_methodMap_methodStack(self, mname):
        self.method_map = {'0': [0, None, [1]], '1': [1, mname, []]}
        self.method_stack = ['0', '1']

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
        self.run()

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

        register = s[-3:]
        main_addr = gdb.execute('p/x $%s' % register, to_string=True)
        main_addr = main_addr.partition("= ")
        main_addr = main_addr[-1]

        return main_addr

    def _get_entry_address(self):
        self.start_program()
        self.run()

        info_file = gdb.execute('info file', to_string=True)
        entry = None

        entry_line = [
            line for line in info_file.splitlines() if 'Entry point' in line
        ]
        entry = entry_line[0]
        entry = entry.split(':')[1]
        return entry

    def _in_scope(self, instr, addr_range):
        s1, e1, s2, e2 = addr_range

        current_addr = get_curr_address(instr)
        hex_value = int(current_addr, 16)
        return True if hex_value in range(int(s1, 16), int(
            e1, 16)) or hex_value in range(int(s2, 16), int(e2, 16)) else False

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

    def start_program(self):
        gdb.execute("set args '%s'" % self.inp)
        gdb.execute("file %s" % self.binary)

    def _set_logger(self):
        gdb.execute('set logging overwrite on')
        gdb.execute('set logging redirect on')
        gdb.execute('set logging on')

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
        self.method_map, self.method_stack = {}, []


def split_string(string):
    return_str = None
    for idx, char in enumerate(string):
        if string[idx] == ':':
            return_str = string[idx + 1:]
            return_str = return_str.strip()
            break
    return return_str

def read_memory(reg, fmt):
    try:
        output_string1 = gdb.execute('%s %s' % (fmt, reg), to_string=True)
        return split_string(output_string1)
    except Exception:
        return  

# def read_as_string(reg):
#     try:
#         output_string1 = gdb.execute('x/s %s' % (reg), to_string=True)
#         return split_string(output_string1)
#     except Exception:
#         return

# def read_as_hex(reg):
#     try:
#         output_string2 = gdb.execute('x/x %s' % (reg), to_string=True)
#         return split_string(output_string2)
#     except Exception:
#         return

# def read_pointed_address(reg):
#     try:
#         output_string = gdb.execute('x/a %s' % (reg), to_string=True)
#         return split_string(output_string)
#     except Exception:
#         return


def read_register_value(reg, original, instr_type):
    if not reg: return None

    if instr_type.endswith('movb'):
        hex_value = read_memory(reg, 'x/x')
        if hex_value:
            return extract_char(hex_value)

    if reg == '$al':
        return extract_char(reg)

    val = read_memory(reg, 'x/s')
    if not val or 'error' in val:
        return None
    elif val in original:
        return val
    else:
        if val[1:-1] in original:
            return val[1:-1]
        address = read_memory(reg, 'x/a')
        str_val = read_register_value(address, original, instr_type)
        return str_val

def extract_char(addr):
    char_val = gdb.execute('p/c %s' % (addr), to_string=True)
    char_val = char_val.split()
    char_val = char_val[-1]
    return char_val[1:-1]


def get_duplicate_indexes(input_str, target):
    return [i for i, c in enumerate(input_str) if target == c]

import string
def get_alternative_chars(s):
    if s.isdigit():
        return list(string.digits.replace(s, ''))
    else:
        return list(string.ascii_letters.replace(s, ''))


def verify_substr_path(inputstr, val, mid, module, comparisons, instr_count):
    duplicate_idxes = get_duplicate_indexes(inputstr, val)
    alternative_chars = get_alternative_chars(val)

    for i in duplicate_idxes:
        new_str = inputstr
        for j in duplicate_idxes:
            if i != j and j > i:
                c = random.choice(alternative_chars)
                new_str = new_str[:j] + c + new_str[j + 1:]

        comparison = check(new_str, [i, val, mid], module, inputstr, instr_count)
        if comparison and comparison not in comparisons:
            comparisons.append(comparison)
            return comparisons
        continue
    return comparisons


def add_value_to_comparisons(val, mid, inputstr, comparisons, module, instr_count):
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
                comparisons = verify_substr_path(inputstr, c, mid,                 module, comparisons, instr_count)
        else:
            indexes_of_duplicates = [i for i, c in enumerate(inputstr) if val == c]
            if len(indexes_of_duplicates) == 1:
                idx = inputstr.find(val)
                comparisons.append([idx, val, mid])
            else:
                comparisons = verify_substr_path(inputstr, val, mid, module,                 comparisons, instr_count)
    return comparisons


def check(new_str, comparison, module, originalstr, instr_count):
    with open(f'inp.0.txt', 'w+') as f:
        print(new_str, file=f)

    with open(f'comparison.txt', 'w+') as f:
        print(comparison, file=f)

    subprocess.call(['gdb', '--batch-silent', '-ex', "py arg0='%s'" % new_str,
    '-ex', 'py arg1="%s"' % module, '-ex', 'py arg2="%s"' % instr_count, "-x", "check2.py"])

    with open(f'inp.0.txt', 'w+') as f:
        print(originalstr, file=f)

    result = None
    try:
        with open(f'updated_comparisons', 'rb') as f:
            result = jsonpickle.decode(f.read())

        if result and result == comparison: return result
    except FileNotFoundError:
        return result


arg_0 = None
with open(f'inp.0.txt', 'r+') as f:
    arg_0 = f.read().strip()

fnames = get_function_names(arg_0, "a.out")
subprocess.call(['strip', '-s', "a.out"])

debugger = BinaryDebugger(arg_0, 'a.out', fnames)
debugger.event_loop()

