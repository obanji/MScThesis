
import gdb, subprocess, os

flag = None
if os.path.exists('flag.txt'):
    with open(f'flag.txt', 'r+') as f:
        flag = f.read().strip()
        subprocess.call(['gcc', '-g', arg1, flag])
else:
    subprocess.call(['gcc', '-g', arg1])

gdb.execute('file a.out')
gdb.execute("set args '%s'" % arg0)
gdb.execute('run')

