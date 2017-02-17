//===-- SimplifyCFGPass.cpp ---------------------------------------------===//
//
//                     The LAV Software Verification Tool
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "simplifycfg"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Pass.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"

#include <iostream>
using namespace llvm;

STATISTIC(NumSimpl, "Number of blocks simplified");

namespace {
//  struct VISIBILITY_HIDDEN FCFGSimplifyPass : public FunctionPass {
struct FCFGSimplifyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  FCFGSimplifyPass() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F);
};
}

char FCFGSimplifyPass::ID = 0;
static RegisterPass<FCFGSimplifyPass> X("lavsimplifycfg",
                                        "Simplify the CFG LAV");

namespace lav {
// Public interface to the CFGSimplification pass
FunctionPass *createFCFGSimplificationPass() { return new FCFGSimplifyPass(); }
}

/// ChangeToUnreachable - Insert an unreachable instruction before the specified
/// instruction, making it and the rest of the code in the block dead.
static void ChangeToUnreachable(Instruction *I) {
  BasicBlock *BB = I->getParent();
  // Loop over all of the successors, removing BB's entry from any PHI
  // nodes.
  for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI)
    (*SI)->removePredecessor(BB);

  new UnreachableInst(getGlobalContext(), I);

  // All instructions after this are dead.
  BasicBlock::iterator BBI = I, BBE = BB->end();
  while (BBI != BBE) {
    if (!BBI->use_empty())
      BBI->replaceAllUsesWith(UndefValue::get(BBI->getType()));
    BB->getInstList().erase(BBI++);
  }
}

/// ChangeToCall - Convert the specified invoke into a normal call.
static void ChangeToCall(InvokeInst *II) {
  BasicBlock *BB = II->getParent();
  SmallVector<Value *, 8> Args(II->op_begin() + 3, II->op_end());
  CallInst *NewCall = CallInst::Create(II->getCalledValue(), Args, "", II);
  NewCall->takeName(II);
  NewCall->setCallingConv(II->getCallingConv());
  NewCall->setAttributes(II->getAttributes());
  II->replaceAllUsesWith(NewCall);

  // Follow the call by a branch to the normal destination.
  BranchInst::Create(II->getNormalDest(), II);

  // Update PHI nodes in the unwind destination
  II->getUnwindDest()->removePredecessor(BB);
  BB->getInstList().erase(II);
}

static bool MarkAliveBlocks(BasicBlock *BB,
                            SmallPtrSet<BasicBlock *, 128> &Reachable) {

  SmallVector<BasicBlock *, 128> Worklist;
  Worklist.push_back(BB);
  bool Changed = false;
  while (!Worklist.empty()) {
    BB = Worklist.back();
    Worklist.pop_back();

    if (!Reachable.insert(BB))
      continue;

    // Do a quick scan of the basic block, turning any obviously unreachable
    // instructions into LLVM unreachable insts.  The instruction combining pass
    // canonicalizes unreachable insts into stores to null or undef.
    for (BasicBlock::iterator BBI = BB->begin(), E = BB->end(); BBI != E;
         ++BBI) {
      if (CallInst *CI = dyn_cast<CallInst>(BBI)) {
        if (CI->doesNotReturn()) {
          // If we found a call to a no-return function, insert an unreachable
          // instruction after it.  Make sure there isn't *already* one there
          // though.
          ++BBI;
          if (!isa<UnreachableInst>(BBI)) {
            ChangeToUnreachable(BBI);
            Changed = true;
          }
          break;
        }
      }

      if (StoreInst *SI = dyn_cast<StoreInst>(BBI)) {
        Value *Ptr = SI->getOperand(1);

        if (isa<UndefValue>(Ptr) ||
            (isa<ConstantPointerNull>(Ptr) &&
             cast<PointerType>(Ptr->getType())->getAddressSpace() == 0)) {
          ChangeToUnreachable(SI);
          Changed = true;
          break;
        }
      }
    }

    // Turn invokes that call 'nounwind' functions into ordinary calls.
    if (InvokeInst *II = dyn_cast<InvokeInst>(BB->getTerminator()))
      if (II->doesNotThrow()) {
        ChangeToCall(II);
        Changed = true;
      }

    Changed |= ConstantFoldTerminator(BB);
    for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI)
      Worklist.push_back(*SI);
  }
  return Changed;
}

/// RemoveUnreachableBlocksFromFn - Remove blocks that are not reachable, even
/// if they are in a dead cycle.  Return true if a change was made, false
/// otherwise.
static bool RemoveUnreachableBlocksFromFn(Function &F) {
  SmallPtrSet<BasicBlock *, 128> Reachable;
  bool Changed = MarkAliveBlocks(F.begin(), Reachable);

  // If there are unreachable blocks in the CFG...
  if (Reachable.size() == F.size())
    return Changed;

  assert(Reachable.size() < F.size());
  NumSimpl += F.size() - Reachable.size();

  // Loop over all of the basic blocks that are not reachable, dropping all of
  // their internal references...
  for (Function::iterator BB = ++F.begin(), E = F.end(); BB != E; ++BB) {
    if (Reachable.count(BB))
      continue;

    for (succ_iterator SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI)
      if (Reachable.count(*SI))
        (*SI)->removePredecessor(BB);
    BB->dropAllReferences();
  }

  for (Function::iterator I = ++F.begin(); I != F.end();)
    if (!Reachable.count(I))
      I = F.getBasicBlockList().erase(I);
    else
      ++I;

  return true;
}

/*
/// IterativeSimplifyCFG - Call SimplifyCFG on all the blocks in the function,
/// iterating until no more changes are made.
static bool IterativeSimplifyCFG(Function &F) {
  bool Changed = false;
  bool LocalChange = true;
  while (LocalChange) {
    LocalChange = false;

    // Loop over all of the basic blocks (except the first one) and remove them
    // if they are unneeded...
    //
    for (Function::iterator BBIt = ++F.begin(); BBIt != F.end(); ) {
      if (SimplifyCFG(BBIt++)) {
        LocalChange = true;
        ++NumSimpl;
      }
    }
    Changed |= LocalChange;
  }
  return Changed;
}
*/

/// FoldBlockIntoPredecessor - Folds a basic block into its predecessor if it
/// only has one predecessor, and that predecessor only has one successor.
static void FoldBlockIntoPredecessor(Function &F) {

  for (Function::iterator BBI = ++F.begin(), E = F.end(); BBI != E; ++BBI) {
    //  (outs() << "nova iteracija: 1\n" );

    BasicBlock *BB = BBI;
    if (BB == NULL)
      continue;
    // Merge basic blocks into their predecessor if there is only one distinct
    // pred, and if there is only one distinct successor of the predecessor, and
    // if there are no PHI nodes.
    BasicBlock *OnlyPred = BB->getSinglePredecessor();
    if (!OnlyPred)
      continue;
    //  (outs() << "nova iteracija: 2\n" );

    if (OnlyPred->getTerminator()->getNumSuccessors() != 1)
      continue;
    //  (outs() << "nova iteracija: 3\n" );

    DEBUG(dbgs() << "Merging: " << *BB << "into: " << *OnlyPred);

    // Resolve any PHI nodes at the start of the block.  They are all
    // guaranteed to have exactly one entry if they exist, unless there are
    // multiple duplicate (but guaranteed to be equal) entries for the
    // incoming edges.  This occurs when there are multiple edges from
    // OnlyPred to OnlySucc.
    FoldSingleEntryPHINodes(BB);
    //  (outs() << "nova iteracija: 4\n" );

    // Delete the unconditional branch from the predecessor...
    OnlyPred->getInstList().pop_back();

    // Make all PHI nodes that referred to BB now refer to Pred as their
    // source...
    BB->replaceAllUsesWith(OnlyPred);

    // Move all definitions in the successor to the predecessor...
    OnlyPred->getInstList().splice(OnlyPred->end(), BB->getInstList());

    std::string OldName = BB->getName();

    // Erase basic block from the function...
    BB->eraseFromParent();

    // Inherit predecessor's name if it exists...
    if (!OldName.empty() && !OnlyPred->hasName())
      OnlyPred->setName(OldName);
    //  (outs() << "nova iteracija: 10\n" );

    BBI = OnlyPred;
    //  return OnlyPred;
  }
}

// It is possible that we may require multiple passes over the code to fully
// simplify the CFG.
//
bool FCFGSimplifyPass::runOnFunction(Function &F) {
  //  bool EverChanged =
  RemoveUnreachableBlocksFromFn(F);
  FoldBlockIntoPredecessor(F);
  /*  EverChanged |= IterativeSimplifyCFG(F);
    
    // If neither pass changed anything, we're done.
    if (!EverChanged) return false;
  
    // IterativeSimplifyCFG can (rarely) make some loops dead.  If this happens,
    // RemoveUnreachableBlocksFromFn is needed to nuke them, which means we
    should
    // iterate between the two optimizations.  We structure the code like this
    to
    // avoid reruning IterativeSimplifyCFG if the second pass of
    // RemoveUnreachableBlocksFromFn doesn't do anything.
    if (!RemoveUnreachableBlocksFromFn(F))
      return true;
    
    do {
      EverChanged = IterativeSimplifyCFG(F);
      EverChanged |= RemoveUnreachableBlocksFromFn(F);
    } while (EverChanged);
   */
  return true;
}
