run:
	# gcc -g subject/duktape/duktape.c -lm
	gcc -g subject/csv/csv.c
	#gcc -g subject/komplott/swizzle.c -ldl
	gdb --batch-silent -x ExecutionTree.py
clean:
	rm a.out
	rm inp.0.txt
	rm gdb.txt
	rm updated_comparisons
	rm tree
	rm check