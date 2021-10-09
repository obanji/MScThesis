
import gdb, subprocess, os

flag = None
if os.path.exists('flag.txt'):
    with open(f'flag.txt', 'r+') as f:
        flag = f.read().strip()
        subprocess.call(['gcc', '-g', arg0, flag])
else:
    subprocess.call(['gcc', '-g', arg0])

arg_0 = None
with open(f'inp.0.txt', 'r+') as f:
    arg_0 = f.read().strip()

gdb.execute('file a.out')
gdb.execute("set args '%s'" % arg_0)
gdb.execute('run')

