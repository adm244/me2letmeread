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

#ifndef _NATIVE_TYPES_H_
#define _NATIVE_TYPES_H_

#define assert_size(obj, size) static_assert(sizeof(obj) == size, "Size of " #obj " should be " #size);

enum BioConversation_DialogFlags {
  //Dialog_Skip = 0x4,
  Dialog_Ambient = 0x80,
  //Dialog_IsSpeaking = 0x100,
  Dialog_Patch_ManualSkip = 0x40000000,
};

enum BioConversation_TopicFlags {
  Topic_IsVoicePlaying = 0x10,
  Topic_Patch_DialogWheelActive = 0x40000000,
};

enum BioConversationEntry_Flags {
  Entry_NonTextline = 0x2,
};

#pragma pack(1)

struct BioString {
  u16 *text; // 0x0
  u32 length; // 0x4
  u32 capacity; //??? 0x8
}; // 0xC
assert_size(BioString, 0xC)

struct BioConversationReply {
  u32 textRefId; // 0x0
  u16 *text; // 0x4
  u32 unk8;
  u32 unkC;
  u32 unk10;
  i32 conditionalFunc; // 0x14
  u32 unk18;
  i32 stateTransition; // 0x1C
  u32 unk20;
  u32 exportId; // 0x24
  u32 unk28;
  u32 unk2C; // flags?
  u32 unk30;
  u32 unk34;
  u32 unk38;
  u32 cameraIntimacy; // 0x3C
  u32 ignoreBodyGestures; // 0x40
  u32 *entryIndexList; // 0x44
  u32 unk48;
  u32 unk4C;
  u32 unk50;
  u32 listenerIndex;
  u32 unk58;
}; // 0x5C
assert_size(BioConversationReply, 0x5C)

struct BioConversationEntryReply {
  u32 index; // 0x0
  u32 textRefId; // 0x4
  u32 unk8;
  u32 unkC;
  u32 unk10;
  // 0x0 - Defualt, 0x5 - Investigate
  u32 category; // 0x14
}; // 0x18
assert_size(BioConversationEntryReply, 0x18)

//NOTE(adm244): same as BioConversationReply, but with a BioString appended?
struct BioConversationEntry {
  u32 textRefId; // 0x0
  BioString text; // 0x4
  u8 unk10[0x24-0x10];
  u32 exportId; // 0x24
  u32 unk28;
  u32 flags; // 0x2C
  u8 unk30[0x44-0x30];
  BioConversationEntryReply *replyList; // 0x44
  u8 unk48[0x58-0x48];
  i32 skippable; // 0x58
  BioString unk5C;
}; // 0x68
assert_size(BioConversationEntry, 0x68)

struct BioConversation {
  void *vtable;
  /*u8 unk04[0x48-0x4];
  BioConversationEntry *entryList; // 0x48
  u32 unk4C;
  u32 unk50;
  BioConversationReply *replyList; // 0x54
  u8 unk58[0x90-0x58];
  i32 currentEntryIndex; // 0x90
  u32 unk94;
  u32 unk98;
  u32 unk9C;
  u32 unkA0;
  void *owner; // 0xA4
  void *player; // 0xA8
  void *speaker; // 0xAC
  u32 unkB0;
  void *listener; // 0xB4
  u32 unkB8[3];
  i32 currentReplyIndex; // 0xC4
  void *kismetSequence; // 0xC8
  u32 unkCC[2];
  u32 topicFlags; // 0xD4
  u32 unkD8[30];
  u32 dialogFlags; // 0x150
  u8 unk154[0x1FC-0x154];*/
  u8 unk04[0x48 - 0x4];
  BioConversationEntry *entryList; // 0x48
  u32 unk4C;
  u32 unk50;
  BioConversationReply *replyList; // 0x54
  u8 unk58[0x8C - 0x58];
  i32 currentEntryIndex; // 0x8C
  u32 unk90;
  void *unk94;
  i32 repliesCount; // 0x98
  u8 unk9C[0xC0 - 0x9C];
  i32 currentReplyIndex; // 0xC0
  u32 unkC4;
  u32 unkC8;
  u32 dialogFlags; // 0xCC
  u8 unkD0[0x1A8 - 0xD0];
  u32 topicFlags; // 0x1A8
  u8 unk1AC[0x238 - 0x1AC];
}; // 0x238?
assert_size(BioConversation, 0x238)

#pragma pack(pop)

typedef bool (__thiscall *_BioConversation_NeedToDisplayReplies)(BioConversation *conversation);
typedef bool (__thiscall *_BioConversation_IsAmbient)(BioConversation *conversation);
typedef BioString * (__fastcall *_BioConversation_GetReplyText_Internal)
(int unk, int replyIndex, BioConversation *conversation, BioString *dest);

#endif
