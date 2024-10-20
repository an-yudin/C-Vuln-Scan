llvm_root=../llvm-project-llvmorg-14.0.6
counter=1

$llvm_root/build/bin/clang -S -emit-llvm funcs.c
$llvm_root/build/bin/clang -S -emit-llvm io.c

for filename in juliet_uaf/*.c; do
	echo $counter " / " $filename
	$llvm_root/build/bin/clang -S -g -emit-llvm $filename -D INCLUDEMAIN -o compiled.ll
	$llvm_root/build/bin/opt -enable-new-pm=0 -S -load $llvm_root/build/lib/libLLVMUAFDetect.so -uafpass compiled.ll -o opted.ll
	$llvm_root/build/bin/llvm-link -S opted.ll io.ll -o linked1.ll
	$llvm_root/build/bin/llvm-link -S linked1.ll funcs.ll -o linked2.ll
	$llvm_root/build/bin/lli linked2.ll
	counter=$((counter+1))
	echo '\n\n'
done

rm -f funcs.ll io.ll compiled.ll opted.ll linked1.ll linked2.ll

