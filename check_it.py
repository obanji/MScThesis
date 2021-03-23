import gdb, subprocess, os


# arg_0 = None
# with open(f'inp.0.txt', 'r+') as f:
#     arg_0 = f.read().strip()

flag = None
if os.path.exists('flag.txt'):
    with open(f'flag.txt', 'r+') as f:
        flag = f.read().strip()
        subprocess.call(['gcc', '-g', arg1, flag])
else:
    subprocess.call(['gcc', '-g', arg1])

gdb.execute('file a.out')
# gdb.execute('set logging overwrite on')
# gdb.execute('set logging on')
gdb.execute("set args '%s'" % arg0)
gdb.execute('run')