//===-- lib/Codegen/MachineRegisterInfo.cpp -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation of the MachineRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;

MachineRegisterInfo::MachineRegisterInfo(const TargetRegisterInfo &TRI) {
  VRegInfo.reserve(256);
  UsedPhysRegs.resize(TRI.getNumRegs());
  
  // Create the physreg use/def lists.
  PhysRegUseDefLists = new MachineOperand*[TRI.getNumRegs()];
  memset(PhysRegUseDefLists, 0, sizeof(MachineOperand*)*TRI.getNumRegs());
}

MachineRegisterInfo::~MachineRegisterInfo() {
#ifndef NDEBUG
  for (unsigned i = 0, e = VRegInfo.size(); i != e; ++i)
    assert(VRegInfo[i].second == 0 && "Vreg use list non-empty still?");
#endif
  delete [] PhysRegUseDefLists;
}

/// HandleVRegListReallocation - We just added a virtual register to the
/// VRegInfo info list and it reallocated.  Update the use/def lists info
/// pointers.
void MachineRegisterInfo::HandleVRegListReallocation() {
  // The back pointers for the vreg lists point into the previous vector.
  // Update them to point to their correct slots.
  for (unsigned i = 0, e = VRegInfo.size(); i != e; ++i) {
    MachineOperand *List = VRegInfo[i].second;
    if (!List) continue;
    // Update the back-pointer to be accurate once more.
    List->Contents.Reg.Prev = &VRegInfo[i].second;
  }
}

/// replaceRegWith - Replace all instances of FromReg with ToReg in the
/// machine function.  This is like llvm-level X->replaceAllUsesWith(Y),
/// except that it also changes any definitions of the register as well.
void MachineRegisterInfo::replaceRegWith(unsigned FromReg, unsigned ToReg) {
  assert(FromReg != ToReg && "Cannot replace a reg with itself");

  // TODO: This could be more efficient by bulk changing the operands.
  for (reg_iterator I = reg_begin(FromReg), E = reg_end(); I != E; ) {
    MachineOperand &O = I.getOperand();
    ++I;
    O.setReg(ToReg);
  }
}


/// getVRegDef - Return the machine instr that defines the specified virtual
/// register or null if none is found.  This assumes that the code is in SSA
/// form, so there should only be one definition.
MachineInstr *MachineRegisterInfo::getVRegDef(unsigned Reg) const {
  assert(Reg-TargetRegisterInfo::FirstVirtualRegister < VRegInfo.size() &&
         "Invalid vreg!");
  for (reg_iterator I = reg_begin(Reg), E = reg_end(); I != E; ++I) {
    // Since we are in SSA form, we can stop at the first definition.
    if (I.getOperand().isDef())
      return &*I;
  }
  return 0;
}


#ifndef NDEBUG
void MachineRegisterInfo::dumpUses(unsigned Reg) const {
  for (use_iterator I = use_begin(Reg), E = use_end(); I != E; ++I)
    I.getOperand().getParent()->dump();
}
#endif
