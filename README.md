# C-Vuln-Scan
Анализатор работает на базе [LLVM 14.0.6](https://github.com/llvm/llvm-project/releases/tag/llvmorg-14.0.6) (последняя версия до введения Opaque Pointers, возможно использование более ранних версий).

## Сборка LLVM
В `/llvm-project-llvmorg-14.0.6` выполнить:
```
cmake -S llvm -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_PROJECTS='clang'
cmake --build build
```

## Сборка LLVM Pass
Для сборки LLVM Pass следует:
- поместить директорию `/UAFDetect` в `/llvm-project-llvmorg-14.0.6/llvm/lib/Transforms`
- добавить в файл `CMakeLists.txt` в `/llvm-project-llvmorg-14.0.6/llvm/lib/Transforms` строку
```
add_subdirectory(UAFDetect)
```
- выполнить команду сборки LLVM (см. выше)

## Сборка вспомогательных функций
Выполнить генерацию промежуточной формы LLVM:
```
/llvm-project-llvmorg-14.0.6/build/bin/clang -S -emit-llvm funcs.c
```

## Запуск анализатора
Пусть файл для анализа имеет имя `test.c`.
Для проведения анализа следует:
- получить промежуточную форму LLVM
```
/llvm-project-llvmorg-14.0.6/build/bin/clang -S -g -emit-llvm test.c
```
- провести LLVM Pass
```
/llvm-project-llvmorg-14.0.6/build/bin/opt -enable-new-pm=0 -S -load /llvm-project-llvmorg-14.0.6/build/lib/libLLVMUAFDetect.so -uafpass test.ll -o opt.ll
```
- слинковать со вспомогательными функциями
```
/llvm-project-llvmorg-14.0.6/build/bin/llvm-link -S opt.ll funcs.ll -o final_opt.ll
```
- исполнить итоговый LLVM-IR
```
/llvm-project-llvmorg-14.0.6/build/bin/lli final_opt.ll
```
