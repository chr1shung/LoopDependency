#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;

#define DEBUG_TYPE "loop"

StringRef *LoopBody = new StringRef("for.body");
STATISTIC(LoopCounter, "Counts number of loops greeted");

namespace {

    struct Demo : public ModulePass {
        static char ID; // Pass identification, replacement for typeid
        Demo() : ModulePass(ID) {}

        int originalModuleSize;
        void getLoadDef(Value *inv){    //recursive traverse the IR
            if(Instruction *I = dyn_cast<Instruction>(inv)){   //make sure *inv is an Instruction

                //when you call getName for llvm temp variable such as %1 %2 ... is empty char
                //so when getName is empty char means you must search deeper inside the IR
                if(I->getName() != ""){
                    errs() << "find it = " << I->getName();
                    errs() << "\n******************\n";
                }
                else{
                    //recursive to the upper define-use chain
                    for(User::op_iterator OI = (*I).op_begin(),e = (*I).op_end();OI!=e;++OI){
                        Value *v=*OI;
                        errs() <<*v<<"\n";
                        getLoadDef(v);
                    }
                }
            }
        }

        virtual bool runOnModule(Module &M) {
            // Demo 1 : How to find Instruction
            // errs() << "Demo 1 : How to find Instruction" << "\n\n";
            // whole *.bc is a module,module's iterator F is class of a function
            for (Module::iterator F = M.begin(); F != M.end(); F++){

                errs() << "Function name : " << F->getName() << "\n\n";
                // function's iterator BB is class of a basicblock
                for (Function::iterator BB = (*F).begin(); BB != (*F).end(); BB++){

                    if(!(BB->getName().find(*LoopBody, 0))) {

                        LoopCounter++;
                        errs() << "BasicBlock name: " << BB->getName() << " #" << LoopCounter << "\n\n";

                        for (BasicBlock::iterator itrIns = (*BB).begin(); itrIns != (*BB).end(); itrIns++){

                            if(const StoreInst *SI =dyn_cast<StoreInst>(itrIns)){
                                errs()<< "---------------------------\n";
                                errs()<< "Instruction : ";
                                errs()<< (*itrIns)<<"\n";
                                //D-U chain's iterator is class of a Value,but you can take it as a instruction class
                                //if you want to use it as a instruction just cast<Instruction>
                                for(User::op_iterator OI = (*itrIns).op_begin(),e = (*itrIns).op_end();OI!=e;++OI){
                                    Value *v=*OI;
                                    errs() <<*v<<"\n";
                                    //if(v->getName() != ""){
                                    if( v->hasName() ){
                                        errs() << v->getName()<<"\n";
                                    }
                                    else{
                                        if(dyn_cast<Instruction>(v)){
                                            getLoadDef(v);
                                        }
                                    }
                                }
                                errs()<< "---------------------------\n\n";
                            }  
                            /*
                            // basicblock's iterator itrIns is class of inst
                            if(dyn_cast<Instruction>(itrIns)){
                                if(strcmp(itrIns->getOpcodeName(),"add") == 0){
                                    errs() << "add:  \n";

                                    Value *val0 = itrIns->getOperand(0);
                                    Value *val1 = itrIns->getOperand(1);

                                    errs() << "Add Value0 ::" << *val0 << "\n";
                                    errs() << "Add Value1 ::" << *val1 << "\n\n";

                                    // errs() << itrIns->getOperand(0)->getName()<<"\n";
                                }
                                else if(strcmp(itrIns->getOpcodeName(), "sub") == 0) {
                                    errs() << "sub:  \n";

                                    Value *val0 = itrIns->getOperand(0);
                                    Value *val1 = itrIns->getOperand(1);

                                    errs() << "Sub Value0 ::" << *val0 << "\n";
                                    errs() << "Sub Value1 ::" << *val1 << "\n\n";

                                    // errs() << "Yooooo " << itrIns->getOperand(0)->getName()<<"\n";
                                }
                            }
                            */
                        }
                    }
                }
            }

            /*
             //Demo 2:
            //use dyn_cast to verify target instruction whether a storeins or not
            errs() <<"\n\n\n\n\nDemo 2 : use dyn_cast to verify instruction whether a storeins or not" <<"\n";

            //whole *.bc is a module,module's iterator F is class of a function
            for (Module::iterator F = M.begin(); F != M.end(); F++){
                // function's iterator BB is class of a basicblock
                for (Function::iterator BB = (*F).begin(); BB != (*F).end(); BB++){
                    // basicblock's iterator itrIns is class of inst
                    for (BasicBlock::iterator itrIns = (*BB).begin(); itrIns != (*BB).end(); itrIns++){

                        if(const StoreInst *SI =dyn_cast<StoreInst>(itrIns)){
                            errs()<< "---------------------------\n";
                            errs()<< "Instruction : ";
                            errs()<< (*itrIns)<<"\n";
                            //D-U chain's iterator is class of a Value,but you can take it as a instruction class
                            //if you want to use it as a instruction just cast<Instruction>
                            for(User::op_iterator OI = (*itrIns).op_begin(),e = (*itrIns).op_end();OI!=e;++OI){
                                Value *v=*OI;
                                errs() <<*v<<"\n";
                                //if(v->getName() != ""){
                                    if( v->hasName() ){
                                        errs() << v->getName()<<"\n";
                                    }
                                    else{
                                        if(dyn_cast<Instruction>(v)){
                                        getLoadDef(v);
                                    }
                                }
                            }
                            errs()<< "---------------------------\n";
                        }
                    }
                }
            }
            */

            return false;
        }
    };
 }

//initialize identifier
char Demo::ID = 0;
//"Demo" is the name of pass
//"Simple demo for assignment 1 " is the explaination of your pass
static RegisterPass<Demo> GS("Demo", "Simple demo for assignment 1");
