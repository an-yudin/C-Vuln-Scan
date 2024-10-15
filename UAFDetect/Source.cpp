#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

namespace {
    struct UAFDetectPass : public ModulePass {
        static char ID;
        UAFDetectPass() : ModulePass(ID) {}

        bool runOnModule(Module &M) override {
            auto &context = M.getContext();
            auto voidTy = Type::getVoidTy(context);
            auto int64Ty = Type::getInt64Ty(context);
            auto int8PtrTy = Type::getInt8PtrTy(context);
            auto int32PtrTy = Type::getInt32PtrTy(context);
            IRBuilder<>* builder = new IRBuilder<>(context);

            std::vector<Type*> argsAlloca = {int32PtrTy};
            FunctionCallee logAlloca = M.getOrInsertFunction("logAlloca",FunctionType::get(voidTy, argsAlloca, false));

            std::vector<Type*> argsMalloc = {int8PtrTy, int64Ty};
            FunctionCallee logMalloc = M.getOrInsertFunction("logMalloc", FunctionType::get(voidTy, argsMalloc, false));

            std::vector<Type*> argsFree = {int8PtrTy, int64Ty, int64Ty};
            FunctionCallee logFree = M.getOrInsertFunction("logFree", FunctionType::get(voidTy, argsFree, false));

            std::vector<Type*> argsQuery = {int32PtrTy, int64Ty, int64Ty, int64Ty};
            FunctionCallee logQuery = M.getOrInsertFunction("logQuery", FunctionType::get(voidTy, argsQuery, false));

            for (auto &F : M) {
                for (auto &B : F) {
                    for (auto &I : B) {
                        if (AllocaInst* alloca = dyn_cast<AllocaInst>(&I)) {
                            BitCastInst* bitcast = new BitCastInst(cast<Value>(alloca), int32PtrTy, "a", (&I)->getNextNode());
                            Value* address = cast<Value>(bitcast);
                            std::vector<Value*> argsAlloca = {address};

                            builder->SetInsertPoint((&I)->getNextNode()->getNextNode());
                            builder->CreateCall(logAlloca, argsAlloca, "");
                        }

                        if (CallInst* call = dyn_cast<CallInst>(&I)) {
                            if (call->getCalledFunction()->getName() == "free") {
                                Value* address = call->getOperand(0);
                                Value* ln = ConstantInt::get(int64Ty, -1);
                                Value* col = ConstantInt::get(int64Ty, -1);
                                const DebugLoc &debugLoc = I.getDebugLoc();
                                if (debugLoc) {
                                    ln = ConstantInt::get(int64Ty, (int64_t)I.getDebugLoc().getLine());
                                    col = ConstantInt::get(int64Ty, (int64_t)I.getDebugLoc().getCol());
                                }
                                std::vector<Value*> argsFree = {address, ln, col};

                                builder->SetInsertPoint((&I)->getNextNode());
                                builder->CreateCall(logFree, argsFree, "");
                            }

                            if (call->getCalledFunction()->getName() == "malloc") {
                                Value* address = cast<Value>(call);
                                Value* size = call->getOperand(0);
                                std::vector<Value*> argsMalloc = {address, size};

                                builder->SetInsertPoint((&I)->getNextNode());
                                builder->CreateCall(logMalloc, argsMalloc, "");
                            }

                            if (call->getCalledFunction()->getName() == "calloc") {
                                ConstantInt* num = dyn_cast<ConstantInt>(call->getOperand(0));
                                ConstantInt* size = dyn_cast<ConstantInt>(call->getOperand(1));
                                Value* totalSize = ConstantInt::get(int64Ty, num->getSExtValue() * size->getSExtValue());
                                Value* address = cast<Value>(call);
                                std::vector<Value*> argsMalloc = {address, totalSize};

                                builder->SetInsertPoint((&I)->getNextNode());
                                builder->CreateCall(logMalloc, argsMalloc, "");
                            }
                        }

                        if (LoadInst* load = dyn_cast<LoadInst>(&I)) {
                            Value* loadAddress = load->getPointerOperand();
                            PointerType* loadType = cast<PointerType>(loadAddress->getType());
                            BitCastInst* bitcast = new BitCastInst(loadAddress, int32PtrTy, "l", (&I)->getNextNode());
                            DataLayout* dataLayout = new DataLayout(&M);
                            uint64_t unsignedSize = dataLayout->getTypeStoreSize(loadType->getPointerElementType());
                            Value* address = cast<Value>(bitcast);
                            Value* size = ConstantInt::get(int64Ty, unsignedSize);
                            Value* ln = ConstantInt::get(int64Ty, -1);
                            Value* col = ConstantInt::get(int64Ty, -1);
                            const DebugLoc &debugLoc = I.getDebugLoc();
                            if (debugLoc) {
                                ln = ConstantInt::get(int64Ty, (int64_t)I.getDebugLoc().getLine());
                                col = ConstantInt::get(int64Ty, (int64_t)I.getDebugLoc().getCol());
                            }
                            std::vector<Value*> argsQuery = {address, size, ln, col};

                            builder->SetInsertPoint((&I)->getNextNode()->getNextNode());
                            builder->CreateCall(logQuery, argsQuery, "");
                        }

                        if (StoreInst* store = dyn_cast<StoreInst>(&I)) {
                            Value* storeAddress = store->getPointerOperand();
                            PointerType* storeType = cast<PointerType>(storeAddress->getType());
                            BitCastInst* bitcast = new BitCastInst(storeAddress, int32PtrTy, "l", (&I)->getNextNode());
                            DataLayout* dataLayout = new DataLayout(&M);
                            uint64_t unsignedSize = dataLayout->getTypeStoreSize(storeType->getPointerElementType());
                            Value* address = cast<Value>(bitcast);
                            Value* size = ConstantInt::get(int64Ty, unsignedSize);
                            Value* ln = ConstantInt::get(int64Ty, -1);
                            Value* col = ConstantInt::get(int64Ty, -1);
                            const DebugLoc &debugLoc = I.getDebugLoc();
                            if (debugLoc) {
                                ln = ConstantInt::get(int64Ty, (int64_t)I.getDebugLoc().getLine());
                                col = ConstantInt::get(int64Ty, (int64_t)I.getDebugLoc().getCol());
                            }
                            std::vector<Value*> argsQuery = {address, size, ln, col};

                            builder->SetInsertPoint((&I)->getNextNode()->getNextNode());
                            builder->CreateCall(logQuery, argsQuery, "");
                        }
                    }
                }
            }
            return true;
        }
    };
}

char UAFDetectPass::ID = 0;

static RegisterPass<UAFDetectPass> X("uafpass", "UAF Detection");

