#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"

using namespace std;
using namespace llvm;

#define DEBUG_TYPE "loop"

StringRef *LoopCond = new StringRef("for.cond");
StringRef *LoopBody = new StringRef("for.body");
STATISTIC(LoopCounter, "Counts number of loops greeted");

namespace {

    struct IRArray {
        string arrayName;
        vector<int> index;
    };
    struct IRArray Arr[10][50];

    int Add[10][2], Sub[10][2], Mul[10][2];
    int High = 0, Low = 0;
    int line, outputFlag, antiFlag, flowFlag;

    map<string, string> nameMap;
    vector<string> arrayOrder;

    struct HW : public ModulePass {
        static char ID; // Pass identification, replacement for typeid
        HW() : ModulePass(ID) {}

        void initialize() {
            for(int i = 0; i < 10; i++) {
                for(int j = 0; j < 2; j++) {
                    Add[i][j] = 0;
                    Sub[i][j] = 0;
                    Mul[i][j] = 1;
                }
            }
            outputFlag = 0;
            antiFlag = 0;
            flowFlag = 0;
        }

        void expandLoop() {

            for(int i = 0; i < arrayOrder.size(); i++) {
                Arr[i/2][!(i%2)].arrayName = arrayOrder[i];
            }

            for(int i = 0; i < line; i++) {
                for(int j = 0; j < 2; j++) {
                    for(int k = Low; k < High; k++) {
                        int idx = k*Mul[i][j]+Add[i][j]-Sub[i][j];
                        Arr[i][j].index.push_back(idx);
                    }
                }
            }
        }

        void findAntiDependency() {
            for(int i = 0; i < line; i++) {
                for(int j = 0; j < line; j++) {
                    if(Arr[i][1].arrayName == Arr[j][0].arrayName) {
                        for(int m = 0; m < High; m++) {
                            for(int n = 0; n < High; n++) {
                                if(Arr[i][1].index[m] == Arr[j][0].index[n] && m < n) {
                                    if(!antiFlag) {
                                        errs() << "=====Anti Dependence=====\n";
                                        errs() << "(i = " << m << ", i = " << n << ")\n";
                                        errs() << Arr[i][1].arrayName << " : ";
                                        errs() << 'S' << i+1 << " ---> S" << j+1 << '\n';
                                        antiFlag = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void findOutputDependency() {
            for(int i = 0; i < line; i++) {
                for(int j = i+1; j < line; j++) {
                    if(Arr[i][0].arrayName == Arr[j][0].arrayName) {
                        for(int m = 0; m < High; m++) {
                            for(int n = 0; n < High; n++) {
                                if(Arr[i][0].index[m] == Arr[j][0].index[n]) {
                                    if(!outputFlag) {
                                        errs() << "=====Output Dependence=====\n";
                                        errs() << "(i = " << m << ", i = " << n << ")\n";
                                        errs() << Arr[i][0].arrayName << " : ";
                                        errs() << 'S' << i+1 << " ---> S" << j+1 << '\n';
                                        outputFlag = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void findFlowDependency() {
            for(int i = 0; i < line; i++) {
                for(int j = i+1; j < line; j++) {
                    if(Arr[i][0].arrayName == Arr[j][1].arrayName) {
                        for(int m = 0; m < High; m++) {
                            for(int n = 0; n < High; n++) {
                                if(Arr[i][0].index[m] == Arr[j][1].index[n]) {
                                    if(!flowFlag) {
                                        errs() << "=====Flow Dependence=====\n";
                                        errs() << "(i = " << m << ", i = " << n << ")\n";
                                        errs() << Arr[i][0].arrayName << " : ";
                                        errs() << 'S' << i+1 << " ---> S" << j+1 << '\n';
                                        flowFlag = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void getLoadDef(Value *inv) {    //recursive traverse the IR
            if(Instruction *I = dyn_cast<Instruction>(inv)){   //make sure *inv is an Instruction

                //when you call getName for llvm temp variable such as %1 %2 ... is empty char
                //so when getName is empty char means you must search deeper inside the IR
                if(I->getName() != ""){
                    //errs() << "find it = " << I->getName();
                    //errs() << "\n******************\n";

                    if(I->getOpcode() == Instruction::GetElementPtr) {
                        Value *tmp = I->getOperand(0);
                        //%ArrayName = alloca [20 x i32], align 16
                        string name = tmp->getName();
                        nameMap[I->getName()] = name;
                    }
                }
                else{
                    //recursive to the upper define-use chain
                    for(User::op_iterator OI = (*I).op_begin(), e = (*I).op_end(); OI != e; ++OI) {
                        Value *v = *OI;
                        //errs() << *v << "\n";
                        getLoadDef(v);
                    }
                }
            }
        }

        virtual bool runOnModule(Module &M) {

            initialize();

            for (Module::iterator F = M.begin(); F != M.end(); F++){
                int functionCount = 0;
                //errs() << "Function name : " << F->getName() << "\n";

                for (Function::iterator BB = (*F).begin(); BB != (*F).end(); BB++, functionCount++) {
                    //find the upper bound of the loop
                    if(!BB->getName().find(*LoopCond, 0)) {
                        LoopCounter++;
                        //errs() << "BasicBlock name: " << BB->getName() << " #" << LoopCounter << "\n";
                        for (BasicBlock::iterator itrIns = (*BB).begin(); itrIns != (*BB).end(); itrIns++) {
                            if(!strcmp("icmp", itrIns->getOpcodeName())) {
                                if(ConstantInt* Integer = dyn_cast<ConstantInt>(itrIns->getOperand(1))) {
                                    High = Integer->getZExtValue();
                                    //errs() << "High : " << High << '\n';
                                }
                            }
                        }
                        //errs() << "=====================\n";
                    }

                    if(!BB->getName().find(*LoopBody, 0)) {
                        for (BasicBlock::iterator itrIns = (*BB).begin(); itrIns != (*BB).end(); itrIns++) {
                            if(dyn_cast<Instruction>(itrIns)) {
                                /*
                                if(itrIns->getOpcode() == Instruction::Load) {
                                    Value *tmp = itrIns->getOperand(0);
                                    //errs() << (*tmp) << '\n';
                                    std::string loadItem = tmp->getName();
                                    //errs() << "Index : " << loadItem << '\n';
                                    if(loadItem != "i" && nameMap.find(loadItem) == nameMap.end()) {
                                        getLoadDef(tmp);
                                    }
                                }
                                */

                                if(itrIns->getOpcode() == Instruction::Store) {
                                    //tmp1 = tmp2 in c code.
                                    Value *tmp1 = itrIns->getOperand(0);
                                    Value *tmp2 = itrIns->getOperand(1);

                                    if(!tmp1->hasName()) {
                                        getLoadDef(tmp1);
                                    }

                                    if(Instruction *I = dyn_cast<Instruction>(tmp2)) {
                                        if(I->getOpcode() == Instruction::GetElementPtr) {
                                            Value *tmp = I->getOperand(0);
                                            string name = tmp->getName();
                                            nameMap[I->getName()] = name;
                                        }
                                    }
                                }

                                //record the order of each array appeared for fucutre loop expand.
                                else if(itrIns->getOpcode() == Instruction::GetElementPtr) {
                                    Value *tmp = itrIns->getOperand(0);
                                    std::string name = tmp->getName();
                                    arrayOrder.push_back(name);
                                }

                                else if(itrIns->getOpcode() == Instruction::Sub) {
                                    int currentLine = arrayOrder.size()/2;
                                    int currentLoc = !(arrayOrder.size()%2);

                                    Value *val1 = itrIns->getOperand(1);

                                    //errs() << "Sub Value1 ::" << *val1 << "\n\n";
                                    if(ConstantInt *Int = dyn_cast<ConstantInt>(val1)) {
                                        int value = Int->getZExtValue();
                                        Sub[currentLine][currentLoc] = value;
                                    }
                                }

                                else if(itrIns->getOpcode() == Instruction::Add) {
                                    int currentLine = arrayOrder.size()/2;
                                    int currentLoc = !(arrayOrder.size()%2);

                                    Value *val1 = itrIns->getOperand(1);

                                    if(ConstantInt *Int = dyn_cast<ConstantInt>(val1)) {
                                        int value = Int->getZExtValue();
                                        Add[currentLine][currentLoc] = value;
                                    }
                                }

                                else if(itrIns->getOpcode() == Instruction::Mul) {
                                    int currentLine = arrayOrder.size()/2;
                                    int currentLoc = !(arrayOrder.size()%2);

                                    Value *val1 = itrIns->getOperand(0);

                                    if(ConstantInt *Int = dyn_cast<ConstantInt>(val1)) {
                                        int value = Int->getZExtValue();
                                        //errs() << "Mul value: " << value << '\n';
                                        Mul[currentLine][currentLoc] = value;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            line = arrayOrder.size()/2;

            expandLoop();
            /*
            errs() << "\nLine :" << line << '\n';
            for(int i = 0; i < line; i++) {
                for(int j = 0; j < 2; j++) {
                    errs() << "line: " << i;
                    errs() << ", loc: " << j;
                    errs() << ", name: " << Arr[i][j].arrayName << '\n';
                    for(int k = 0; k < Arr[i][j].index.size(); k++) {
                        errs() << Arr[i][j].index[k] << " ";
                    }
                    errs() << '\n';
                }
            }
            */
            findOutputDependency();
            findFlowDependency();
            findAntiDependency();

            return false;
        }
    };
 }

//initialize identifier
char HW::ID = 0;
//"Demo" is the name of pass
//"Simple demo for assignment 1 " is the explaination of your pass
static RegisterPass<HW> GS("HW", "Simple program to find data dependency");
