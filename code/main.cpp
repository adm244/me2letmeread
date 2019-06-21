/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

//IMPORTANT(adm244): SCRATCH VERSION JUST TO GET IT UP WORKING

#define PSAPI_VERSION 1
#include <windows.h>
#include <psapi.h>
#include <assert.h>
#include <stdio.h>

#include "types.h"
#include "detours.cpp"
#include "native_types.h"

internal _BioConversation_NeedToDisplayReplies BioConversation_NeedToDisplayReplies = 0;
internal _BioConversation_IsAmbient BioConversation_IsAmbient = 0;
//internal _BioConversation_GetReplyTextInternal BioConversation_GetReplyText = (_BioConversation_GetReplyText)0x00B35280;

internal bool ShouldReply(BioConversation *conversation)
{
  return (conversation->topicFlags & Topic_Patch_DialogWheelActive);
}

internal bool IsSkipped(BioConversation *conversation)
{
  //NOTE(adm244): there's still some sound clicking and poping
  // is this a patch problem or game itself comes with these bugs?
  
  //NOTE(adm244): fixes infinite-loading bug
  /*BioConversationEntry entry = conversation->entryList[conversation->currentEntryIndex];
  if (entry.flags & Entry_NonTextline) {
    return true;
  }*/
  
  //NOTE(adm244): skipes "empty" replies
  /*if (conversation->currentReplyIndex >= 0) {
    BioConversationEntryReply entryReply = entry.replyList[conversation->currentReplyIndex];
  
    BioString replyText = {0};
    BioConversation_GetReplyText(conversation, &replyText, entryReply.index, 1);
    
    //NOTE(adm244): probably should check if string contains only whitespaces or empty
    if (replyText.length < 3) {
      return true;
    }
  }*/
  
  if (!BioConversation_IsAmbient(conversation) && !ShouldReply(conversation)) {
    bool isSkipped = (conversation->dialogFlags & Dialog_Patch_ManualSkip);
    conversation->dialogFlags &= ~Dialog_Patch_ManualSkip;
    return isSkipped;
  }
  
  return true;
}

internal void SkipNode(BioConversation *conversation)
{
  if ((conversation->topicFlags & Topic_IsVoicePlaying)
    || !(conversation->topicFlags & Topic_Patch_DialogWheelActive)) {
    conversation->dialogFlags |= Dialog_Patch_ManualSkip;
  }
}

internal void *skip_jz_dest_address = 0;
internal void *skip_jnz_dest_address = 0;
internal void *skip_post_jnz_address = 0;

internal void *skip_node_address = 0;
internal void *skip_node_mov_address = 0;

internal __declspec(naked) void IsSkipped_Hook()
{
  _asm {
    push ebx
    push esi
    push edi
    push ebp
    push esp
    
    push ebx
    call IsSkipped
    add esp, 4
    
    pop esp
    pop ebp
    pop edi
    pop esi
    pop ebx
    
    test al, al
    jz dont_skip_dialog
    jmp skip_dialog
    
  dont_skip_dialog:
    jmp [skip_jz_dest_address]
    
  skip_dialog:
    test esi, esi
    jnz jnz_jump
    mov eax, [esp + 20h]
    jmp [skip_post_jnz_address]
    
  jnz_jump:
    jmp [skip_jnz_dest_address]
  }
}

internal __declspec(naked) void SkipNode_Hook()
{
  _asm {
    push ebx
    push esi
    push edi
    push ebp
    push esp
    
    push ecx
    call SkipNode
    pop ecx
    
    pop esp
    pop ebp
    pop edi
    pop esi
    pop ebx
    
    mov eax, [skip_node_mov_address]
    mov eax, [eax]
    push esi
    push edi
    mov edi, ecx
    
    mov edx, skip_node_address
    add edx, 9
    jmp edx
  }
}

internal __declspec(naked) void RepliesActive_Hook()
{
  _asm {
    mov eax, Topic_Patch_DialogWheelActive
    or [esi+1A8h], eax
    
    mov eax, 1
    pop esi
    retn
  }
}

internal __declspec(naked) void RepliesInactive_Hook()
{
  _asm {
    mov eax, Topic_Patch_DialogWheelActive
    not eax
    and [esi+1A8h], eax
    
    xor eax, eax
    pop esi
    retn
  }
}

/*
  TODO(adm244): find these places:
    - UBioConversation::vtable (0x0117CC48)
    - UBioConversation::NeedToDisplayReplies() (0x00B37030)
      - Replies active (0x00B37079)
      - Replies inactive (0x00B370B1)
    - UBioConversation::IsAmbient() (0x00B36FA0)
    - UBioConversation::UpdateConversation() (0x00B32880)
      - JZ instruction (0x00B32E07)
      - JZ destination (0x00B32E64)
      - JNZ instruction (0x00B32E0B)
      - JNZ destination (0x00B32E3A)
      - Detour address at (JNZ - 2) (0x00B32E09)
    - UBioConversation::GetReplyText() (0x00B35280)
      - UBioConversation::GetReplyTextInternal() (0x00B353B0)
    - UBioConversation::SkipNode() (0x00B332C0)
  
  TODO(adm244): verify these types:
    - BioString
    - UBioConversation
    - UBioConversationReply
    - UBioConversationEntry
    - UBioConversationEntryReply
    - UBioConversation_DialogFlags
    - UBioConversation_TopicFlags
    - UBioConversationEntry_Flags
*/

internal BOOL WINAPI DllMain(HMODULE loader, DWORD reason, LPVOID reserved)
{
  if( reason == DLL_PROCESS_ATTACH )
  {
    //TODO(adm244): get addresses by a signature search
    
    // get function pointers for BioConversation object
    void *bio_conversation_vtable = (void *)0x0117CC48;
    BioConversation_NeedToDisplayReplies = (_BioConversation_NeedToDisplayReplies)(*((u32 *)bio_conversation_vtable + 102));
    BioConversation_IsAmbient = (_BioConversation_IsAmbient)(*((u32 *)bio_conversation_vtable + 90));
#ifdef DEBUG
    assert((u64)BioConversation_NeedToDisplayReplies == 0x00B37030);
    assert((u64)BioConversation_IsAmbient == 0x00B36FA0);
#endif
    
    // hook NeedToDisplayReplies function
    void *replies_active_patch_address = (u8 *)BioConversation_NeedToDisplayReplies + 0x49;
    void *replies_inactive_patch_address = (u8 *)BioConversation_NeedToDisplayReplies + 0x81;
    
    WriteDetour(replies_active_patch_address, &RepliesActive_Hook, 2);
    WriteDetour(replies_inactive_patch_address, &RepliesInactive_Hook, 0);
    
    // hook UpdateConversation function
    void *skip_jz_address = (void *)0x00B32E07;
    skip_jz_dest_address = RIPRel8(skip_jz_address, 1);
#ifdef DEBUG
    assert((u64)skip_jz_dest_address == 0x00B32E64);
#endif
    
    void *skip_address = (void *)((u64)skip_jz_address + 2);
    void *skip_jnz_address = (void *)((u64)skip_address + 2);
    skip_jnz_dest_address = RIPRel8(skip_jnz_address, 1);
    skip_post_jnz_address = (void *)((u64)skip_jnz_address + 6);
#ifdef DEBUG
    assert((u64)skip_jnz_address == 0x00B32E0B);
    assert((u64)skip_jnz_dest_address == 0x00B32E3A);
    assert((u64)skip_address == 0x00B32E09);
#endif

    WriteDetour(skip_address, &IsSkipped_Hook, 3);
    
    // hook SkipNode function
    skip_node_address = (void *)0x00B332C0;
    skip_node_mov_address = (void *)(*((u32 *)((u8 *)skip_node_address + 1)));
#ifdef DEBUG
    assert((u64)skip_node_mov_address == 0x0126779C);
#endif
    
    WriteDetour(skip_node_address, &SkipNode_Hook, 4);
  }

  return TRUE;
}
