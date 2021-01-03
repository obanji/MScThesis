run:
	gcc -g subject/lisp/lisp.c -lm
	#gcc -g subject/calc/calc_parse.c
	#gcc -g subject/komplott/swizzle.c -ldl
	gdb --batch-silent -x miner.py
clean:
	rm tree
	rm a.out
	rm gdb.txt