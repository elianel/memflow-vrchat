#pragma once

#define SIG_UNITYPLAYER_TIMEMANAGER "48 8B 0D ? ? ? ? 48 83 C1 70 E8 ? ? ? ? 48 8B 0D ? ? ? ?" // .text section, last 4
#define OFFSET_UNITYPLAYER_TIMEMANAGER_TIMESCALE 0x0FC                                         // float
#define OFFSET_UNITYPLAYER_TIMEMANAGER_DELTATIME 0x0A8                                         // float