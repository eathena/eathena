#include "baseparam.h"
#include "showmsg.h"


#include "clif.h"
#include "packetdb.h"


// new packetdb needs different sections for sending and receiving
// receiving packets are keyed by command and contain format and (recv)processor
// sending packets are keyed by (send)processor and contain format and command
// this would elliminate the necessity of any PACKETVER define and packet_len_table
// (which is useless without the format)






class packet_ver
{
	packet_cmd		cmd[MAX_PACKET_DB];
public:
	unsigned short	connect_cmd;

	packet_ver()	{}
	~packet_ver()	{}

	packet_cmd& operator[](size_t i)
	{
#if defined(CHECK_BOUNDS)
		if(i>=MAX_PACKET_DB)
			return cmd[0];
#endif//defined(CHECK_BOUNDS)
		return cmd[i];
	}
};

class _packet_db
{
	packet_ver	vers[MAX_PACKET_VER+1];
public:
	int default_ver;

	_packet_db()
		: default_ver(MAX_PACKET_VER)
	{
		 memset(vers,0,sizeof(vers)); //!!
	}

	~_packet_db()	{}

	packet_ver& operator[](size_t i)
	{
#if defined(CHECK_BOUNDS)
		if(i>MAX_PACKET_VER)
			return vers[default_ver];
#endif//defined(CHECK_BOUNDS)
		return vers[i];
	}
} packet_db;

const packet_cmd& packet(uint packet_ver, uint cmd)
{
	return packet_db[packet_ver][cmd];
}
uint packet_connect(uint packet_ver)
{
	return packet_db[packet_ver].connect_cmd;
}






///////////////////////////////////////////////////////////////////////////////
// パケットデータベース読み込み
int packetdb_readdb(void)
{	// set defaults for packet_db ver 0..18

	///////////////////////////////////////////////////////////////////////////
	// the default packet used for connecting to the server
	packet_db[0].connect_cmd = 0x72;
	packet_db[0][0x0000] = packet_cmd(10);
	packet_db[0][0x0064] = packet_cmd(55);
	packet_db[0][0x0065] = packet_cmd(17);
	packet_db[0][0x0066] = packet_cmd(3);
	packet_db[0][0x0067] = packet_cmd(37);
	packet_db[0][0x0068] = packet_cmd(46);
	packet_db[0][0x0069] = packet_cmd(-1);
	packet_db[0][0x006a] = packet_cmd(23);
	packet_db[0][0x006b] = packet_cmd(-1);
	packet_db[0][0x006c] = packet_cmd(3);
	packet_db[0][0x006d] = packet_cmd(108);
	packet_db[0][0x006e] = packet_cmd(3);
	packet_db[0][0x006f] = packet_cmd(2);
	packet_db[0][0x0070] = packet_cmd(3);
	packet_db[0][0x0071] = packet_cmd(28);
	packet_db[0][0x0072] = packet_cmd(19,clif_parse_WantToConnection,2,6,10,14,18);
	packet_db[0][0x0073] = packet_cmd(11);
	packet_db[0][0x0074] = packet_cmd(3);
	packet_db[0][0x0075] = packet_cmd(-1);
	packet_db[0][0x0076] = packet_cmd(9);
	packet_db[0][0x0077] = packet_cmd(5);
	packet_db[0][0x0078] = packet_cmd(52);
	packet_db[0][0x0079] = packet_cmd(51);
	packet_db[0][0x007a] = packet_cmd(56);
	packet_db[0][0x007b] = packet_cmd(58);
	packet_db[0][0x007c] = packet_cmd(41);
	packet_db[0][0x007d] = packet_cmd(2,clif_parse_LoadEndAck,0);
	packet_db[0][0x007e] = packet_cmd(6,clif_parse_TickSend,2);
	packet_db[0][0x007f] = packet_cmd(6);
	packet_db[0][0x0080] = packet_cmd(7);
	packet_db[0][0x0081] = packet_cmd(3);
	packet_db[0][0x0082] = packet_cmd(2);
	packet_db[0][0x0083] = packet_cmd(2);
	packet_db[0][0x0084] = packet_cmd(2);
	packet_db[0][0x0085] = packet_cmd(5,clif_parse_WalkToXY,2);
	packet_db[0][0x0086] = packet_cmd(16);
	packet_db[0][0x0087] = packet_cmd(12);
	packet_db[0][0x0088] = packet_cmd(10);
	packet_db[0][0x0089] = packet_cmd(7,clif_parse_ActionRequest,2,6);
	packet_db[0][0x008a] = packet_cmd(29);
	packet_db[0][0x008b] = packet_cmd(2);
	packet_db[0][0x008c] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[0][0x008d] = packet_cmd(-1);
	packet_db[0][0x008e] = packet_cmd(-1);
	packet_db[0][0x008f] = packet_cmd(0);
	packet_db[0][0x0090] = packet_cmd(7,clif_parse_NpcClicked,2);
	packet_db[0][0x0091] = packet_cmd(22);
	packet_db[0][0x0092] = packet_cmd(28);
	packet_db[0][0x0093] = packet_cmd(2);
	packet_db[0][0x0094] = packet_cmd(6,clif_parse_GetCharNameRequest,2);
	packet_db[0][0x0095] = packet_cmd(30);
	packet_db[0][0x0096] = packet_cmd(-1,clif_parse_Wis,2,4,28);
	packet_db[0][0x0097] = packet_cmd(-1);
	packet_db[0][0x0098] = packet_cmd(3);
	packet_db[0][0x0099] = packet_cmd(-1,clif_parse_GMmessage,2,4);
	packet_db[0][0x009a] = packet_cmd(-1);
	packet_db[0][0x009b] = packet_cmd(5,clif_parse_ChangeDir,2,4);
	packet_db[0][0x009c] = packet_cmd(9);
	packet_db[0][0x009d] = packet_cmd(17);
	packet_db[0][0x009e] = packet_cmd(17);
	packet_db[0][0x009f] = packet_cmd(6,clif_parse_TakeItem,2);
	packet_db[0][0x00a0] = packet_cmd(23);
	packet_db[0][0x00a1] = packet_cmd(6);
	packet_db[0][0x00a2] = packet_cmd(6,clif_parse_DropItem,2,4);
	packet_db[0][0x00a3] = packet_cmd(-1);
	packet_db[0][0x00a4] = packet_cmd(-1);
	packet_db[0][0x00a5] = packet_cmd(-1);
	packet_db[0][0x00a6] = packet_cmd(-1);
	packet_db[0][0x00a7] = packet_cmd(8,clif_parse_UseItem,2,4);
	packet_db[0][0x00a8] = packet_cmd(7);
	packet_db[0][0x00a9] = packet_cmd(6,clif_parse_EquipItem,2,4);
	packet_db[0][0x00aa] = packet_cmd(7);
	packet_db[0][0x00ab] = packet_cmd(4,clif_parse_UnequipItem,2);
	packet_db[0][0x00ac] = packet_cmd(7);
	packet_db[0][0x00ad] = packet_cmd(0);
	packet_db[0][0x00ae] = packet_cmd(-1);
	packet_db[0][0x00af] = packet_cmd(6);
	packet_db[0][0x00b0] = packet_cmd(8);
	packet_db[0][0x00b1] = packet_cmd(8);
	packet_db[0][0x00b2] = packet_cmd(3,clif_parse_Restart,2);
	packet_db[0][0x00b3] = packet_cmd(3);
	packet_db[0][0x00b4] = packet_cmd(-1);
	packet_db[0][0x00b5] = packet_cmd(6);
	packet_db[0][0x00b6] = packet_cmd(6);
	packet_db[0][0x00b7] = packet_cmd(-1);
	packet_db[0][0x00b8] = packet_cmd(7,clif_parse_NpcSelectMenu,2,6);
	packet_db[0][0x00b9] = packet_cmd(6,clif_parse_NpcNextClicked,2);
	packet_db[0][0x00ba] = packet_cmd(2);
	packet_db[0][0x00bb] = packet_cmd(5,clif_parse_StatusUp,2,4);
	packet_db[0][0x00bc] = packet_cmd(6);
	packet_db[0][0x00bd] = packet_cmd(44);
	packet_db[0][0x00be] = packet_cmd(5);
	packet_db[0][0x00bf] = packet_cmd(3,clif_parse_Emotion,2);
	packet_db[0][0x00c0] = packet_cmd(7);
	packet_db[0][0x00c1] = packet_cmd(2,clif_parse_HowManyConnections,0);
	packet_db[0][0x00c2] = packet_cmd(6);
	packet_db[0][0x00c3] = packet_cmd(8);
	packet_db[0][0x00c4] = packet_cmd(6);
	packet_db[0][0x00c5] = packet_cmd(7,clif_parse_NpcBuySellSelected,2,6);
	packet_db[0][0x00c6] = packet_cmd(-1);
	packet_db[0][0x00c7] = packet_cmd(-1);
	packet_db[0][0x00c8] = packet_cmd(-1,clif_parse_NpcBuyListSend,2,4);
	packet_db[0][0x00c9] = packet_cmd(-1,clif_parse_NpcSellListSend,2,4);
	packet_db[0][0x00ca] = packet_cmd(3);
	packet_db[0][0x00cb] = packet_cmd(3);
	packet_db[0][0x00cc] = packet_cmd(6,clif_parse_GMKick,2);
	packet_db[0][0x00cd] = packet_cmd(3);
	packet_db[0][0x00ce] = packet_cmd(2,clif_parse_GMKillAll,0);
	packet_db[0][0x00cf] = packet_cmd(27,clif_parse_PMIgnore,2,26);
	packet_db[0][0x00d0] = packet_cmd(3,clif_parse_PMIgnoreAll,2);
	packet_db[0][0x00d1] = packet_cmd(4);
	packet_db[0][0x00d2] = packet_cmd(4);
	packet_db[0][0x00d3] = packet_cmd(2,clif_parse_PMIgnoreList,0);
	packet_db[0][0x00d4] = packet_cmd(-1);
	packet_db[0][0x00d5] = packet_cmd(-1,clif_parse_CreateChatRoom,2,4,6,7,15);
	packet_db[0][0x00d6] = packet_cmd(3);
	packet_db[0][0x00d7] = packet_cmd(-1);
	packet_db[0][0x00d8] = packet_cmd(6);
	packet_db[0][0x00d9] = packet_cmd(14,clif_parse_ChatAddMember,2,6);
	packet_db[0][0x00da] = packet_cmd(3);
	packet_db[0][0x00db] = packet_cmd(-1);
	packet_db[0][0x00dc] = packet_cmd(28);
	packet_db[0][0x00dd] = packet_cmd(29);
	packet_db[0][0x00de] = packet_cmd(-1,clif_parse_ChatRoomStatusChange,2,4,6,7,15);
	packet_db[0][0x00df] = packet_cmd(-1);
	packet_db[0][0x00e0] = packet_cmd(30,clif_parse_ChangeChatOwner,2,6);
	packet_db[0][0x00e1] = packet_cmd(30);
	packet_db[0][0x00e2] = packet_cmd(26,clif_parse_KickFromChat,2);
	packet_db[0][0x00e3] = packet_cmd(2,clif_parse_ChatLeave,0);
	packet_db[0][0x00e4] = packet_cmd(6,clif_parse_TradeRequest,2);
	packet_db[0][0x00e5] = packet_cmd(26);
	packet_db[0][0x00e6] = packet_cmd(3,clif_parse_TradeAck,2);
	packet_db[0][0x00e7] = packet_cmd(3);
	packet_db[0][0x00e8] = packet_cmd(8,clif_parse_TradeAddItem,2,4);
	packet_db[0][0x00e9] = packet_cmd(19);
	packet_db[0][0x00ea] = packet_cmd(5);
	packet_db[0][0x00eb] = packet_cmd(2,clif_parse_TradeOk,0);
	packet_db[0][0x00ec] = packet_cmd(3);
	packet_db[0][0x00ed] = packet_cmd(2,clif_parse_TradeCancel,0);
	packet_db[0][0x00ee] = packet_cmd(2);
	packet_db[0][0x00ef] = packet_cmd(2,clif_parse_TradeCommit,0);
	packet_db[0][0x00f0] = packet_cmd(3);
	packet_db[0][0x00f1] = packet_cmd(2);
	packet_db[0][0x00f2] = packet_cmd(6);
	packet_db[0][0x00f3] = packet_cmd(8,clif_parse_MoveToKafra,2,4);
	packet_db[0][0x00f4] = packet_cmd(21);
	packet_db[0][0x00f5] = packet_cmd(8,clif_parse_MoveFromKafra,2,4);
	packet_db[0][0x00f6] = packet_cmd(8);
	packet_db[0][0x00f7] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[0][0x00f8] = packet_cmd(2);
	packet_db[0][0x00f9] = packet_cmd(26,clif_parse_CreateParty,2);
	packet_db[0][0x00fa] = packet_cmd(3);
	packet_db[0][0x00fb] = packet_cmd(-1);
	packet_db[0][0x00fc] = packet_cmd(6,clif_parse_PartyInvite,2);
	packet_db[0][0x00fd] = packet_cmd(27);
	packet_db[0][0x00fe] = packet_cmd(30);
	packet_db[0][0x00ff] = packet_cmd(10,clif_parse_ReplyPartyInvite,2,6);
	packet_db[0][0x0100] = packet_cmd(2,clif_parse_LeaveParty,0);
	packet_db[0][0x0101] = packet_cmd(6);
	packet_db[0][0x0102] = packet_cmd(6,clif_parse_PartyChangeOption,2,4);
	packet_db[0][0x0103] = packet_cmd(30,clif_parse_RemovePartyMember,2,6);
	packet_db[0][0x0104] = packet_cmd(79);
	packet_db[0][0x0105] = packet_cmd(31);
	packet_db[0][0x0106] = packet_cmd(10);
	packet_db[0][0x0107] = packet_cmd(10);
	packet_db[0][0x0108] = packet_cmd(-1,clif_parse_PartyMessage,2,4);
	packet_db[0][0x0109] = packet_cmd(-1);
	packet_db[0][0x010a] = packet_cmd(4);
	packet_db[0][0x010b] = packet_cmd(6);
	packet_db[0][0x010c] = packet_cmd(6);
	packet_db[0][0x010d] = packet_cmd(2);
	packet_db[0][0x010e] = packet_cmd(11);
	packet_db[0][0x010f] = packet_cmd(-1);
	packet_db[0][0x0110] = packet_cmd(10);
	packet_db[0][0x0111] = packet_cmd(39);
	packet_db[0][0x0112] = packet_cmd(4,clif_parse_SkillUp,2);
	packet_db[0][0x0113] = packet_cmd(10,clif_parse_UseSkillToId,2,4,6);
	packet_db[0][0x0114] = packet_cmd(31);
	packet_db[0][0x0115] = packet_cmd(35);
	packet_db[0][0x0116] = packet_cmd(10,clif_parse_UseSkillToPos,2,4,6,8);
	packet_db[0][0x0117] = packet_cmd(18);
	packet_db[0][0x0118] = packet_cmd(2,clif_parse_StopAttack,0);
	packet_db[0][0x0119] = packet_cmd(13);
	packet_db[0][0x011a] = packet_cmd(15);
	packet_db[0][0x011b] = packet_cmd(20,clif_parse_UseSkillMap,2,4);
	packet_db[0][0x011c] = packet_cmd(68);
	packet_db[0][0x011d] = packet_cmd(2,clif_parse_RequestMemo,0);
	packet_db[0][0x011e] = packet_cmd(3);
	packet_db[0][0x011f] = packet_cmd(16);
	packet_db[0][0x0120] = packet_cmd(6);
	packet_db[0][0x0121] = packet_cmd(14);
	packet_db[0][0x0122] = packet_cmd(-1);
	packet_db[0][0x0123] = packet_cmd(-1);
	packet_db[0][0x0124] = packet_cmd(21);
	packet_db[0][0x0125] = packet_cmd(8);
	packet_db[0][0x0126] = packet_cmd(8,clif_parse_PutItemToCart,2,4);
	packet_db[0][0x0127] = packet_cmd(8,clif_parse_GetItemFromCart,2,4);
	packet_db[0][0x0128] = packet_cmd(8,clif_parse_MoveFromKafraToCart,2,4);
	packet_db[0][0x0129] = packet_cmd(8,clif_parse_MoveToKafraFromCart,2,4);
	packet_db[0][0x012a] = packet_cmd(2,clif_parse_RemoveOption,0);
	packet_db[0][0x012b] = packet_cmd(2);
	packet_db[0][0x012c] = packet_cmd(3);
	packet_db[0][0x012d] = packet_cmd(4);
	packet_db[0][0x012e] = packet_cmd(2,clif_parse_CloseVending,0);
	packet_db[0][0x012f] = packet_cmd(-1);
	packet_db[0][0x0130] = packet_cmd(6,clif_parse_VendingListReq,2);
	packet_db[0][0x0131] = packet_cmd(86);
	packet_db[0][0x0132] = packet_cmd(6);
	packet_db[0][0x0133] = packet_cmd(-1);
	packet_db[0][0x0134] = packet_cmd(-1,clif_parse_PurchaseReq,2,4,8);
	packet_db[0][0x0135] = packet_cmd(7);
	packet_db[0][0x0136] = packet_cmd(-1);
	packet_db[0][0x0137] = packet_cmd(6);
	packet_db[0][0x0138] = packet_cmd(3);
	packet_db[0][0x0139] = packet_cmd(16);
	packet_db[0][0x013a] = packet_cmd(4);
	packet_db[0][0x013b] = packet_cmd(4);
	packet_db[0][0x013c] = packet_cmd(4);
	packet_db[0][0x013d] = packet_cmd(6);
	packet_db[0][0x013e] = packet_cmd(24);
	packet_db[0][0x013f] = packet_cmd(26,clif_parse_GM_Monster_Item,2);
	packet_db[0][0x0140] = packet_cmd(22,clif_parse_MapMove,2,18,20);
	packet_db[0][0x0141] = packet_cmd(14);
	packet_db[0][0x0142] = packet_cmd(6);
	packet_db[0][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[0][0x0144] = packet_cmd(23);
	packet_db[0][0x0145] = packet_cmd(19);
	packet_db[0][0x0146] = packet_cmd(6,clif_parse_NpcCloseClicked,2);
	packet_db[0][0x0147] = packet_cmd(39);
	packet_db[0][0x0148] = packet_cmd(8);
	packet_db[0][0x0149] = packet_cmd(9,clif_parse_GMReqNoChat,2,6,7);
	packet_db[0][0x014a] = packet_cmd(6);
	packet_db[0][0x014b] = packet_cmd(27);
	packet_db[0][0x014c] = packet_cmd(-1);
	packet_db[0][0x014d] = packet_cmd(2,clif_parse_GuildCheckMaster,0);
	packet_db[0][0x014e] = packet_cmd(6);
	packet_db[0][0x014f] = packet_cmd(6,clif_parse_GuildRequestInfo,2);
	packet_db[0][0x0150] = packet_cmd(110);
	packet_db[0][0x0151] = packet_cmd(6,clif_parse_GuildRequestEmblem,2);
	packet_db[0][0x0152] = packet_cmd(-1);
	packet_db[0][0x0153] = packet_cmd(-1,clif_parse_GuildChangeEmblem,2,4);
	packet_db[0][0x0154] = packet_cmd(-1);
	packet_db[0][0x0155] = packet_cmd(-1,clif_parse_GuildChangeMemberPosition,2);
	packet_db[0][0x0156] = packet_cmd(-1);
	packet_db[0][0x0157] = packet_cmd(6);
	packet_db[0][0x0158] = packet_cmd(-1);
	packet_db[0][0x0159] = packet_cmd(54,clif_parse_GuildLeave,2,6,10,14);
	packet_db[0][0x015a] = packet_cmd(66);
	packet_db[0][0x015b] = packet_cmd(54,clif_parse_GuildExplusion,2,6,10,14);
	packet_db[0][0x015c] = packet_cmd(90);
	packet_db[0][0x015d] = packet_cmd(42,clif_parse_GuildBreak,2);
	packet_db[0][0x015e] = packet_cmd(6);
	packet_db[0][0x015f] = packet_cmd(42);
	packet_db[0][0x0160] = packet_cmd(-1);
	packet_db[0][0x0161] = packet_cmd(-1,clif_parse_GuildChangePositionInfo,2);
	packet_db[0][0x0162] = packet_cmd(-1);
	packet_db[0][0x0163] = packet_cmd(-1);
	packet_db[0][0x0164] = packet_cmd(-1);
	packet_db[0][0x0165] = packet_cmd(30,clif_parse_CreateGuild,6);
	packet_db[0][0x0166] = packet_cmd(-1);
	packet_db[0][0x0167] = packet_cmd(3);
	packet_db[0][0x0168] = packet_cmd(14,clif_parse_GuildInvite,2);
	packet_db[0][0x0169] = packet_cmd(3);
	packet_db[0][0x016a] = packet_cmd(30);
	packet_db[0][0x016b] = packet_cmd(10,clif_parse_GuildReplyInvite,2,6);
	packet_db[0][0x016c] = packet_cmd(43);
	packet_db[0][0x016d] = packet_cmd(14);
	packet_db[0][0x016e] = packet_cmd(186,clif_parse_GuildChangeNotice,2,6,66);
	packet_db[0][0x016f] = packet_cmd(182);
	packet_db[0][0x0170] = packet_cmd(14,clif_parse_GuildRequestAlliance,2);
	packet_db[0][0x0171] = packet_cmd(30);
	packet_db[0][0x0172] = packet_cmd(10,clif_parse_GuildReplyAlliance,2,6);
	packet_db[0][0x0173] = packet_cmd(3);
	packet_db[0][0x0174] = packet_cmd(-1);
	packet_db[0][0x0175] = packet_cmd(6);
	packet_db[0][0x0176] = packet_cmd(106);
	packet_db[0][0x0177] = packet_cmd(-1);
	packet_db[0][0x0178] = packet_cmd(4,clif_parse_ItemIdentify,2);
	packet_db[0][0x0179] = packet_cmd(5);
	packet_db[0][0x017a] = packet_cmd(4,clif_parse_UseCard,2);
	packet_db[0][0x017b] = packet_cmd(-1);
	packet_db[0][0x017c] = packet_cmd(6,clif_parse_InsertCard,2,4);
	packet_db[0][0x017d] = packet_cmd(7);
	packet_db[0][0x017e] = packet_cmd(-1,clif_parse_GuildMessage,2,4);
	packet_db[0][0x017f] = packet_cmd(-1);
	packet_db[0][0x0180] = packet_cmd(6,clif_parse_GuildOpposition,2);
	packet_db[0][0x0181] = packet_cmd(3);
	packet_db[0][0x0182] = packet_cmd(106);
	packet_db[0][0x0183] = packet_cmd(10,clif_parse_GuildDelAlliance,2,6);
	packet_db[0][0x0184] = packet_cmd(10);
	packet_db[0][0x0185] = packet_cmd(34);
	packet_db[0][0x0186] = packet_cmd(0);
	packet_db[0][0x0187] = packet_cmd(6);
	packet_db[0][0x0188] = packet_cmd(8);
	packet_db[0][0x0189] = packet_cmd(4);
	packet_db[0][0x018a] = packet_cmd(4,clif_parse_QuitGame,0);
	packet_db[0][0x018b] = packet_cmd(4);
	packet_db[0][0x018c] = packet_cmd(29);
	packet_db[0][0x018d] = packet_cmd(-1);
	packet_db[0][0x018e] = packet_cmd(10,clif_parse_ProduceMix,2,4,6,8);
	packet_db[0][0x018f] = packet_cmd(6);
	packet_db[0][0x0190] = packet_cmd(90,clif_parse_UseSkillToPos,2,4,6,8,10);
	packet_db[0][0x0191] = packet_cmd(86);
	packet_db[0][0x0192] = packet_cmd(24);
	packet_db[0][0x0193] = packet_cmd(6,clif_parse_SolveCharName,2);
	packet_db[0][0x0194] = packet_cmd(30);
	packet_db[0][0x0195] = packet_cmd(102);
	packet_db[0][0x0196] = packet_cmd(8);
	packet_db[0][0x0197] = packet_cmd(4,clif_parse_ResetChar,2);
	packet_db[0][0x0198] = packet_cmd(8);
	packet_db[0][0x0199] = packet_cmd(4);
	packet_db[0][0x019a] = packet_cmd(14);
	packet_db[0][0x019b] = packet_cmd(10);
	packet_db[0][0x019c] = packet_cmd(-1,clif_parse_LGMmessage,2,4);
	packet_db[0][0x019d] = packet_cmd(6,clif_parse_GMHide,0);
	packet_db[0][0x019e] = packet_cmd(2);
	packet_db[0][0x019f] = packet_cmd(6,clif_parse_CatchPet,2);
	packet_db[0][0x01a0] = packet_cmd(3);
	packet_db[0][0x01a1] = packet_cmd(3,clif_parse_PetMenu,2);
	packet_db[0][0x01a2] = packet_cmd(35);
	packet_db[0][0x01a3] = packet_cmd(5);
	packet_db[0][0x01a4] = packet_cmd(11);
	packet_db[0][0x01a5] = packet_cmd(26,clif_parse_ChangePetName,2);
	packet_db[0][0x01a6] = packet_cmd(-1);
	packet_db[0][0x01a7] = packet_cmd(4,clif_parse_SelectEgg,2);
	packet_db[0][0x01a8] = packet_cmd(4);
	packet_db[0][0x01a9] = packet_cmd(6,clif_parse_SendEmotion,2);
	packet_db[0][0x01aa] = packet_cmd(10);
	packet_db[0][0x01ab] = packet_cmd(12);
	packet_db[0][0x01ac] = packet_cmd(6);
	packet_db[0][0x01ad] = packet_cmd(-1);
	packet_db[0][0x01ae] = packet_cmd(4,clif_parse_SelectArrow,2);
	packet_db[0][0x01af] = packet_cmd(4,clif_parse_ChangeCart,2);
	packet_db[0][0x01b0] = packet_cmd(11);
	packet_db[0][0x01b1] = packet_cmd(7);
	packet_db[0][0x01b2] = packet_cmd(-1,clif_parse_OpenVending,2,4,84,85);
	packet_db[0][0x01b3] = packet_cmd(67);
	packet_db[0][0x01b4] = packet_cmd(12);
	packet_db[0][0x01b5] = packet_cmd(18);
	packet_db[0][0x01b6] = packet_cmd(114);
	packet_db[0][0x01b7] = packet_cmd(6);
	packet_db[0][0x01b8] = packet_cmd(3);
	packet_db[0][0x01b9] = packet_cmd(6);
	packet_db[0][0x01ba] = packet_cmd(26);
	packet_db[0][0x01bb] = packet_cmd(26,clif_parse_Shift,2);
	packet_db[0][0x01bc] = packet_cmd(26);
	packet_db[0][0x01bd] = packet_cmd(26,clif_parse_Recall,2);
	packet_db[0][0x01be] = packet_cmd(2);
	packet_db[0][0x01bf] = packet_cmd(3);
	packet_db[0][0x01c0] = packet_cmd(2);
	packet_db[0][0x01c1] = packet_cmd(14);
	packet_db[0][0x01c2] = packet_cmd(10);
	packet_db[0][0x01c3] = packet_cmd(-1);
	packet_db[0][0x01c4] = packet_cmd(22);
	packet_db[0][0x01c5] = packet_cmd(22);
	packet_db[0][0x01c6] = packet_cmd(4);
	packet_db[0][0x01c7] = packet_cmd(2);
	packet_db[0][0x01c8] = packet_cmd(13);
	packet_db[0][0x01c9] = packet_cmd(97);
	packet_db[0][0x01ca] = packet_cmd(3);
	packet_db[0][0x01cb] = packet_cmd(9);
	packet_db[0][0x01cc] = packet_cmd(9);
	packet_db[0][0x01cd] = packet_cmd(30);
	packet_db[0][0x01ce] = packet_cmd(6,clif_parse_AutoSpell,2);
	packet_db[0][0x01cf] = packet_cmd(28);
	packet_db[0][0x01d0] = packet_cmd(8);
	packet_db[0][0x01d1] = packet_cmd(14);
	packet_db[0][0x01d2] = packet_cmd(10);
	packet_db[0][0x01d3] = packet_cmd(35);
	packet_db[0][0x01d4] = packet_cmd(6);
	packet_db[0][0x01d5] = packet_cmd(-1,clif_parse_NpcStringInput,2,4,8);
	packet_db[0][0x01d6] = packet_cmd(4);
	packet_db[0][0x01d7] = packet_cmd(11);
	packet_db[0][0x01d8] = packet_cmd(54);
	packet_db[0][0x01d9] = packet_cmd(53);
	packet_db[0][0x01da] = packet_cmd(60);
	packet_db[0][0x01db] = packet_cmd(2);
	packet_db[0][0x01dc] = packet_cmd(-1);
	packet_db[0][0x01dd] = packet_cmd(47);
	packet_db[0][0x01de] = packet_cmd(33);
	packet_db[0][0x01df] = packet_cmd(6,clif_parse_GMReqNoChatCount,2);
	packet_db[0][0x01e0] = packet_cmd(30);
	packet_db[0][0x01e1] = packet_cmd(8);
	packet_db[0][0x01e2] = packet_cmd(34);
	packet_db[0][0x01e3] = packet_cmd(14);
	packet_db[0][0x01e4] = packet_cmd(2);
	packet_db[0][0x01e5] = packet_cmd(6);
	packet_db[0][0x01e6] = packet_cmd(26);
	packet_db[0][0x01e7] = packet_cmd(2,clif_parse_NoviceDoriDori,0);
	packet_db[0][0x01e8] = packet_cmd(28,clif_parse_CreateParty2,2);
	packet_db[0][0x01e9] = packet_cmd(81);
	packet_db[0][0x01ea] = packet_cmd(6);
	packet_db[0][0x01eb] = packet_cmd(10);
	packet_db[0][0x01ec] = packet_cmd(26);
	packet_db[0][0x01ed] = packet_cmd(2,clif_parse_NoviceExplosionSpirits,0);
	packet_db[0][0x01ee] = packet_cmd(-1);
	packet_db[0][0x01ef] = packet_cmd(-1);
	packet_db[0][0x01f0] = packet_cmd(-1);
	packet_db[0][0x01f1] = packet_cmd(-1);
	packet_db[0][0x01f2] = packet_cmd(20);
	packet_db[0][0x01f3] = packet_cmd(10);
	packet_db[0][0x01f4] = packet_cmd(32);
	packet_db[0][0x01f5] = packet_cmd(9);
	packet_db[0][0x01f6] = packet_cmd(34);
	packet_db[0][0x01f7] = packet_cmd(14);
	packet_db[0][0x01f8] = packet_cmd(2);
	packet_db[0][0x01f9] = packet_cmd(6,clif_parse_ReqAdopt,5);
	packet_db[0][0x01fa] = packet_cmd(48);
	packet_db[0][0x01fb] = packet_cmd(56);
	packet_db[0][0x01fc] = packet_cmd(-1);
	packet_db[0][0x01fd] = packet_cmd(4,clif_parse_RepairItem,2);
	packet_db[0][0x01fe] = packet_cmd(5);
	packet_db[0][0x01ff] = packet_cmd(10);
	packet_db[0][0x0200] = packet_cmd(26);
	packet_db[0][0x0201] = packet_cmd(-1);
	packet_db[0][0x0202] = packet_cmd(26,clif_parse_FriendsListAdd,2);
	packet_db[0][0x0203] = packet_cmd(10,clif_parse_FriendsListRemove,2,6);
	packet_db[0][0x0204] = packet_cmd(18);
	packet_db[0][0x0205] = packet_cmd(26);
	packet_db[0][0x0206] = packet_cmd(11);
	packet_db[0][0x0207] = packet_cmd(34);
	packet_db[0][0x0208] = packet_cmd(14,clif_parse_FriendsListReply,2,6,10);
	packet_db[0][0x0209] = packet_cmd(36);
	packet_db[0][0x020a] = packet_cmd(10);
	packet_db[0][0x020b] = packet_cmd(0);
	packet_db[0][0x020c] = packet_cmd(0);
	packet_db[0][0x020d] = packet_cmd(-1);
	packet_db[0][0x020e] = packet_cmd(24);
	packet_db[0][0x020f] = packet_cmd(10);
	packet_db[0][0x0210] = packet_cmd(22);
	packet_db[0][0x0211] = packet_cmd(0);
	packet_db[0][0x0212] = packet_cmd(26);
	packet_db[0][0x0213] = packet_cmd(26);
	packet_db[0][0x0214] = packet_cmd(42);
	packet_db[0][0x0215] = packet_cmd(-1);
	packet_db[0][0x0216] = packet_cmd(-1);
	packet_db[0][0x0217] = packet_cmd(2);
	packet_db[0][0x0218] = packet_cmd(2);
	packet_db[0][0x0219] = packet_cmd(282);
	packet_db[0][0x021a] = packet_cmd(282);
	packet_db[0][0x021b] = packet_cmd(10);
	packet_db[0][0x021c] = packet_cmd(10);
	packet_db[0][0x021d] = packet_cmd(-1);
	packet_db[0][0x021e] = packet_cmd(-1);
	packet_db[0][0x021f] = packet_cmd(66);
	packet_db[0][0x0220] = packet_cmd(10);
	packet_db[0][0x0221] = packet_cmd(-1);
	packet_db[0][0x0222] = packet_cmd(-1);
	packet_db[0][0x0223] = packet_cmd(8);
	packet_db[0][0x0224] = packet_cmd(10);
	packet_db[0][0x0225] = packet_cmd(2);
	packet_db[0][0x0226] = packet_cmd(282);
	packet_db[0][0x0227] = packet_cmd(18);
	packet_db[0][0x0228] = packet_cmd(18);
	packet_db[0][0x0229] = packet_cmd(15);
	packet_db[0][0x022a] = packet_cmd(58);
	packet_db[0][0x022b] = packet_cmd(57);
	packet_db[0][0x022c] = packet_cmd(64);
	packet_db[0][0x022d] = packet_cmd(5);
	packet_db[0][0x022e] = packet_cmd(69);
	packet_db[0][0x022f] = packet_cmd(0);
	packet_db[0][0x0230] = packet_cmd(12);
	packet_db[0][0x0231] = packet_cmd(26);
	packet_db[0][0x0232] = packet_cmd(9);
	packet_db[0][0x0233] = packet_cmd(11);
	packet_db[0][0x0234] = packet_cmd(-1);
	packet_db[0][0x0235] = packet_cmd(-1);
	packet_db[0][0x0236] = packet_cmd(10);
	packet_db[0][0x0237] = packet_cmd(2, clif_parse_RankingPk, 0);	// shows top 10 slayers in the server
	packet_db[0][0x0238] = packet_cmd(282);
	packet_db[0][0x0239] = packet_cmd(11);
	packet_db[0][0x023d] = packet_cmd(-1);
	packet_db[0][0x023e] = packet_cmd(4);
	packet_db[0][0x023f] = packet_cmd(2, clif_parse_RefreshMailBox, 0);
	packet_db[0][0x0240] = packet_cmd(-1);
	packet_db[0][0x0241] = packet_cmd(6, clif_parse_ReadMail, 2);
	packet_db[0][0x0242] = packet_cmd(-1);
	packet_db[0][0x0243] = packet_cmd(6, clif_parse_DeleteMail, 2);
	packet_db[0][0x0244] = packet_cmd(6, clif_parse_MailGetAppend, 2);
	packet_db[0][0x0245] = packet_cmd(3);
	packet_db[0][0x0246] = packet_cmd(4, clif_parse_MailWinOpen, 2);
	packet_db[0][0x0247] = packet_cmd(8, clif_parse_SendMailSetAppend, 2, 4);
	packet_db[0][0x0248] = packet_cmd(-1,clif_parse_SendMail,2,4,28,69);
	packet_db[0][0x0249] = packet_cmd(3);
	packet_db[0][0x024a] = packet_cmd(70);
	packet_db[0][0x024b] = packet_cmd(4);
	packet_db[0][0x024c] = packet_cmd(8);
	packet_db[0][0x024d] = packet_cmd(12);
	packet_db[0][0x024e] = packet_cmd(4);
	packet_db[0][0x024f] = packet_cmd(10);
	packet_db[0][0x0250] = packet_cmd(3);
	packet_db[0][0x0251] = packet_cmd(32);
	packet_db[0][0x0252] = packet_cmd(-1);
	packet_db[0][0x0253] = packet_cmd(3);
	packet_db[0][0x0254] = packet_cmd(3);//feelsaveok,0
	packet_db[0][0x0255] = packet_cmd(5);
	packet_db[0][0x0256] = packet_cmd(5);
	packet_db[0][0x0257] = packet_cmd(8);
	packet_db[0][0x0258] = packet_cmd(2);
	packet_db[0][0x0259] = packet_cmd(3);
	packet_db[0][0x025a] = packet_cmd(-1);
	packet_db[0][0x025b] = packet_cmd(-1);
	packet_db[0][0x025c] = packet_cmd(4);
	packet_db[0][0x025d] = packet_cmd(-1);
	packet_db[0][0x025e] = packet_cmd(4);

	//0x025f max
	///////////////////////////////////////////////////////////////////////////
	// init packet version 5 and lower
	packet_db[1] = packet_db[0];
	packet_db[1][0x0196] = packet_cmd(9);
	///////////////////////////////////////////////////////////////////////////
	packet_db[2] = packet_db[1];
	packet_db[0][0x0078] = packet_cmd(54);
	packet_db[0][0x0079] = packet_cmd(53);
	packet_db[0][0x007a] = packet_cmd(58);
	packet_db[0][0x007b] = packet_cmd(60);
	///////////////////////////////////////////////////////////////////////////
	packet_db[3] = packet_db[2];
	///////////////////////////////////////////////////////////////////////////
	packet_db[4] = packet_db[3];
	///////////////////////////////////////////////////////////////////////////
	packet_db[5] = packet_db[4];
	///////////////////////////////////////////////////////////////////////////
	// packet version 6 (2004-07-07)
	packet_db[6] = packet_db[5];
	packet_db[6].connect_cmd = 0x72;
	packet_db[6][0x0072] = packet_cmd(22,clif_parse_WantToConnection,5,9,13,17,21);
	packet_db[6][0x0085] = packet_cmd(8,clif_parse_WalkToXY,5);
	packet_db[6][0x00a7] = packet_cmd(13,clif_parse_UseItem,5,9);
	packet_db[6][0x0113] = packet_cmd(15,clif_parse_UseSkillToId,4,9,11);
	packet_db[6][0x0116] = packet_cmd(15,clif_parse_UseSkillToPos,4,9,11,13);
	packet_db[6][0x0190] = packet_cmd(95,clif_parse_UseSkillToPos,4,9,11,13,15);
	///////////////////////////////////////////////////////////////////////////
	// packet version 7 (2004-07-13)
	packet_db[7] = packet_db[6];
	packet_db[7].connect_cmd = 0x72;
	packet_db[7][0x0072] = packet_cmd(39,clif_parse_WantToConnection,12,22,30,34,38);
	packet_db[7][0x0085] = packet_cmd(9,clif_parse_WalkToXY,6);
	packet_db[7][0x009b] = packet_cmd(13,clif_parse_ChangeDir,5,12);
	packet_db[7][0x009f] = packet_cmd(10,clif_parse_TakeItem,6);
	packet_db[7][0x00a7] = packet_cmd(17,clif_parse_UseItem,6,13);
	packet_db[7][0x0113] = packet_cmd(19,clif_parse_UseSkillToId,7,9,15);
	packet_db[7][0x0116] = packet_cmd(19,clif_parse_UseSkillToPos,7,9,15,17);
	packet_db[7][0x0190] = packet_cmd(99,clif_parse_UseSkillToPos,7,9,15,17,19);
	///////////////////////////////////////////////////////////////////////////
	// packet version 8 (2004-07-26)
	packet_db[8] = packet_db[7];
	packet_db[8].connect_cmd = 0x7e;
	packet_db[8][0x0072] = packet_cmd(14,clif_parse_DropItem,5,12);
	packet_db[8][0x007e] = packet_cmd(33,clif_parse_WantToConnection,12,18,24,28,32);
	packet_db[8][0x0085] = packet_cmd(20,clif_parse_UseSkillToId,7,12,16);
	packet_db[8][0x0089] = packet_cmd(15,clif_parse_GetCharNameRequest,11);
	packet_db[8][0x008c] = packet_cmd(23,clif_parse_UseSkillToPos,3,6,17,21);
	packet_db[8][0x0094] = packet_cmd(10,clif_parse_TakeItem,6);
	packet_db[8][0x009b] = packet_cmd(6,clif_parse_WalkToXY,3);
	packet_db[8][0x009f] = packet_cmd(13,clif_parse_ChangeDir,5,12);
	packet_db[8][0x00a2] = packet_cmd(103,clif_parse_UseSkillToPos,3,6,17,21,23);
	packet_db[8][0x00a7] = packet_cmd(12,clif_parse_SolveCharName,8);
	packet_db[8][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[8][0x00f5] = packet_cmd(17,clif_parse_UseItem,6,12);
	packet_db[8][0x00f7] = packet_cmd(10,clif_parse_TickSend,6);
	packet_db[8][0x0113] = packet_cmd(16,clif_parse_MoveToKafra,5,12);
	packet_db[8][0x0116] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[8][0x0190] = packet_cmd(26,clif_parse_MoveFromKafra,10,22);
	packet_db[8][0x0193] = packet_cmd(9,clif_parse_ActionRequest,3,8);
	///////////////////////////////////////////////////////////////////////////
	// packet version 9 (2004-08-09)(2004-08-16)(2004-08-17)
	packet_db[9] = packet_db[8];
	packet_db[9].connect_cmd = 0x7e;
	packet_db[9][0x0072] = packet_cmd(17,clif_parse_DropItem,8,15);
	packet_db[9][0x007e] = packet_cmd(37,clif_parse_WantToConnection,9,21,28,32,36);
	packet_db[9][0x0085] = packet_cmd(26,clif_parse_UseSkillToId,11,18,22);
	packet_db[9][0x0089] = packet_cmd(12,clif_parse_GetCharNameRequest,8);
	packet_db[9][0x008c] = packet_cmd(40,clif_parse_UseSkillToPos,5,15,29,38);
	packet_db[9][0x0094] = packet_cmd(13,clif_parse_TakeItem,9);
	packet_db[9][0x009b] = packet_cmd(15,clif_parse_WalkToXY,12);
	packet_db[9][0x009f] = packet_cmd(12,clif_parse_ChangeDir,7,11);
	packet_db[9][0x00a2] = packet_cmd(120,clif_parse_UseSkillToPos,5,15,29,38,40);
	packet_db[9][0x00a7] = packet_cmd(11,clif_parse_SolveCharName,7);
	packet_db[9][0x00f5] = packet_cmd(24,clif_parse_UseItem,9,20);
	packet_db[9][0x00f7] = packet_cmd(13,clif_parse_TickSend,9);
	packet_db[9][0x0113] = packet_cmd(23,clif_parse_MoveToKafra,5,19);
	packet_db[9][0x0190] = packet_cmd(26,clif_parse_MoveFromKafra,11,22);
	packet_db[9][0x0193] = packet_cmd(18,clif_parse_ActionRequest,7,17);
	packet_db[9][0x0211] = packet_cmd(0);
	packet_db[9][0x0212] = packet_cmd(26);
	packet_db[9][0x0213] = packet_cmd(26);
	packet_db[9][0x0214] = packet_cmd(42);
	packet_db[9][0x020f] = packet_cmd(10);
	packet_db[9][0x0210] = packet_cmd(22);
	///////////////////////////////////////////////////////////////////////////
	// packet version 10 (2004-09-06)
	packet_db[10] = packet_db[9];
	packet_db[10].connect_cmd = 0xf5;
	packet_db[10][0x0072] = packet_cmd(20,clif_parse_UseItem,9,20);
	packet_db[10][0x007e] = packet_cmd(19,clif_parse_MoveToKafra,3,15);
	packet_db[10][0x0085] = packet_cmd(23,clif_parse_ActionRequest,9,22);
	packet_db[10][0x0089] = packet_cmd(9,clif_parse_WalkToXY,6);
	packet_db[10][0x008c] = packet_cmd(105,clif_parse_UseSkillToPos,10,14,18,23,25);
	packet_db[10][0x0094] = packet_cmd(17,clif_parse_DropItem,6,15);
	packet_db[10][0x009b] = packet_cmd(14,clif_parse_GetCharNameRequest,10);
	packet_db[10][0x009f] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[10][0x00a2] = packet_cmd(14,clif_parse_SolveCharName,10);
	packet_db[10][0x00a7] = packet_cmd(25,clif_parse_UseSkillToPos,10,14,18,23);
	packet_db[10][0x00f3] = packet_cmd(10,clif_parse_ChangeDir,4,9);
	packet_db[10][0x00f5] = packet_cmd(34,clif_parse_WantToConnection,7,15,25,29,33);
	packet_db[10][0x00f7] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[10][0x0113] = packet_cmd(11,clif_parse_TakeItem,7);
	packet_db[10][0x0116] = packet_cmd(11,clif_parse_TickSend,7);
	packet_db[10][0x0190] = packet_cmd(22,clif_parse_UseSkillToId,9,15,18);
	packet_db[10][0x0193] = packet_cmd(17,clif_parse_MoveFromKafra,3,13);
	///////////////////////////////////////////////////////////////////////////
	// packet version 11 (2004-09-21)
	packet_db[11] = packet_db[10];
	packet_db[11].connect_cmd = 0xf5;
	packet_db[11][0x0072] = packet_cmd(18,clif_parse_UseItem,10,14);
	packet_db[11][0x007e] = packet_cmd(25,clif_parse_MoveToKafra,6,21);
	packet_db[11][0x0085] = packet_cmd(9,clif_parse_ActionRequest,3,8);
	packet_db[11][0x0089] = packet_cmd(14,clif_parse_WalkToXY,11);
	packet_db[11][0x008c] = packet_cmd(109,clif_parse_UseSkillToPos,16,20,23,27,29);
	packet_db[11][0x0094] = packet_cmd(19,clif_parse_DropItem,12,17);
	packet_db[11][0x00a2] = packet_cmd(10,clif_parse_SolveCharName,6);
	packet_db[11][0x00a7] = packet_cmd(29,clif_parse_UseSkillToPos,6,20,23,27);
	packet_db[11][0x00f3] = packet_cmd(18,clif_parse_ChangeDir,8,17);
	packet_db[11][0x00f5] = packet_cmd(32,clif_parse_WantToConnection,10,17,23,27,31);
	packet_db[11][0x009b] = packet_cmd(10,clif_parse_GetCharNameRequest,6);
	packet_db[11][0x0113] = packet_cmd(14,clif_parse_TakeItem,10);
	packet_db[11][0x0116] = packet_cmd(14,clif_parse_TickSend,10);
	packet_db[11][0x0190] = packet_cmd(14,clif_parse_UseSkillToId,4,7,10);
	packet_db[11][0x0193] = packet_cmd(12,clif_parse_MoveFromKafra,4,8);
	///////////////////////////////////////////////////////////////////////////
	// packet version 12 (2004-12-18)
	packet_db[12] = packet_db[11];
	packet_db[12].connect_cmd = 0xf5;
	packet_db[12][0x0072] = packet_cmd(17,clif_parse_UseItem,6,13);
	packet_db[12][0x007e] = packet_cmd(16,clif_parse_MoveToKafra,5,12);
	packet_db[12][0x0089] = packet_cmd(6,clif_parse_WalkToXY,3);
	packet_db[12][0x008c] = packet_cmd(103,clif_parse_UseSkillToPos,2,6,17,21,23);
	packet_db[12][0x0094] = packet_cmd(14,clif_parse_DropItem,5,12);
	packet_db[12][0x009b] = packet_cmd(15,clif_parse_GetCharNameRequest,11);
	packet_db[12][0x00a2] = packet_cmd(12,clif_parse_SolveCharName,8);
	packet_db[12][0x00a7] = packet_cmd(23,clif_parse_UseSkillToPos,3,6,17,21);
	packet_db[12][0x00f3] = packet_cmd(13,clif_parse_ChangeDir,5,12);
	packet_db[12][0x00f5] = packet_cmd(33,clif_parse_WantToConnection,12,18,24,28,32);
	packet_db[12][0x0113] = packet_cmd(10,clif_parse_TakeItem,6);
	packet_db[12][0x0116] = packet_cmd(10,clif_parse_TickSend,6);
	packet_db[12][0x0190] = packet_cmd(20,clif_parse_UseSkillToId,7,12,16);
	packet_db[12][0x0193] = packet_cmd(26,clif_parse_MoveFromKafra,10,22);
	///////////////////////////////////////////////////////////////////////////
	// packet version 13 (2004-10-25)
	packet_db[13] = packet_db[12];
	packet_db[13].connect_cmd = 0xf5;
	packet_db[13][0x0072] = packet_cmd(13,clif_parse_UseItem,5,9);
	packet_db[13][0x007e] = packet_cmd(13,clif_parse_MoveToKafra,6,9);
	packet_db[13][0x0085] = packet_cmd(15,clif_parse_ActionRequest,4,14);
	packet_db[13][0x008c] = packet_cmd(108,clif_parse_UseSkillToPos,6,9,23,26,28);
	packet_db[13][0x0094] = packet_cmd(12,clif_parse_DropItem,6,10);
	packet_db[13][0x009b] = packet_cmd(10,clif_parse_GetCharNameRequest,6);
	packet_db[13][0x00a2] = packet_cmd(16,clif_parse_SolveCharName,12);
	packet_db[13][0x00a7] = packet_cmd(28,clif_parse_UseSkillToPos,6,9,23,26);
	packet_db[13][0x00f3] = packet_cmd(15,clif_parse_ChangeDir,6,14);
	packet_db[13][0x00f5] = packet_cmd(29,clif_parse_WantToConnection,5,14,20,24,28);
	packet_db[13][0x0113] = packet_cmd(9,clif_parse_TakeItem,5);
	packet_db[13][0x0116] = packet_cmd(9,clif_parse_TickSend,5);
	packet_db[13][0x0190] = packet_cmd(26,clif_parse_UseSkillToId,4,10,22);
	packet_db[13][0x0193] = packet_cmd(22,clif_parse_MoveFromKafra,12,18);
	///////////////////////////////////////////////////////////////////////////
	// packet version 14 (2004-11-01)
	packet_db[14] = packet_db[13];
	packet_db[14].connect_cmd = 0;// packet version disabled
	packet_db[14][0x0143] = packet_cmd(23,clif_parse_NpcAmountInput,2,6);
	packet_db[14][0x0215] = packet_cmd(6);
	packet_db[14][0x0216] = packet_cmd(6);
	///////////////////////////////////////////////////////////////////////////
	// packet version 15 (2004-12-04)
	packet_db[15] = packet_db[14];
	packet_db[15].connect_cmd = 0xf5;
	packet_db[15][0x0190] = packet_cmd(15,clif_parse_UseItem,3,11);
	packet_db[15][0x0094] = packet_cmd(14,clif_parse_MoveToKafra,4,10);
	packet_db[15][0x009f] = packet_cmd(18,clif_parse_ActionRequest,6,17);
	packet_db[15][0x00a7] = packet_cmd(7,clif_parse_WalkToXY,4);
	packet_db[15][0x007e] = packet_cmd(30,clif_parse_UseSkillToPos,4,9,22,28);
	packet_db[15][0x0116] = packet_cmd(12,clif_parse_DropItem,4,10);
	packet_db[15][0x008c] = packet_cmd(13,clif_parse_GetCharNameRequest,9);
	packet_db[15][0x0085] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[15][0x00f7] = packet_cmd(14,clif_parse_SolveCharName,10);
	packet_db[15][0x0113] = packet_cmd(110,clif_parse_UseSkillToPos,4,9,22,28,30);
	packet_db[15][0x00f3] = packet_cmd(8,clif_parse_ChangeDir,3,7);
	packet_db[15][0x00f5] = packet_cmd(29,clif_parse_WantToConnection,3,10,20,24,28);
	packet_db[15][0x00a2] = packet_cmd(7,clif_parse_TakeItem,3);
	packet_db[15][0x0089] = packet_cmd(7,clif_parse_TickSend,3);
	packet_db[15][0x0072] = packet_cmd(22,clif_parse_UseSkillToId,8,12,18);
	packet_db[15][0x0193] = packet_cmd(21,clif_parse_MoveFromKafra,4,17);
	packet_db[15][0x009b] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[15][0x0217] = packet_cmd(2,clif_parse_Blacksmith,0);
	packet_db[15][0x0218] = packet_cmd(2,clif_parse_Alchemist,0);
	packet_db[15][0x0219] = packet_cmd(282);
	packet_db[15][0x021a] = packet_cmd(282);
	packet_db[15][0x021b] = packet_cmd(10);
	packet_db[15][0x021c] = packet_cmd(10);
	packet_db[15][0x021d] = packet_cmd(6);
	packet_db[15][0x021e] = packet_cmd(6);
	packet_db[15][0x0221] = packet_cmd(-1);
	packet_db[15][0x0222] = packet_cmd(6,clif_parse_WeaponRefine,2);
	packet_db[15][0x0223] = packet_cmd(8);
	///////////////////////////////////////////////////////////////////////////
	// packet version 16 (2005-01-10)
	packet_db[16] = packet_db[15];
	packet_db[16].connect_cmd = 0x9b;
	packet_db[16][0x009b] = packet_cmd(32,clif_parse_WantToConnection,3,12,23,27,31);
	packet_db[16][0x0089] = packet_cmd(9,clif_parse_TickSend,5);
	packet_db[16][0x00a7] = packet_cmd(13,clif_parse_WalkToXY,10);
	packet_db[16][0x0190] = packet_cmd(20,clif_parse_ActionRequest,9,19);
	packet_db[16][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[16][0x008c] = packet_cmd(8,clif_parse_GetCharNameRequest,4);
	packet_db[16][0x0085] = packet_cmd(23,clif_parse_ChangeDir,12,22);
	packet_db[16][0x0094] = packet_cmd(20,clif_parse_MoveToKafra,10,16);
	packet_db[16][0x0193] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[16][0x00f7] = packet_cmd(21,clif_parse_MoveFromKafra,11,17);
	packet_db[16][0x009f] = packet_cmd(17,clif_parse_UseItem,5,13);
	packet_db[16][0x0116] = packet_cmd(20,clif_parse_DropItem,15,18);
	packet_db[16][0x00f5] = packet_cmd(9,clif_parse_TakeItem,5);
	packet_db[16][0x0113] = packet_cmd(34,clif_parse_UseSkillToPos,10,18,22,32);
	packet_db[16][0x0072] = packet_cmd(26,clif_parse_UseSkillToId,8,16,22);
	packet_db[16][0x007e] = packet_cmd(114,clif_parse_UseSkillToPos,10,18,22,32,34);
	packet_db[16][0x00a2] = packet_cmd(11,clif_parse_SolveCharName,7);
	packet_db[16][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[16][0x021f] = packet_cmd(66);
	packet_db[16][0x0220] = packet_cmd(10);
	///////////////////////////////////////////////////////////////////////////
	// packet version 17 (2005-05-10)
	packet_db[17] = packet_db[16];
	packet_db[17].connect_cmd = 0x9b;
	packet_db[17][0x009b] = packet_cmd(26,clif_parse_WantToConnection,4,9,17,18,25);
	packet_db[17][0x0089] = packet_cmd(8,clif_parse_TickSend,4);
	packet_db[17][0x00a7] = packet_cmd(8,clif_parse_WalkToXY,5);
	packet_db[17][0x0190] = packet_cmd(19,clif_parse_ActionRequest,5,18);
	packet_db[17][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[17][0x008c] = packet_cmd(11,clif_parse_GetCharNameRequest,7);
	packet_db[17][0x0085] = packet_cmd(11,clif_parse_ChangeDir,7,10);
	packet_db[17][0x0094] = packet_cmd(14,clif_parse_MoveToKafra,7,10);
	packet_db[17][0x0193] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[17][0x00f7] = packet_cmd(22,clif_parse_MoveFromKafra,14,18);
	packet_db[17][0x009f] = packet_cmd(14,clif_parse_UseItem,4,10);
	packet_db[17][0x0116] = packet_cmd(10,clif_parse_DropItem,5,8);
	packet_db[17][0x00f5] = packet_cmd(8,clif_parse_TakeItem,4);
	packet_db[17][0x0113] = packet_cmd(22,clif_parse_UseSkillToPos,5,9,12,20);
	packet_db[17][0x0072] = packet_cmd(25,clif_parse_UseSkillToId,6,10,21);
	packet_db[17][0x007e] = packet_cmd(102,clif_parse_UseSkillToPos,5,9,12,20,22);
	packet_db[17][0x00a2] = packet_cmd(15,clif_parse_SolveCharName,11);
	packet_db[17][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[17][0x0224] = packet_cmd(10);
	packet_db[17][0x0225] = packet_cmd(2);
	packet_db[17][0x0226] = packet_cmd(282);
	packet_db[17][0x0227] = packet_cmd(18);
	packet_db[17][0x0228] = packet_cmd(18);
	packet_db[17][0x0229] = packet_cmd(15);
	packet_db[17][0x022a] = packet_cmd(58);
	packet_db[17][0x022b] = packet_cmd(57);
	packet_db[17][0x022c] = packet_cmd(64);
	packet_db[17][0x022d] = packet_cmd(5);
	packet_db[17][0x0232] = packet_cmd(9);
	packet_db[17][0x0233] = packet_cmd(11);
	packet_db[17][0x0234] = packet_cmd(-1);
	///////////////////////////////////////////////////////////////////////////
	// packet version 18 (2005-06-28)
	packet_db[18] = packet_db[17];
	packet_db[18].connect_cmd = 0x9b;
	packet_db[18][0x009b] = packet_cmd(32,clif_parse_WantToConnection,9,15,23,30,31);
	packet_db[18][0x0089] = packet_cmd(13,clif_parse_TickSend,9);
	packet_db[18][0x00a7] = packet_cmd(11,clif_parse_WalkToXY,8);
	packet_db[18][0x0190] = packet_cmd(24,clif_parse_ActionRequest,11,23);
	packet_db[18][0x00f3] = packet_cmd(-1,clif_parse_GlobalMessage,2,4);
	packet_db[18][0x008c] = packet_cmd(8,clif_parse_GetCharNameRequest,4);
	packet_db[18][0x0085] = packet_cmd(17,clif_parse_ChangeDir,8,16);
	packet_db[18][0x0094] = packet_cmd(31,clif_parse_MoveToKafra,16,27);
	packet_db[18][0x0193] = packet_cmd(2,clif_parse_CloseKafra,0);
	packet_db[18][0x00f7] = packet_cmd(18,clif_parse_MoveFromKafra,11,14);
	packet_db[18][0x009f] = packet_cmd(19,clif_parse_UseItem,9,15);
	packet_db[18][0x0116] = packet_cmd(12,clif_parse_DropItem,3,10);
	packet_db[18][0x00f5] = packet_cmd(13,clif_parse_TakeItem,9);
	packet_db[18][0x0113] = packet_cmd(33,clif_parse_UseSkillToPos,12,15,18,31);
	packet_db[18][0x0072] = packet_cmd(34,clif_parse_UseSkillToId,6,17,30);
	packet_db[18][0x007e] = packet_cmd(113,clif_parse_UseSkillToPos,12,15,18,31,33);
	packet_db[18][0x00a2] = packet_cmd(9,clif_parse_SolveCharName,5);
	packet_db[18][0x0143] = packet_cmd(10,clif_parse_NpcAmountInput,2,6);
	packet_db[18][0x020e] = packet_cmd(10);
	packet_db[18][0x022e] = packet_cmd(71);
	packet_db[18][0x0235] = packet_cmd(115);
	packet_db[18][0x0248] = packet_cmd(116);
	///////////////////////////////////////////////////////////////////////////
	// packet version 19 (2005-06-28)
	packet_db[19] = packet_db[18];
	packet_db[19].connect_cmd = 0x9b;
	packet_db[19][0x009b] = packet_cmd(37,clif_parse_WantToConnection,9,21,28,32,36);
	packet_db[19][0x00a2] = packet_cmd(11,clif_parse_SolveCharName,7);
	packet_db[19][0x008c] = packet_cmd(12,clif_parse_GetCharNameRequest,8);
	packet_db[19][0x00a7] = packet_cmd(15,clif_parse_WalkToXY,12);
	packet_db[19][0x0116] = packet_cmd(17,clif_parse_DropItem,8,15);
	packet_db[19][0x009f] = packet_cmd(24,clif_parse_UseItem,9,20);
	packet_db[19][0x0072] = packet_cmd(26,clif_parse_UseSkillToId,11,18,22);
	packet_db[19][0x0113] = packet_cmd(40,clif_parse_UseSkillToPos,5,15,29,38);
	packet_db[19][0x007e] = packet_cmd(120,clif_parse_UseSkillToPos,5,15,29,38,40);
	packet_db[19][0x0085] = packet_cmd(12,clif_parse_ChangeDir,7,11);
	packet_db[19][0x0094] = packet_cmd(23,clif_parse_MoveToKafra,5,19);
	packet_db[19][0x00f7] = packet_cmd(26,clif_parse_MoveFromKafra,11,22);
	packet_db[19][0x0190] = packet_cmd(18,clif_parse_ActionRequest,7,17);


	///////////////////////////////////////////////////////////////////////////
	size_t i;
	for(i=20; i<=MAX_PACKET_VER; ++i)
		packet_db[i] = packet_db[i-1];
	///////////////////////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////////////////////
	// read in packet_db from file
	FILE *fp;
	char line[1024];
	int ln=0;
	int cmd;
	size_t j, packet_ver;
	int k;
	char *str[64],*p,*str2[64],*p2,w1[256],w2[256];

	static const struct {
		int (*func)(int, map_session_data &);
		char *name;
	} clif_parse_func[]={
		{clif_parse_WantToConnection,"wanttoconnection"},
		{clif_parse_LoadEndAck,"loadendack"},
		{clif_parse_TickSend,"ticksend"},
		{clif_parse_WalkToXY,"walktoxy"},
		{clif_parse_QuitGame,"quitgame"},
		{clif_parse_GetCharNameRequest,"getcharnamerequest"},
		{clif_parse_GlobalMessage,"globalmessage"},
		{clif_parse_MapMove,"mapmove"},
		{clif_parse_ChangeDir,"changedir"},
		{clif_parse_Emotion,"emotion"},
		{clif_parse_HowManyConnections,"howmanyconnections"},
		{clif_parse_ActionRequest,"actionrequest"},
		{clif_parse_Restart,"restart"},
		{clif_parse_Wis,"wis"},
		{clif_parse_GMmessage,"gmmessage"},
		{clif_parse_TakeItem,"takeitem"},
		{clif_parse_DropItem,"dropitem"},
		{clif_parse_UseItem,"useitem"},
		{clif_parse_EquipItem,"equipitem"},
		{clif_parse_UnequipItem,"unequipitem"},
		{clif_parse_NpcClicked,"npcclicked"},
		{clif_parse_NpcBuySellSelected,"npcbuysellselected"},
		{clif_parse_NpcBuyListSend,"npcbuylistsend"},
		{clif_parse_NpcSellListSend,"npcselllistsend"},
		{clif_parse_CreateChatRoom,"createchatroom"},
		{clif_parse_ChatAddMember,"chataddmember"},
		{clif_parse_ChatRoomStatusChange,"chatroomstatuschange"},
		{clif_parse_ChangeChatOwner,"changechatowner"},
		{clif_parse_KickFromChat,"kickfromchat"},
		{clif_parse_ChatLeave,"chatleave"},
		{clif_parse_TradeRequest,"traderequest"},
		{clif_parse_TradeAck,"tradeack"},
		{clif_parse_TradeAddItem,"tradeadditem"},
		{clif_parse_TradeOk,"tradeok"},
		{clif_parse_TradeCancel,"tradecancel"},
		{clif_parse_TradeCommit,"tradecommit"},
		{clif_parse_StopAttack,"stopattack"},
		{clif_parse_PutItemToCart,"putitemtocart"},
		{clif_parse_GetItemFromCart,"getitemfromcart"},
		{clif_parse_RemoveOption,"removeoption"},
		{clif_parse_ChangeCart,"changecart"},
		{clif_parse_StatusUp,"statusup"},
		{clif_parse_SkillUp,"skillup"},
		{clif_parse_UseSkillToId,"useskilltoid"},
		{clif_parse_UseSkillToPos,"useskilltopos"},
		{clif_parse_UseSkillToPos,"useskilltoposinfo"},
//		{clif_parse_UseSkillToPosMoreInfo,"useskilltoposinfo"},
		{clif_parse_UseSkillMap,"useskillmap"},
		{clif_parse_RequestMemo,"requestmemo"},
		{clif_parse_ProduceMix,"producemix"},
		{clif_parse_NpcSelectMenu,"npcselectmenu"},
		{clif_parse_NpcNextClicked,"npcnextclicked"},
		{clif_parse_NpcAmountInput,"npcamountinput"},
		{clif_parse_NpcStringInput,"npcstringinput"},
		{clif_parse_NpcCloseClicked,"npccloseclicked"},
		{clif_parse_ItemIdentify,"itemidentify"},
		{clif_parse_SelectArrow,"selectarrow"},
		{clif_parse_AutoSpell,"autospell"},
		{clif_parse_UseCard,"usecard"},
		{clif_parse_InsertCard,"insertcard"},
		{clif_parse_RepairItem,"repairitem"},
		{clif_parse_WeaponRefine,"weaponrefine"},
		{clif_parse_SolveCharName,"solvecharname"},
		{clif_parse_ResetChar,"resetchar"},
		{clif_parse_LGMmessage,"lgmmessage"},
		{clif_parse_MoveToKafra,"movetokafra"},
		{clif_parse_MoveFromKafra,"movefromkafra"},
		{clif_parse_MoveToKafraFromCart,"movetokafrafromcart"},
		{clif_parse_MoveFromKafraToCart,"movefromkafratocart"},
		{clif_parse_CloseKafra,"closekafra"},
		{clif_parse_CreateParty,"createparty"},
		{clif_parse_CreateParty2,"createparty2"},
		{clif_parse_PartyInvite,"partyinvite"},
		{clif_parse_ReplyPartyInvite,"replypartyinvite"},
		{clif_parse_LeaveParty,"leaveparty"},
		{clif_parse_RemovePartyMember,"removepartymember"},
		{clif_parse_PartyChangeOption,"partychangeoption"},
		{clif_parse_PartyMessage,"partymessage"},
		{clif_parse_CloseVending,"closevending"},
		{clif_parse_VendingListReq,"vendinglistreq"},
		{clif_parse_PurchaseReq,"purchasereq"},
		{clif_parse_OpenVending,"openvending"},
		{clif_parse_CreateGuild,"createguild"},
		{clif_parse_GuildCheckMaster,"guildcheckmaster"},
		{clif_parse_GuildRequestInfo,"guildrequestinfo"},
		{clif_parse_GuildChangePositionInfo,"guildchangepositioninfo"},
		{clif_parse_GuildChangeMemberPosition,"guildchangememberposition"},
		{clif_parse_GuildRequestEmblem,"guildrequestemblem"},
		{clif_parse_GuildChangeEmblem,"guildchangeemblem"},
		{clif_parse_GuildChangeNotice,"guildchangenotice"},
		{clif_parse_GuildInvite,"guildinvite"},
		{clif_parse_GuildReplyInvite,"guildreplyinvite"},
		{clif_parse_GuildLeave,"guildleave"},
		{clif_parse_GuildExplusion,"guildexplusion"},
		{clif_parse_GuildMessage,"guildmessage"},
		{clif_parse_GuildRequestAlliance,"guildrequestalliance"},
		{clif_parse_GuildReplyAlliance,"guildreplyalliance"},
		{clif_parse_GuildDelAlliance,"guilddelalliance"},
		{clif_parse_GuildOpposition,"guildopposition"},
		{clif_parse_GuildBreak,"guildbreak"},
		{clif_parse_PetMenu,"petmenu"},
		{clif_parse_CatchPet,"catchpet"},
		{clif_parse_SelectEgg,"selectegg"},
		{clif_parse_SendEmotion,"sendemotion"},
		{clif_parse_ChangePetName,"changepetname"},
		{clif_parse_GMKick,"gmkick"},
		{clif_parse_GMHide,"gmhide"},
		{clif_parse_GMReqNoChat,"gmreqnochat"},
		{clif_parse_GMReqNoChatCount,"gmreqnochatcount"},
		{clif_parse_NoviceDoriDori,"sndoridori"},
		{clif_parse_NoviceExplosionSpirits,"snexplosionspirits"},
		{clif_parse_PMIgnore,"wisexin"},
		{clif_parse_PMIgnoreList,"wisexlist"},
		{clif_parse_PMIgnoreAll,"wisall"},
		{clif_parse_GMKillAll,"killall"},
		{clif_parse_Recall,"summon"},
		{clif_parse_GM_Monster_Item,"itemmonster"},
		{clif_parse_Shift,"shift"},

		{clif_parse_FriendsListAdd,"friendslistadd"},
		{clif_parse_FriendsListAdd,"friendaddrequest"},
		{clif_parse_FriendsListRemove,"friendslistremove"},
		{clif_parse_FriendsListRemove,"frienddeleterequest"},
		{clif_parse_FriendsListReply,"friendslistreply"},
		{clif_parse_FriendsListReply,"friendaddreply"},
		
		{clif_parse_Blacksmith,"blacksmith"},
		{clif_parse_Blacksmith,"rankingblacksmith"},
		{clif_parse_Alchemist,"alchemist"},
		{clif_parse_Alchemist,"rankingalchemist"},
		{clif_parse_Taekwon,"taekwon"},
		{clif_parse_Taekwon,"rankingtaekwon"},
		{clif_parse_RankingPk,"rankingpk"},
		{clif_parse_BabyRequest,"babyrequest"},

		{clif_parse_HomMenu,"hommenu"},
		{clif_parse_HomWalkMaster,"homwalkmaster"},
		{clif_parse_HomWalkToXY,"homwalktoxy"},
		{clif_parse_HomActionRequest,"homactionrequest"},
		{clif_parse_ChangeHomName,"changehomname"},

		{clif_parse_MailWinOpen,"mailwinopen"},
		{clif_parse_ReadMail,"readmail"},
		{clif_parse_MailGetAppend,"mailgetappend"},
		{clif_parse_SendMail,"sendmail"},
		{clif_parse_RefreshMailBox,"refleshmailbox"},
		{clif_parse_SendMailSetAppend,"sendmailsetappend"},
		{clif_parse_DeleteMail,"deletemail"},

		{clif_parse_FeelSaveOk,"feelsaveok"},
		{clif_parse_AdoptRequest,"adopt"},
		
		{clif_parse_clientsetting,"clientsetting"},
		{clif_parse_debug,"debug"},
		{NULL,NULL}
	};
	const char* cfgName = "db/packet_db.txt";

	if( (fp=basics::safefopen(cfgName,"r"))==NULL )
	{
		ShowWarning("Packet Configuration '"CL_WHITE"%s"CL_RESET"' not found, using defaults.\n", cfgName);
		return 1;
	}
	packet_ver = MAX_PACKET_VER;
	while( fgets(line,sizeof(line),fp) )
	{
		ln++;
		if( prepare_line(line) && 2==sscanf(line,"%256[^:=]%*[:=]%256[^\r\n]",w1,w2) )
		{
			basics::itrim(w1);
			if(!*w1) continue;
			basics::itrim(w2);

			if(strcasecmp(w1,"packet_ver")==0)
			{	// start of a new version
				size_t prev_ver = packet_ver;
				packet_ver = atoi(w2);
				if (packet_ver > MAX_PACKET_VER)
				{	//Check to avoid overflowing. [Skotlex]
					ShowWarning("The packet_db table only has support up to version %d\n", MAX_PACKET_VER);
					break;
				}
				// copy from previous version into new version and continue
				// - indicating all following packets should be read into the newer version
				for(i=prev_ver+1; i<=packet_ver; ++i)
					packet_db[i]=packet_db[prev_ver];
				continue;
			}
			else if(strcasecmp(w1,"enable_packet_db")==0)
			{	// only working when found in the beginning of the packet_db file
				if( !basics::config_switch<bool>(w2) )
					return 0;
				continue;
			}
			else if(strcasecmp(w1,"default_packet_ver")==0)
			{	// use this when in daubt, but pretty useless
				packet_db.default_ver = basics::config_switch<int>(w2,0,MAX_PACKET_VER);
				// check for invalid version
				if( packet_db.default_ver > MAX_PACKET_VER ||
					packet_db.default_ver < 0 )
					packet_db.default_ver = MAX_PACKET_VER;
				continue;
			}
		}

		if(packet_ver<=MAX_PACKET_VER)
		{	// only read valid packet_ver's
			memset(str,0,sizeof(str));
			for(j=0,p=line;j<4 && p; ++j)
			{
				str[j]=p;
				p=strchr(p,',');
				if(p) *p++=0;
			}
			if(str[0]==NULL)
				continue;
			cmd=strtol(str[0],(char **)NULL,0);
			if(cmd<=0 || cmd>=MAX_PACKET_DB)
				continue;
			if(str[1]==NULL)
			{
				ShowError("'"CL_WHITE"%s"CL_RESET"', line %i: packet len error.\n", cfgName, ln);
				continue;
			}
			k = atoi(str[1]);
			packet_db[packet_ver][cmd].len = k;

			if(str[2]==NULL){
				continue;
			}
			for(j=0;j<sizeof(clif_parse_func)/sizeof(clif_parse_func[0]); ++j)
			{
				if( clif_parse_func[j].name != NULL &&
					strcmp(str[2],clif_parse_func[j].name)==0)
				{
					// if (packet_db[packet_ver][cmd].func != clif_parse_func[j].func && !clif_config.prefer_packet_db)
					//	break;	// not used for now
					packet_db[packet_ver][cmd].func = clif_parse_func[j].func;
					break;
				}
			}
			if( j>=sizeof(clif_parse_func)/sizeof(clif_parse_func[0]) )
			{
				ShowError("'"CL_WHITE"%s"CL_RESET"', line %i: parse command '%s' not found.\n", cfgName, ln, str[2]);
			}
			// set the identifying cmd for the packet_db version
			if(strcasecmp(str[2],"wanttoconnection")==0)
			{
				packet_db[packet_ver].connect_cmd = cmd;
			}
			if(str[3]==NULL)
			{
				ShowError("'"CL_WHITE"%s"CL_RESET"', line %i: no positions.\n", cfgName, ln);
			}
			for(j=0,p2=str[3];p2; ++j){
				str2[j]=p2;
				p2=strchr(p2,':');
				if(p2) *p2++=0;
				k = atoi(str2[j]);
				packet_db[packet_ver][cmd].pos[j] = k;
			}
		}
	}
	fclose(fp);
	ShowStatus("Done reading Packet Configuration '"CL_WHITE"%s"CL_RESET"'.\n",cfgName);
	return 0;
}


