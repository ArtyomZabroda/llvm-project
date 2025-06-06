//===-- Block.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Symbol/Block.h"

#include "lldb/Core/Module.h"
#include "lldb/Core/Section.h"
#include "lldb/Symbol/Function.h"
#include "lldb/Symbol/SymbolFile.h"
#include "lldb/Symbol/VariableList.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"

#include <memory>

using namespace lldb;
using namespace lldb_private;

Block::Block(Function &function, user_id_t function_uid)
    : Block(function_uid, function) {}

Block::Block(lldb::user_id_t uid, SymbolContextScope &parent_scope)
    : UserID(uid), m_parent_scope(parent_scope), m_parsed_block_info(false),
      m_parsed_block_variables(false), m_parsed_child_blocks(false) {}

Block::~Block() = default;

void Block::GetDescription(Stream *s, Function *function,
                           lldb::DescriptionLevel level, Target *target) const {
  *s << "id = " << ((const UserID &)*this);

  size_t num_ranges = m_ranges.GetSize();
  if (num_ranges > 0) {

    addr_t base_addr = LLDB_INVALID_ADDRESS;
    if (target)
      base_addr = function->GetAddress().GetLoadAddress(target);
    if (base_addr == LLDB_INVALID_ADDRESS)
      base_addr = function->GetAddress().GetFileAddress();

    s->Printf(", range%s = ", num_ranges > 1 ? "s" : "");
    for (size_t i = 0; i < num_ranges; ++i) {
      const Range &range = m_ranges.GetEntryRef(i);
      DumpAddressRange(s->AsRawOstream(), base_addr + range.GetRangeBase(),
                       base_addr + range.GetRangeEnd(), 4);
    }
  }

  if (m_inlineInfoSP.get() != nullptr) {
    bool show_fullpaths = (level == eDescriptionLevelVerbose);
    m_inlineInfoSP->Dump(s, show_fullpaths);
  }
}

void Block::Dump(Stream *s, addr_t base_addr, int32_t depth,
                 bool show_context) const {
  if (depth < 0) {
    Block *parent = GetParent();
    if (parent) {
      // We have a depth that is less than zero, print our parent blocks first
      parent->Dump(s, base_addr, depth + 1, show_context);
    }
  }

  s->Printf("%p: ", static_cast<const void *>(this));
  s->Indent();
  *s << "Block" << static_cast<const UserID &>(*this);
  const Block *parent_block = GetParent();
  if (parent_block) {
    s->Printf(", parent = {0x%8.8" PRIx64 "}", parent_block->GetID());
  }
  if (m_inlineInfoSP.get() != nullptr) {
    bool show_fullpaths = false;
    m_inlineInfoSP->Dump(s, show_fullpaths);
  }

  if (!m_ranges.IsEmpty()) {
    *s << ", ranges =";

    size_t num_ranges = m_ranges.GetSize();
    for (size_t i = 0; i < num_ranges; ++i) {
      const Range &range = m_ranges.GetEntryRef(i);
      if (parent_block != nullptr && !parent_block->Contains(range))
        *s << '!';
      else
        *s << ' ';
      DumpAddressRange(s->AsRawOstream(), base_addr + range.GetRangeBase(),
                       base_addr + range.GetRangeEnd(), 4);
    }
  }
  s->EOL();

  if (depth > 0) {
    s->IndentMore();

    if (m_variable_list_sp.get()) {
      m_variable_list_sp->Dump(s, show_context);
    }

    collection::const_iterator pos, end = m_children.end();
    for (pos = m_children.begin(); pos != end; ++pos)
      (*pos)->Dump(s, base_addr, depth - 1, show_context);

    s->IndentLess();
  }
}

Block *Block::FindBlockByID(user_id_t block_id) {
  if (block_id == GetID())
    return this;

  Block *matching_block = nullptr;
  collection::const_iterator pos, end = m_children.end();
  for (pos = m_children.begin(); pos != end; ++pos) {
    matching_block = (*pos)->FindBlockByID(block_id);
    if (matching_block)
      break;
  }
  return matching_block;
}

Block *Block::FindInnermostBlockByOffset(const lldb::addr_t offset) {
  if (!Contains(offset))
    return nullptr;
  for (const BlockSP &block_sp : m_children) {
    if (Block *block = block_sp->FindInnermostBlockByOffset(offset))
      return block;
  }
  return this;
}

void Block::CalculateSymbolContext(SymbolContext *sc) {
  m_parent_scope.CalculateSymbolContext(sc);
  sc->block = this;
}

lldb::ModuleSP Block::CalculateSymbolContextModule() {
  return m_parent_scope.CalculateSymbolContextModule();
}

CompileUnit *Block::CalculateSymbolContextCompileUnit() {
  return m_parent_scope.CalculateSymbolContextCompileUnit();
}

Function *Block::CalculateSymbolContextFunction() {
  return m_parent_scope.CalculateSymbolContextFunction();
}

Block *Block::CalculateSymbolContextBlock() { return this; }

Function &Block::GetFunction() {
  // Blocks always have an enclosing function because their parent is either a
  // function or a block (which has a parent, inductively).
  Function *function = CalculateSymbolContextFunction();
  assert(function);
  return *function;
}

void Block::DumpSymbolContext(Stream *s) {
  GetFunction().DumpSymbolContext(s);
  s->Printf(", Block{0x%8.8" PRIx64 "}", GetID());
}

void Block::DumpAddressRanges(Stream *s, lldb::addr_t base_addr) {
  if (!m_ranges.IsEmpty()) {
    size_t num_ranges = m_ranges.GetSize();
    for (size_t i = 0; i < num_ranges; ++i) {
      const Range &range = m_ranges.GetEntryRef(i);
      DumpAddressRange(s->AsRawOstream(), base_addr + range.GetRangeBase(),
                       base_addr + range.GetRangeEnd(), 4);
    }
  }
}

bool Block::Contains(addr_t range_offset) const {
  return m_ranges.FindEntryThatContains(range_offset) != nullptr;
}

bool Block::Contains(const Block *block) const {
  if (this == block)
    return false; // This block doesn't contain itself...

  // Walk the parent chain for "block" and see if any if them match this block
  const Block *block_parent;
  for (block_parent = block->GetParent(); block_parent != nullptr;
       block_parent = block_parent->GetParent()) {
    if (this == block_parent)
      return true; // One of the parents of "block" is this object!
  }
  return false;
}

bool Block::Contains(const Range &range) const {
  return m_ranges.FindEntryThatContains(range) != nullptr;
}

Block *Block::GetParent() const {
  return m_parent_scope.CalculateSymbolContextBlock();
}

Block *Block::GetContainingInlinedBlock() {
  if (GetInlinedFunctionInfo())
    return this;
  return GetInlinedParent();
}

Block *Block::GetInlinedParent() {
  Block *parent_block = GetParent();
  if (parent_block) {
    if (parent_block->GetInlinedFunctionInfo())
      return parent_block;
    else
      return parent_block->GetInlinedParent();
  }
  return nullptr;
}

Block *Block::GetContainingInlinedBlockWithCallSite(
    const Declaration &find_call_site) {
  Block *inlined_block = GetContainingInlinedBlock();

  while (inlined_block) {
    const auto *function_info = inlined_block->GetInlinedFunctionInfo();

    if (function_info &&
        function_info->GetCallSite().FileAndLineEqual(find_call_site, true))
      return inlined_block;
    inlined_block = inlined_block->GetInlinedParent();
  }
  return nullptr;
}

bool Block::GetRangeContainingOffset(const addr_t offset, Range &range) {
  const Range *range_ptr = m_ranges.FindEntryThatContains(offset);
  if (range_ptr) {
    range = *range_ptr;
    return true;
  }
  range.Clear();
  return false;
}

bool Block::GetRangeContainingAddress(const Address &addr,
                                      AddressRange &range) {
  Function &function = GetFunction();
  if (uint32_t idx = GetRangeIndexContainingAddress(addr); idx != UINT32_MAX) {
    const Range *range_ptr = m_ranges.GetEntryAtIndex(idx);
    assert(range_ptr);

    Address func_addr = function.GetAddress();
    range.GetBaseAddress() =
        Address(func_addr.GetFileAddress() + range_ptr->GetRangeBase(),
                func_addr.GetModule()->GetSectionList());
    range.SetByteSize(range_ptr->GetByteSize());
    return true;
  }
  range.Clear();
  return false;
}

bool Block::GetRangeContainingLoadAddress(lldb::addr_t load_addr,
                                          Target &target, AddressRange &range) {
  Address load_address;
  load_address.SetLoadAddress(load_addr, &target);
  AddressRange containing_range;
  return GetRangeContainingAddress(load_address, containing_range);
}

uint32_t Block::GetRangeIndexContainingAddress(const Address &addr) {
  Function &function = GetFunction();

  const Address &func_addr = function.GetAddress();
  if (addr.GetModule() != func_addr.GetModule())
    return UINT32_MAX;

  const addr_t file_addr = addr.GetFileAddress();
  const addr_t func_file_addr = func_addr.GetFileAddress();
  return m_ranges.FindEntryIndexThatContains(file_addr - func_file_addr);
}

static AddressRange ToAddressRange(const Address &func_addr,
                                   const Block::Range &block_range) {
  assert(func_addr.GetModule());
  return AddressRange(func_addr.GetFileAddress() + block_range.base,
                      block_range.size,
                      func_addr.GetModule()->GetSectionList());
}

bool Block::GetRangeAtIndex(uint32_t range_idx, AddressRange &range) {
  if (range_idx >= m_ranges.GetSize())
    return false;

  Address addr = GetFunction().GetAddress();
  if (!addr.GetModule())
    return false;

  range = ToAddressRange(addr, m_ranges.GetEntryRef(range_idx));
  return true;
}

AddressRanges Block::GetRanges() {
  Address addr = GetFunction().GetAddress();
  if (!addr.GetModule())
    return {};

  AddressRanges ranges;
  for (size_t i = 0, e = m_ranges.GetSize(); i < e; ++i)
    ranges.push_back(ToAddressRange(addr, m_ranges.GetEntryRef(i)));
  return ranges;
}

bool Block::GetStartAddress(Address &addr) {
  Address func_addr = GetFunction().GetAddress();
  if (!func_addr.GetModule() || m_ranges.IsEmpty())
    return false;

  addr = ToAddressRange(func_addr, m_ranges.GetEntryRef(0)).GetBaseAddress();
  return true;
}

void Block::FinalizeRanges() {
  m_ranges.Sort();
  m_ranges.CombineConsecutiveRanges();
}

void Block::AddRange(const Range &range) {
  Block *parent_block = GetParent();
  if (parent_block && !parent_block->Contains(range)) {
    Log *log = GetLog(LLDBLog::Symbols);
    if (log) {
      ModuleSP module_sp(m_parent_scope.CalculateSymbolContextModule());
      Function &function = GetFunction();
      const addr_t function_file_addr = function.GetAddress().GetFileAddress();
      const addr_t block_start_addr = function_file_addr + range.GetRangeBase();
      const addr_t block_end_addr = function_file_addr + range.GetRangeEnd();
      Type *func_type = function.GetType();

      const Declaration &func_decl = func_type->GetDeclaration();
      if (func_decl.GetLine()) {
        LLDB_LOGF(log,
                  "warning: %s:%u block {0x%8.8" PRIx64
                  "} has range[%u] [0x%" PRIx64 " - 0x%" PRIx64
                  ") which is not contained in parent block {0x%8.8" PRIx64
                  "} in function {0x%8.8" PRIx64 "} from %s",
                  func_decl.GetFile().GetPath().c_str(), func_decl.GetLine(),
                  GetID(), (uint32_t)m_ranges.GetSize(), block_start_addr,
                  block_end_addr, parent_block->GetID(), function.GetID(),
                  module_sp->GetFileSpec().GetPath().c_str());
      } else {
        LLDB_LOGF(log,
                  "warning: block {0x%8.8" PRIx64 "} has range[%u] [0x%" PRIx64
                  " - 0x%" PRIx64
                  ") which is not contained in parent block {0x%8.8" PRIx64
                  "} in function {0x%8.8" PRIx64 "} from %s",
                  GetID(), (uint32_t)m_ranges.GetSize(), block_start_addr,
                  block_end_addr, parent_block->GetID(), function.GetID(),
                  module_sp->GetFileSpec().GetPath().c_str());
      }
    }
    parent_block->AddRange(range);
  }
  m_ranges.Append(range);
}

// Return the current number of bytes that this object occupies in memory
size_t Block::MemorySize() const {
  size_t mem_size = sizeof(Block) + m_ranges.GetSize() * sizeof(Range);
  if (m_inlineInfoSP.get())
    mem_size += m_inlineInfoSP->MemorySize();
  if (m_variable_list_sp.get())
    mem_size += m_variable_list_sp->MemorySize();
  return mem_size;
}

BlockSP Block::CreateChild(user_id_t uid) {
  m_children.push_back(std::shared_ptr<Block>(new Block(uid, *this)));
  return m_children.back();
}

void Block::SetInlinedFunctionInfo(const char *name, const char *mangled,
                                   const Declaration *decl_ptr,
                                   const Declaration *call_decl_ptr) {
  m_inlineInfoSP = std::make_shared<InlineFunctionInfo>(name, mangled, decl_ptr,
                                                        call_decl_ptr);
}

VariableListSP Block::GetBlockVariableList(bool can_create) {
  if (!m_parsed_block_variables) {
    if (m_variable_list_sp.get() == nullptr && can_create) {
      m_parsed_block_variables = true;
      SymbolContext sc;
      CalculateSymbolContext(&sc);
      assert(sc.module_sp);
      sc.module_sp->GetSymbolFile()->ParseVariablesForContext(sc);
    }
  }
  return m_variable_list_sp;
}

uint32_t
Block::AppendBlockVariables(bool can_create, bool get_child_block_variables,
                            bool stop_if_child_block_is_inlined_function,
                            const std::function<bool(Variable *)> &filter,
                            VariableList *variable_list) {
  uint32_t num_variables_added = 0;
  VariableList *block_var_list = GetBlockVariableList(can_create).get();
  if (block_var_list) {
    for (const VariableSP &var_sp : *block_var_list) {
      if (filter(var_sp.get())) {
        num_variables_added++;
        variable_list->AddVariable(var_sp);
      }
    }
  }

  if (get_child_block_variables) {
    collection::const_iterator pos, end = m_children.end();
    for (pos = m_children.begin(); pos != end; ++pos) {
      Block *child_block = pos->get();
      if (!stop_if_child_block_is_inlined_function ||
          child_block->GetInlinedFunctionInfo() == nullptr) {
        num_variables_added += child_block->AppendBlockVariables(
            can_create, get_child_block_variables,
            stop_if_child_block_is_inlined_function, filter, variable_list);
      }
    }
  }
  return num_variables_added;
}

uint32_t Block::AppendVariables(bool can_create, bool get_parent_variables,
                                bool stop_if_block_is_inlined_function,
                                const std::function<bool(Variable *)> &filter,
                                VariableList *variable_list) {
  uint32_t num_variables_added = 0;
  VariableListSP variable_list_sp(GetBlockVariableList(can_create));

  bool is_inlined_function = GetInlinedFunctionInfo() != nullptr;
  if (variable_list_sp) {
    for (size_t i = 0; i < variable_list_sp->GetSize(); ++i) {
      VariableSP variable = variable_list_sp->GetVariableAtIndex(i);
      if (filter(variable.get())) {
        num_variables_added++;
        variable_list->AddVariable(variable);
      }
    }
  }

  if (get_parent_variables) {
    if (stop_if_block_is_inlined_function && is_inlined_function)
      return num_variables_added;

    Block *parent_block = GetParent();
    if (parent_block)
      num_variables_added += parent_block->AppendVariables(
          can_create, get_parent_variables, stop_if_block_is_inlined_function,
          filter, variable_list);
  }
  return num_variables_added;
}

SymbolFile *Block::GetSymbolFile() {
  if (ModuleSP module_sp = CalculateSymbolContextModule())
    return module_sp->GetSymbolFile();
  return nullptr;
}

CompilerDeclContext Block::GetDeclContext() {
  if (SymbolFile *sym_file = GetSymbolFile())
    return sym_file->GetDeclContextForUID(GetID());
  return CompilerDeclContext();
}

void Block::SetBlockInfoHasBeenParsed(bool b, bool set_children) {
  m_parsed_block_info = b;
  if (set_children) {
    m_parsed_child_blocks = true;
    collection::const_iterator pos, end = m_children.end();
    for (pos = m_children.begin(); pos != end; ++pos)
      (*pos)->SetBlockInfoHasBeenParsed(b, true);
  }
}

void Block::SetDidParseVariables(bool b, bool set_children) {
  m_parsed_block_variables = b;
  if (set_children) {
    collection::const_iterator pos, end = m_children.end();
    for (pos = m_children.begin(); pos != end; ++pos)
      (*pos)->SetDidParseVariables(b, true);
  }
}

Block *Block::GetSibling() const {
  if (Block *parent_block = GetParent())
    return parent_block->GetSiblingForChild(this);
  return nullptr;
}

// A parent of child blocks can be asked to find a sibling block given
// one of its child blocks
Block *Block::GetSiblingForChild(const Block *child_block) const {
  if (!m_children.empty()) {
    collection::const_iterator pos, end = m_children.end();
    for (pos = m_children.begin(); pos != end; ++pos) {
      if (pos->get() == child_block) {
        if (++pos != end)
          return pos->get();
        break;
      }
    }
  }
  return nullptr;
}
