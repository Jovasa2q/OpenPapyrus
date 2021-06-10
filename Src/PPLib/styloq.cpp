// STYLOQ.CPP
// Copyright (c) A.Sobolev 2021
// @construction
//
#include <pp.h>
#pragma hdrstop
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rand.h>

const int ec_curve_name_id = NID_X9_62_prime256v1;
/*
	����� ������� �������������� ����������� ������� ��������� Rabbit-MQ ��� ������ ����������� � ��������,
	���������������� ������.

	�������:
	��������� ������������� ������� (SVCPID) - �������������, ����������� �������� ��� ����, ����� ��� ����� �������� �������
	��������� ������������� �������, ��������������� � �������� (CLSPID) - �������������, ������������ �������� ������� ��� ����, �����
		��� ��� ������ �������. �������������� ������ ������� ��� ������ �������� �����������.
	��� (CLFACE) - ������������� ������� ����� ��������. ������ ����� ����� ������������ ����� �����, ������� �������������� �����
		���������. ������, ��������� CLSPID->CLFACE ������������ ��� 1:1, �� ����, � ����� ������ ������� ������ ��������� ����
		���� ��� ������� � ��� ���������������. 
		���� ����������� ���������������� ���������� ������� ���� �� �������: ������ �� ������ ���������� ������� ���������� ���������� ����������,
		��������������� � �����, �������.
		� ������ �������, ���� ������ ��������� ������� �������� ������� ���� ���������� ��������� ���������, �� ������ ��� �����������
		(��������, �� ������������ ���������� �������) � ���������� ����, ��� � ������� �������� ���� ������ ��� ��� ����� ��������� ��������
		���������� �������� ����������� ��� ��������� ������������� (CLSPID) � ������, ��� ��� ������������� ������ � ���� �� ������.

		������ ��� ����������� ����� �� ���� ���������: 
		1. ��������� ���������: ������� ����� ����������� ����, ����� �������� ���������������� �����, ������ ������� �� ��������,
			��� ���� ������ �������� ������ � ���, ��� ��� ���������
		2. ��������� ��������������: ������ �������� ������� ����������, ����������� � ������� ������������ ����������������
			���������� ����. ��������, ��������� �����, ��������, ������� � ���� ��������, ��� ����� ��������. � ����� ������,
			���, ����������� � ���� ��������� ������� � �����������. ������� �������, ������, ������ ������� �������� �������
			��������� �������������� ��� ���� �������� �� ���� ������� � ��� �������� ��, ��� ������.
		3. ������������: ������ �� ��������� ���� ��� �� ��� ���������, �� ��� ��������� ��������������. ����� �������, ������
			������������ �� �������� ������ � ��� ������ ����� ���� � ����� �� �������� ����������� ������ ����� ������� �� ������
			��������.
	��������� ������� (SVCCMD) - ��������, ������������ �������� � ����������� ��������. ������� ����� ��������� ���������, �������
		������� � ������� � �������. ��������� ������� ������������ ������� ��������� ����� � ���� key-value, ����������� �����������
		���������������� ������� (json ��� xml).

	������ ��������� ����������� ��������� �������������, �� �������� ��� ������ �������
	������ ��� ����������� �� ������� ������������� ����������� ��������� �������������, ��������������� ������ � ���� ��������.
	������ �� ���������� ���� � ��� �� ������������� ��� �������������� � ������� ���������.
	������ �������� ������� ��������� ������������ ������ � �����.

	��������:
	I. ������ �� QR-�����������
	1.1 ������ ��������� ����������� (QR-code), ���������� ��������� �� �������, ����� ������� ������ � ����������� ������ � �������,
		������� ����� ��������� ������.
	1.2 ���� ������ �� ����� ����������� ������� � ����� ����������� ��������� �����������, �� ������ ������������� �������������� 
		(������� ����������� ����� ������ �����)
	1.3 ���� ������ ��������������� � �������, �� ��������� �����������, ������������� ���������� � ���������� ���������� 
		�������������� ������� ��������.
	II. ���������� ������� ��������, ������������ ������������������ ��������
	2.1 ������ ������������� ���������� � �������� �� ������, ������� ���� ��������� ��� ��������� ��������. 
		�������������� ���� �� ��������� ���������:
		2.1.1 ���� �������� ���������� ��������� ������ �� ����� - � ���� ������ ������ ����������� �� ���� ����������
		2.1.2 ��������� ��������� ������ �������� ���������� - ������ �������� ������������ ����� ������ � ��������
		����� ��������� ���������� ������ ���������� ���� �� ������, �������������� ��������
	III. �������������� �������� ������������������� �������. ������ ����� ����������� �������� �����-���� ���������� 
		������������������� �������. ��� ���� ������ ����� ����������� ����������� ��������� ���������� �� �������.
	IV. ����� �������� ��������. ������ ����� ����������� ��������� ����������������� ���������� ������, ������� �����
		����� ���� �������� ���������� �� ����, ����� ������ � ��� ����������� ��� ���. ����� ������ ����� �����������
		�������� �� ����������������� ������ �������. ��� ����, ��� ������ �������, ��� � ������ �������� ��������
		��������� ����������� ��������-����������� � ����� ������������ � ���������� ��������, � ��� �� ������������ 
		�������.

	��������������, ��� ����� ������� ����������� �� ��������� ������ � ������� �� �������� ���������� �������� ������,
	�������� ��� �����������.

	����������� � �����������:
		������� ����������� � ����������� ����������� ����������� ��������� SRP. ��� ��������� ������������� ��������������
		� ������������ ����������� ������� ������ (�� ��� ������).
	����� ������� ����� �������� � ��������:
		����� �������������� ����������� ������������� ������. ����������� ���������� � �������� ������ (RSA). �������� � ���������
		����� ������������ ��� ������������ ������ ������. ����� ����� ������ ����������.
	������ ������ � ������ ������ ����������� ������������ ����������� ���������� �������, ������� ����������� ����������
	��� ���������� ��������������� �������. ��������� ������ ����� ��������������� � ������������ �� ������ ������� ������-����
	������������� ��������� ������.

	���� ��������� ������->������:
	1. ������ �������� ���������, ����������:
	    -- ������������� �������
		-- ��������� ������������� �������������� �������
*/
//
// @construction
// Descr: �����, ����������� �������� ������ ��������.
//   ������ - ���� � ����������� ����������. �� ���� ����� ���������� �������� ������� ������
//   ����� ����� ���������.
//   ��������, ������� � ������� ������, ����������� � ���, ��� ������ � ����� �� ���� ������
//   ��������������� ����������� ���������� ���� ������������. �� ������ ����� ��������������� 
//   ������ ��������� RabbitMQ (��������� � ��� ���� � ��� ��������). ����� ����, ��� ������ �����
//   �������������� ����� � ������ �������� ��������. �����, �������������� �������� ���� ��������
//   ��������� � ��� �� ������ tcp-���������� ����� ��� � http. � �����, �������� �������, ������
//   � ���� �����.
//
//   ����, _RoundTripPool �������� ����������� ����������� ��������� _RoundTripPool::Entry
//   ������ �� ������� ������ ��������� ����� round-trip-����������. ������� � ���� ������������ ������.
//   ��������, SRP-����������� �������� (� ����������� �� ��������) 4-6 ���. ������������ ������ ������ - 
//   ���� ����� 2 ��� � ��� �����. ��� ��� � ��������� ������������ ���� � ���� ������������� �������.
//   ��� ���, _RoundTripPool::Entry �������� ����������� ������ Send � Poll (������ Send � �� ������,
//   ��� �� ����� �����). ����� Poll ���������� ������������ �������� ���� �� ��� ������� Entry ���-����
//   �� ������. ��� ����, �������� ��, ��� ������� round-trip-���������� ����� ���� ����� ������, ��
//   ���������� ����� Entry �������������� ���������� ����� ��� �����-�� ������� ��� ���, � ��� ��, ���
//   � ��� ������.
//
//   ����� �����: ��������� Entry ����� ���� ����� (�� �����, ��� �����, �� ���� ��� �����). ������������ ���������� -
//   ����� ������� (���������� ���������� � �������� ��� ���� � tcp-����� �� ������� ����). ����������� ���������
//   ����� ������������������ ����� ������ �������, �� ��� - �� �����. ������ � ����������� ������
//   ���� ���������� ������ Carrier (�����������). ���� ������������� ����� ���� ���������. ��� ����, ������ 
//   ������� Entry ����� ����������� (�����������) ����� ���������� ��������������� TransportBlock.
//   ��� � ����� �������� ���: 
//      -- ����� Entry::Poll �������� Entry::GetCarrier(_RoundTripPool *) ������� ����������
//      ������������ �� ������������� ������ � ����������� �� ���������� TransportBlock. ����������
//      ����� ��������� �� ����� _RoundTripPool::GetCarrier(Entry::TransportBlock *),
//      ������� ������� ��� �������� ������������ � ����������� Entry::TransportBlock ��� ������� ������.
//      -- ���� GetCarrier() ������� !0 �� �� ��� ���������� ��� ����, ��� � �������� PPStyloQInterchange::TransportPacket
//      ���� �������� ����� ����� �������, �� ����� ����� Poll ��� ����������� ��� � ��� ������.  
//
//   ��� ������ ����������� RabbitMQ: 
//      ����� ������������ ��������� ��������: Exchange=StyloQ; Queue=MIME64(public-ident)
//      ����� ������� �� ������������ ���������: Exchange=StyloQ; Queue=round-trip-ident
//
//
// @construction
//
class StyloQProtocol : public PPJobSrvProtocol {
public:
	StyloQProtocol() : PPJobSrvProtocol()
	{
	}
	StyloQProtocol & Z()
	{
		PPJobSrvProtocol::Z();
		P.Z();
		return *this;
	}
	enum {
		psubtypeForward      = 0,
		psubtypeForwardError = 1,
		psubtypeReplyOk      = 2,
		psubtypeReplyError   = 3,
	};
	int    StartWriting(int cmdId, int subtype)
	{
		int    ok = 1;
		PPJobSrvProtocol::Z();
		H.ProtocolVer = CurrentProtocolVer;
		H.Type = cmdId;
		H.DataLen = sizeof(H);
		if(subtype == psubtypeForwardError)
			H.Flags |= hfRepError;
		else if(subtype == psubtypeReplyOk)
			H.Flags |= hfAck;
		else if(subtype == psubtypeReplyError)
			H.Flags |= (hfAck|hfRepError);
		THROW_SL(Write(&H, sizeof(H)));
		State |= stStructured;
		State &= ~stReading;
		CATCHZOK
		return ok;
	}
	int    FinishWriting(const SBinaryChunk * pCryptoKey)
	{
		int    ok = 1;
		bool   encrypted = false;
		uint8  padding = 0; // ������ "�������" ������ �� �������� ����� ���������� //
		assert(State & stStructured);
		assert(GetWrOffs() == sizeof(H));
		THROW(State & stStructured);
		if(pCryptoKey) {
			const binary128 key_md5 = SlHash::Md5(0, pCryptoKey->PtrC(), pCryptoKey->Len());
			SlCrypto cryp(SlCrypto::algAes, SlCrypto::kbl128, SlCrypto::algmodEcb);
			SlCrypto::Key crypkey;
			const SlCrypto::CipherProperties & r_cp = cryp.GetCipherProperties();
			P.Pack();
			STempBuffer cbuf2(P.GetDataLen() + 64); // 64 ensurance
			size_t cryp_size = 0;
			THROW_SL(cbuf2.IsValid());
			THROW_SL(cryp.SetupKey(crypkey, &key_md5, sizeof(key_md5), 0, 0));
			{
				const size_t raw_data_len = P.GetDataLen();
				size_t padded_size = r_cp.BlockSize ? (raw_data_len ? ((((raw_data_len-1) / r_cp.BlockSize) + 1) * r_cp.BlockSize) : r_cp.BlockSize) : raw_data_len;
				assert(padded_size >= raw_data_len && (padded_size - raw_data_len) < 256);
				THROW(padded_size >= raw_data_len && (padded_size - raw_data_len) < 256);
				padding = static_cast<uint8>(padded_size - raw_data_len);
				THROW_SL(P.Ensure(padded_size));
				THROW_SL(cryp.Encrypt_(&crypkey, P.GetPtr(), padded_size, cbuf2.vptr(), cbuf2.GetSize(), &cryp_size));
				THROW_SL(SBuffer::Write(cbuf2.vcptr(), cryp_size));
				encrypted = true;
			}
		}
		else {
			THROW(P.Serialize(+1, *this, 0/*!*/));
		}
		{
			Header * p_hdr = static_cast<Header *>(SBuffer::Ptr(SBuffer::GetRdOffs()));
			p_hdr->DataLen = static_cast<int32>(SBuffer::GetAvailableSize());
			SETFLAG(p_hdr->Flags, hfEncrypted, encrypted);
			p_hdr->Padding = padding;
		}
		CATCHZOK
		return ok;
	}
	int    Read(SBuffer & rMsgBuf, const SBinaryChunk * pCryptoKey)
	{
		int    ok = 1;
		Z();
		//THROW(SBuffer::Copy(rMsgBuf));
		SBuffer::SetRdOffs(0);
		THROW(rMsgBuf.Read(&H, sizeof(H)) == sizeof(H));
		THROW(H.ProtocolVer == CurrentProtocolVer);
		{
			const  size_t src_size = rMsgBuf.GetAvailableSize();
			THROW(H.DataLen == (src_size + sizeof(H)));
			THROW((H.Flags & hfEncrypted) || H.Padding == 0);		
			if(H.Flags & hfEncrypted) {
				THROW(pCryptoKey);
				{
					const binary128 key_md5 = SlHash::Md5(0, pCryptoKey->PtrC(), pCryptoKey->Len());
					SlCrypto cryp(SlCrypto::algAes, SlCrypto::kbl128, SlCrypto::algmodEcb);
					SlCrypto::Key crypkey;
					const SlCrypto::CipherProperties & r_cp = cryp.GetCipherProperties();
					STempBuffer cbuf2(src_size + 64); // 64 ensurance
					size_t cryp_size = 0;
					THROW_SL(cbuf2.IsValid());
					THROW_SL(cryp.SetupKey(crypkey, &key_md5, sizeof(key_md5), 0, 0));
					THROW(!r_cp.BlockSize || (src_size % r_cp.BlockSize) == 0);
					THROW_SL(cryp.Decrypt_(&crypkey, rMsgBuf.GetBufC(rMsgBuf.GetRdOffs()), src_size, cbuf2.vptr(), cbuf2.GetSize(), &cryp_size));
					THROW(cryp_size > H.Padding);
					THROW_SL(P.SetOuterBuffer(cbuf2.vptr(), cryp_size-H.Padding));
				}
			}
			else {
				THROW(P.Serialize(-1, rMsgBuf, 0/*!*/));
			}
		}
		CATCHZOK
		return ok;
	}
	int    Read(PPMqbClient::Message & rMsg, const SBinaryChunk * pCryptoKey)
	{
		return Read(rMsg.Body, pCryptoKey);
	}
	static int Test()
	{
		int    ok = 1;
		StyloQProtocol pack_src;
		StyloQProtocol pack_dest;
		const size_t crypto_key_size = 16;
		uint8 raw_crypto_key[128];
		for(uint roundidx = 0; roundidx < 100; roundidx++) {
			pack_src.Z();
			pack_dest.Z();
			SObfuscateBuffer(raw_crypto_key, crypto_key_size);
			const SBinaryChunk crypto_key(raw_crypto_key, crypto_key_size);
			{
				SBinaryChunk bc;
				pack_src.StartWriting(PPSCMD_SQ_SRPREGISTER, psubtypeForward);
				for(uint cidx = 0; cidx < 17; cidx++) {
					uint csz = SLS.GetTLA().Rg.GetUniformInt(8192);
					THROW(bc.Set(0xfe, csz));
					THROW(pack_src.P.Put(cidx+1, bc));
				}
				THROW(pack_src.FinishWriting(&crypto_key));
				//
				THROW(pack_dest.Read(pack_src, &crypto_key));
				THROW(pack_dest.P.IsEqual(pack_src.P));
			}
		}
		CATCHZOK
		return ok;
	}
	SSecretTagPool P;
};

class PPStyloQInterchange {
public:
	// 1 - native service, 2 - foreign service, 3 - client, 4 - client-session, 5 - service-session
	enum { // @persistent
		kUndef          = 0,
		kNativeService  = 1, // ����������� �������������. ������������ ��� ������ ����, ������� ����������, ������� ������� �� ����� ��������� //
		kForeignService = 2,
		kClient         = 3,
		kSession        = 4,
	};
	struct StoragePacket {
		StyloQSecTbl::Rec Rec;
		SSecretTagPool Pool;
	};
	struct Invitation {
		enum {
			capRegistrationAllowed = 0x0001,
			capVerifiableFaceOnly  = 0x0002,
			capAnonymFaceDisabled  = 0x0004
		};
		Invitation() : Capabitities(0)
		{
		}
		Invitation & Z()
		{
			SvcPublicId.Z();
			Capabitities = 0;
			AccessPoint.Z();
			Command.Z();
			CommandParam.Z();
			return *this;
		}
		int    FASTCALL IsEqual(const Invitation & rS) const
		{
			return (SvcPublicId == rS.SvcPublicId && Capabitities == rS.Capabitities && AccessPoint == rS.AccessPoint && 
				Command == rS.Command && CommandParam == rS.CommandParam);
		}
		S_GUID SvcPublicId;
		uint32 Capabitities; // capXXX
		SString AccessPoint;
		SString Command;
		SString CommandParam;
	};
	static int RunStyloQServer();
	PPStyloQInterchange();
	~PPStyloQInterchange();
	//
	// Descr: ������� ��������� �������������� ��������� ����������� ������
	//   � �������� � ����������� �� � ���� ������.
	// Returns:
	//   >0 - ������� ������� ��������� �������������� �������������
	//   <0 - ����������� �������/������� ��� ����������������
	//    0 - ������
	//
	int    SetupPeerInstance(PPID * pID, int use_ta);
	//
	// Descr: ���������� ����������� ��������
	//
	int    MakeInvitation(const Invitation & rSource, SString & rInvitationData);

	enum {
		fsksError = 0,
		fsksNewSession,
		fsksSessionById,
		fsksSessionByCliId,
		fsksSessionBySvcId
	};

	int    FetchSessionKeys(SSecretTagPool & rSessCtx, const SBinaryChunk & rForeignIdent);
	//
	// Descr: ���������� ��������� � ��������� ����� ��� ������ � ������������ � ����� ������������ ������ ������� ���������� //
	//   � ������ ������ ������� � �������� rSessCtx ������� �������� SSecretTagPool::tagSessionPrivateKey � SSecretTagPool::tagSessionPublicKey.
	//   ������� �������� ������ � rSessCtx ������� �� �������.
	// Returns:
	//   >0 - success
	//    0 - error
	//
	int    KexGenerateKeys(SSecretTagPool & rSessCtx);
	int    KexGenerageSecret(SSecretTagPool & rSessCtx, const SSecretTagPool & rOtherCtx);
	
	class RoundTripBlock {
	public:
		RoundTripBlock() : P_Mqbc(0), P_MqbRpe(0), InnerSvcID(0), InnerSessID(0), InnerCliID(0), State(0)
		{
		}
		RoundTripBlock(const void * pSvcIdent, size_t svcIdentLen, const char * pSvcAccsPoint) : P_Mqbc(0), P_MqbRpe(0), InnerSessID(0), InnerCliID(0), State(0)
		{
			if(pSvcIdent && svcIdentLen)
				Other.Put(SSecretTagPool::tagSvcIdent, pSvcIdent, svcIdentLen);
			if(!isempty(pSvcAccsPoint)) {
				Other.Put(SSecretTagPool::tagSvcAccessPoint, pSvcAccsPoint, strlen(pSvcAccsPoint)+1);
			}
		}
		~RoundTripBlock()
		{
			delete P_Mqbc;
			delete P_MqbRpe;
		}

		SSecretTagPool Other; // ���� ���������� �������, � �������� �������������� ������
		SSecretTagPool Sess;  // ���� ���������� ������
		StoragePacket StP;    // ���� ����������� ������, ����������� �� ���������
		PPMqbClient * P_Mqbc; // ��������� ������� MQ ��� "����������" �� ��������� ���������� ������� �� ������
		PPMqbClient::RoutingParamEntry * P_MqbRpe; // ��������� ������������� ��� ������������� ������� MQ
		PPID   InnerSvcID;
		PPID   InnerSessID;
		PPID   InnerCliID;
		uint   State;
	};
	int    InitRoundTripBlock(RoundTripBlock & rB);
	//
	// Descr: ������������� ��������� ����������� ������ ��� RoundTripBlock.
	//
	int    SetRoundTripBlockReplyValues(RoundTripBlock & rB, PPMqbClient * pMqbc, const PPMqbClient::RoutingParamEntry & rRpe);
	int    KexServiceReply(SSecretTagPool & rSessCtx, SSecretTagPool & rCli);
	//
	// Descr: ������ ����������� ������� ��������
	//
	int    AcceptInvitation(const char * pInvitationData, Invitation & rInv)
	{
		int    ok = 1;
		rInv.Z();
		STempBuffer temp_binary(4096);
		StringSet ss;
		SString temp_buf(pInvitationData);
		temp_buf.Tokenize("&", ss);
		uint   tokn = 0;
		for(uint ssp = 0; ss.get(&ssp, temp_buf);) {
			tokn++;
			size_t actual_size = 0;
			if(tokn == 1) { // prefix and svcid
				const char first_symb = temp_buf.C(0);
				THROW(first_symb == 'A'); // @error invalid invitation prefix
				temp_buf.ShiftLeft();
				temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
				THROW(actual_size == sizeof(rInv.SvcPublicId)); // @error invalid service public id
				memcpy(&rInv.SvcPublicId, temp_binary, actual_size);
			}
			else if(tokn == 2) { // capabilities
				temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
				THROW(actual_size == sizeof(rInv.Capabitities)); // @error invalid capabilities
				memcpy(&rInv.Capabitities, temp_binary, actual_size);
			}
			else if(tokn == 3) { // access point
				temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
				temp_buf.Z().CatN(temp_binary, actual_size);
				InetUrl url(temp_buf);
				THROW(url.GetProtocol());
				{
					SString host;
					url.GetComponent(InetUrl::cHost, 0, host); 
					THROW(host.NotEmpty()); // @error invalid url
				}
				rInv.AccessPoint = temp_buf;
			}
			else if(tokn == 4) { // command
				temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
				temp_buf.Z().CatN(temp_binary, actual_size);
				rInv.Command.CatN(temp_binary, actual_size);
			}
			else if(tokn == 5) { // command params
				temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
				temp_buf.Z().CatN(temp_binary, actual_size);
				rInv.CommandParam.CatN(temp_binary, actual_size);
			}
			else {
				CALLEXCEPT(); // invalid token count
			}
		}
		CATCHZOK
		return ok;
	}
	int    Registration_ClientRequest(RoundTripBlock & rB);
	int    Registration_ServiceReply(const RoundTripBlock & rB, const StyloQProtocol & rPack);
	int    KexClientRequest(RoundTripBlock & rB);
	int    Session_ClientRequest(RoundTripBlock & rB);
	int    Verification_ClientRequest(RoundTripBlock & rB);
	int    SearchGlobalIdentEntry(const SBinaryChunk & rIdent, StoragePacket * pPack);
	int    SearchSession(const SBinaryChunk & rOtherPublic, StoragePacket * pPack);
	int    Command_ClientRequest(RoundTripBlock & rB, const char * pCmdJson, SString & rReply);
	//
	// Descr: ��������� � ���� ������ ��������� ������ ��� ����, ����� � ��������� ��� ����� ���� �� 
	//   ������ ���������� ���������� � ��������������� ��������.
	//   ����� pPack (���� �� �������) ������ ��������� ��������� ����:
	//     SSecretTagPool::tagSessionPublicKey
	//     SSecretTagPool::tagSessionPrivateKey
	//     SSecretTagPool::tagSessionPublicKeyOther
	//     SSecretTagPool::tagSessionSecret
	//     SSecretTagPool::tagSvcIdent || SSecretTagPool::tagClientIdent � ����������� �� ����, �� ����� ������� �� ��������� //
	//   ������ ������ ����� �������� ���� SSecretTagPool::tagSessionPublicKeyOther
	//
	int    StoreSession(PPID * pID, StoragePacket * pPack, int use_ta);
	int    Dump();
private:
	int    ReadCurrentPacket(StoragePacket * pPack);
	int    PutPeerEntry(PPID * pID, StoragePacket * pPack, int use_ta);
	int    GetPeerEntry(PPID id, StoragePacket * pPack);
	//
	// OwnPeerEntry �������� ��������� ����: 
	//    SSecretTagPool::tagPrimaryRN - ��������� ���� (������� ��������� �����)
	//    SSecretTagPool::tagSvcIdent - ��������� �������������. ��� - �������������, ������� ���� �������������� � ����������������� ���������.
	//    SSecretTagPool::tagAG
	//    SSecretTagPool::tagFPI
	//
	int    GetOwnPeerEntry(StoragePacket * pPack);
	int    ExtractSessionFromPacket(const StoragePacket & rPack, SSecretTagPool & rSessCtx);
	enum {
		gcisfMakeSecret = 0x0001
	};
	//
	// Descr: ���������� ���������� ������������� ��� ������� � ��������������� pSvcId[svcIdLen].
	//   �������� ��� rOwnPool ������ ��������� SSecretTagPool::tagPrimaryRN � SSecretTagPool::tagAG.
	//   � �������������� ��� rPool ����������� ��������������� ���������� ������������� resultIdentTag (SSecretTagPool::tagClientIdent || SSecretTagPool::tagSvcIdent).
	//   SSecretTagPool::tagSvcIdent ����������� ��� ��������� ������������ ������ ���������� ��������� (� ��� ������� � ��� �������).
	//   ���� �������� flags �������� ������� ���� gcisfMakeSecret, �� ��� �� ������������ ������ (SSecretTagPool::tagSecret).
	//
	int    GeneratePublicIdent(const SSecretTagPool & rOwnPool, const SBinaryChunk & rSvcIdent, uint resultIdentTag, long flags, SSecretTagPool & rPool);
	StyloQSecTbl T;
};

int PPStyloQInterchange::GeneratePublicIdent(const SSecretTagPool & rOwnPool, const SBinaryChunk & rSvcIdent, uint resultIdentTag, long flags, SSecretTagPool & rPool)
{
	assert(oneof2(resultIdentTag, SSecretTagPool::tagSvcIdent, SSecretTagPool::tagClientIdent));
	int    ok = 1;
	SBinaryChunk prmrn;
	SBinaryChunk ag;
	THROW(rOwnPool.Get(SSecretTagPool::tagPrimaryRN, &prmrn));
	THROW(rOwnPool.Get(SSecretTagPool::tagAG, &ag));
	prmrn.Cat(rSvcIdent.PtrC(), rSvcIdent.Len());
	prmrn.Cat(ag.PtrC(), ag.Len());
	{
		binary160 hash = SlHash::Sha1(0, prmrn.PtrC(), prmrn.Len());
		assert(sizeof(hash) == 20);
		rPool.Put(resultIdentTag, &hash, sizeof(hash));
		if(flags & gcisfMakeSecret) {
			SBinaryChunk secret;
			secret.Cat(ag.PtrC(), ag.Len());
			secret.Cat(rSvcIdent.PtrC(), rSvcIdent.Len());
			binary160 secret_hash = SlHash::Sha1(0, secret.PtrC(), secret.Len());
			assert(sizeof(secret_hash) == 20);
			rPool.Put(SSecretTagPool::tagSecret, &secret_hash, sizeof(secret_hash));
		}
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::StoreSession(PPID * pID, StoragePacket * pPack, int use_ta)
{
	int    ok = 1;
	PPID   correspind_id = 0;
	StoragePacket org_pack;
	StoragePacket correspond_pack;
	PPTransaction tra(use_ta);
	THROW(tra);
	if(pPack) {
		int    correspond_type = 0;
		SBinaryChunk temp_chunk;
		SBinaryChunk cli_ident;
		SBinaryChunk svc_ident;
		THROW(pPack->Pool.Get(SSecretTagPool::tagSessionPublicKey, 0));
		THROW(pPack->Pool.Get(SSecretTagPool::tagSessionPrivateKey, 0));
		THROW(pPack->Pool.Get(SSecretTagPool::tagSessionSecret, 0));
		{
			pPack->Pool.Get(SSecretTagPool::tagClientIdent, &cli_ident);
			pPack->Pool.Get(SSecretTagPool::tagSvcIdent, &svc_ident);
			THROW(cli_ident.Len() || svc_ident.Len());
			THROW((cli_ident.Len() * svc_ident.Len()) == 0); // �� ������ ���� ���, ��� ������ ������������ � ���������� � ��������� ��������������
			THROW(oneof2(cli_ident.Len(), 0, 20));
			THROW(oneof2(svc_ident.Len(), 0, 20));
		}
		if(cli_ident.Len()) {
			THROW(SearchGlobalIdentEntry(cli_ident, &correspond_pack) > 0); // ������ ������������ ������ ���������������� �������
			correspond_type = kClient;
		}
		else if(svc_ident.Len()) {
			THROW(SearchGlobalIdentEntry(svc_ident, &correspond_pack) > 0); // ������ ������������ ������ ���������������� �������
			correspond_type = kForeignService;
		}
		else {
			assert(0); // ���� ������ ���� ��������� ��� �������
		}
		correspind_id = correspond_pack.Rec.ID;
		THROW(correspond_pack.Rec.Kind == correspond_type); // ����������������� ����� ������������� ���� ������ ���� ����������� 
		THROW(pPack->Pool.Get(SSecretTagPool::tagSessionPublicKeyOther, &temp_chunk));
		const binary160 bi = SlHash::Sha1(0, temp_chunk.PtrC(), temp_chunk.Len());
		assert(sizeof(bi) <= sizeof(pPack->Rec.BI));
		pPack->Rec.Kind = kSession;
		pPack->Rec.ID = 0;
		pPack->Rec.CorrespondID = correspond_pack.Rec.ID;
		memcpy(pPack->Rec.BI, &bi, sizeof(bi));
		if(!pID || !*pID) {
			// create session
			PPID   new_id = 0;
			if(correspond_pack.Rec.CorrespondID) {
				StoragePacket prev_sess_pack;
				if(GetPeerEntry(correspond_pack.Rec.CorrespondID, &prev_sess_pack) > 0) {
					if(prev_sess_pack.Rec.Kind != kSession) {
						// @problem �������� ���������� ������������������ ������ - �������� ������ ��������� ����� CorrespondID 
						// �� ������, �� ���������� �������.
						// ��� �� �����, �� ������ �� ������, ��������� ����� ������ ������ correspond_pack.Rec.CorrespondID
					}
					THROW(PutPeerEntry(&correspond_pack.Rec.CorrespondID, 0, 0));
				}
			}
			THROW(PutPeerEntry(&new_id, pPack, 0));
			correspond_pack.Rec.CorrespondID = new_id;
			assert(correspind_id > 0);
			THROW(PutPeerEntry(&correspind_id, &correspond_pack, 0));
			ASSIGN_PTR(pID, new_id);
		}
		else {
			// update session by id
			THROW(GetPeerEntry(*pID, &org_pack) > 0);
			//
			// �������� ������������ ��������� ���������� ���������� ������ 
			//
			THROW(org_pack.Rec.Kind == pPack->Rec.Kind);
			THROW(org_pack.Rec.CorrespondID == correspind_id);
			if(cli_ident.Len()) {
				THROW(org_pack.Pool.Get(SSecretTagPool::tagClientIdent, &temp_chunk) && temp_chunk == cli_ident);
			}
			else if(svc_ident.Len()) {
				THROW(org_pack.Pool.Get(SSecretTagPool::tagSvcIdent, &temp_chunk) && temp_chunk == svc_ident);
			}
			pPack->Rec.ID = org_pack.Rec.ID;
			THROW(PutPeerEntry(pID, pPack, 0));
		}
	}
	else {
		THROW(pID && *pID); // @error invalid arguments
		// remove session by id
		THROW(GetPeerEntry(*pID, &org_pack) > 0);
		correspind_id = org_pack.Rec.CorrespondID;
		if(correspind_id) {
			if(GetPeerEntry(correspind_id, &correspond_pack) > 0) {
				if(correspond_pack.Rec.CorrespondID == *pID) {
					// �������� ������ �� ��������� ������ � ������������������ ������
					correspond_pack.Rec.CorrespondID = 0;
					THROW(PutPeerEntry(&correspind_id, &correspond_pack, 0));
				}
				else {
					// @problem (���������� ����������� ������� ��������, �� ��������� ���������� ������ - �� ��� ����� ������� ������)
				}
			}
			else {
				// @problem (���������� ����������� ������� ��������, �� ��������� ���������� ������ - �� ��� ����� ������� ������)
			}
		}
		THROW(PutPeerEntry(pID, 0, 0));
	}
	THROW(tra.Commit());
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::ExtractSessionFromPacket(const StoragePacket & rPack, SSecretTagPool & rSessCtx)
{
	int    ok = 0;
	if(rPack.Rec.Kind == kSession) {
		SBinaryChunk public_key;
		SBinaryChunk private_key;
		SBinaryChunk sess_secret;
		if(rPack.Pool.Get(SSecretTagPool::tagSessionPrivateKey, &private_key)) {
			if(rPack.Pool.Get(SSecretTagPool::tagSessionPublicKey, &public_key)) {
				if(rPack.Pool.Get(SSecretTagPool::tagSessionSecret, &sess_secret)) {
					rSessCtx.Put(SSecretTagPool::tagSessionPrivateKey, private_key);
					rSessCtx.Put(SSecretTagPool::tagSessionPublicKey, public_key);
					rSessCtx.Put(SSecretTagPool::tagSessionSecret, sess_secret);
					ok = 1;
				}
			}
		}
	}
	return ok;
}

int PPStyloQInterchange::ReadCurrentPacket(StoragePacket * pPack)
{
	int    ok = -1;
	if(pPack) {
		SBuffer sbuf;
		SSerializeContext sctx;
		T.copyBufTo(&pPack->Rec);
		T.readLobData(T.VT, sbuf);
		T.destroyLobData(T.VT);
		THROW_SL(pPack->Pool.Serialize(-1, sbuf, &sctx));
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::Registration_ClientRequest(RoundTripBlock & rB)
{
	int    ok = 0;
	SString temp_buf;
	SBinaryChunk cli_ident;
	SBinaryChunk cli_secret;
	SBinaryChunk sess_secret;
	SBinaryChunk svc_ident;
	StyloQProtocol tp;
	SBinaryChunk __s;
	SBinaryChunk __v;
	THROW(rB.Other.Get(SSecretTagPool::tagSvcIdent, &svc_ident))
	//if(GeneratePublicIdent(rB.StP.Pool, svc_ident, SSecretTagPool::tagClientIdent, gcisfMakeSecret, rB.Sess)) {
	THROW(rB.Sess.Get(SSecretTagPool::tagClientIdent, &cli_ident));
	THROW(rB.Sess.Get(SSecretTagPool::tagSecret, &cli_secret));
	THROW(rB.Sess.Get(SSecretTagPool::tagSessionSecret, &sess_secret));
	temp_buf.EncodeMime64(cli_ident.PtrC(), cli_ident.Len());
	SlSRP::CreateSaltedVerificationKey2(SlSRP::SRP_SHA1, SlSRP::SRP_NG_8192, temp_buf, PTR8C(cli_secret.PtrC()), cli_secret.Len(), __s, __v, 0, 0);
	{
		SBuffer b;
		tp.StartWriting(PPSCMD_SQ_SRPREGISTER, StyloQProtocol::psubtypeForward);
		tp.P.Put(SSecretTagPool::tagClientIdent, cli_ident);
		/*
			@todo ����� ��� ���� �������� ��� �������
		*/
		tp.P.Put(SSecretTagPool::tagSrpVerifier, __v);
		tp.P.Put(SSecretTagPool::tagSrpVerifierSalt, __s);
		assert(tp.P.IsValid());
		THROW(tp.FinishWriting(&sess_secret));
		if(rB.P_Mqbc && rB.P_MqbRpe) {
			PPMqbClient::MessageProperties props;
			if(rB.P_Mqbc->Publish(rB.P_MqbRpe->ExchangeName, rB.P_MqbRpe->RoutingKey, &props, tp.constptr(), tp.GetAvailableSize())) {
				PPMqbClient::Envelope env;
				if(rB.P_Mqbc->ConsumeMessage(env, 500) > 0) {
					rB.P_Mqbc->Ack(env.DeliveryTag, 0);
					{
						PPID   id = 0;
						StoragePacket new_storage_pack;
						new_storage_pack.Rec.Kind = kForeignService;
						memcpy(new_storage_pack.Rec.BI, svc_ident.PtrC(), svc_ident.Len());
						new_storage_pack.Pool.Put(SSecretTagPool::tagSvcIdent, svc_ident);
						new_storage_pack.Pool.Put(SSecretTagPool::tagClientIdent, cli_ident);
						new_storage_pack.Pool.Put(SSecretTagPool::tagSrpVerifier, __v);
						new_storage_pack.Pool.Put(SSecretTagPool::tagSrpVerifierSalt, __s);
						THROW(PutPeerEntry(&id, &new_storage_pack, 1));
					}
					ok = 1;
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::Registration_ServiceReply(const RoundTripBlock & rB, const StyloQProtocol & rPack)
{
	int    ok = 1;
	PPID   id = 0;
	StoragePacket new_storage_pack;
	StoragePacket ex_storage_pack;
	SBinaryChunk cli_ident;
	SBinaryChunk cli_ident_other_for_test;
	SBinaryChunk srp_s;
	SBinaryChunk srp_v;
	THROW(rB.Other.Get(SSecretTagPool::tagClientIdent, &cli_ident_other_for_test));
	THROW(rPack.P.Get(SSecretTagPool::tagClientIdent, &cli_ident));
	assert(cli_ident_other_for_test == cli_ident);
	THROW(rPack.P.Get(SSecretTagPool::tagSrpVerifier, &srp_v));
	THROW(rPack.P.Get(SSecretTagPool::tagSrpVerifierSalt, &srp_s));
	if(SearchGlobalIdentEntry(cli_ident, &ex_storage_pack) > 0) {
		if(ex_storage_pack.Rec.Kind == kClient) {
			SBinaryChunk temp_bc;
			if(ex_storage_pack.Pool.Get(SSecretTagPool::tagClientIdent, &temp_bc) && temp_bc == cli_ident &&
				ex_storage_pack.Pool.Get(SSecretTagPool::tagSrpVerifier, &temp_bc) && temp_bc == srp_v &&
				ex_storage_pack.Pool.Get(SSecretTagPool::tagSrpVerifierSalt, &temp_bc) && temp_bc == srp_s) {
				// ������ ��� �������� ������ ��������� ����.
				ok = 1;
			}
			else {
				// ������� ������� �������� ������������������ c ������� ����������� �����������: � ����� ������ ��� - ������. ������ �� ����� ������ ���� ��������������� ������.
				// � ���������� �������� ��������.
				ok = 0;
			}
		}
		else {
			// ���� ������ - �� ������� ����������: ���������� ���� ���� � �������������� �������������� cli_ident.
			ok = 0;
		}
	}
	else {
		THROW(cli_ident.Len() <= sizeof(new_storage_pack.Rec.BI));
		new_storage_pack.Rec.Kind = kClient;
		memcpy(new_storage_pack.Rec.BI, cli_ident.PtrC(), cli_ident.Len());
		new_storage_pack.Pool.Put(SSecretTagPool::tagClientIdent, cli_ident);
		new_storage_pack.Pool.Put(SSecretTagPool::tagSrpVerifier, srp_v);
		new_storage_pack.Pool.Put(SSecretTagPool::tagSrpVerifierSalt, srp_s);
		THROW(PutPeerEntry(&id, &new_storage_pack, 1));
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::KexClientRequest(RoundTripBlock & rB)
{
	//
	// ������������ ������: ������������� ����� ���������� ��� ����������� ������� � ��������.
	//
	int    ok = -1;
	SString temp_buf;
	SString consume_tag;
	SBinaryChunk svc_acsp;
	SBinaryChunk svc_ident;
	PPMqbClient * p_mqbc = 0;
	THROW(rB.Other.Get(SSecretTagPool::tagSvcAccessPoint, &svc_acsp));
	THROW(rB.Other.Get(SSecretTagPool::tagSvcIdent, &svc_ident));
	{
		temp_buf.Z().CatN(static_cast<const char *>(svc_acsp.PtrC()), svc_acsp.Len());
		InetUrl url(temp_buf);
		SString host;
		THROW(url.GetComponent(InetUrl::cHost, 0, host));
		if(oneof2(url.GetProtocol(), InetUrl::protAMQP, InetUrl::protAMQPS)) {
			SBinaryChunk sess_pub_key;
			SBinaryChunk own_ident;
			SBinaryChunk other_sess_public;
			StyloQProtocol tp;
			PPMqbClient::RoutingParamEntry rpe; // ������� �� �������, � ������� ������ ���� ����� ��������
			PPMqbClient::RoutingParamEntry rpe_regular; // ������� �� �������, � ������� ������ ����� � ���� ��������
			PPMqbClient::MessageProperties props;
			PPMqbClient::Envelope env;
			PPMqbClient::InitParam mqip;
			THROW(PPMqbClient::SetupInitParam(mqip, "styloq", 0));
			int  fsks = FetchSessionKeys(rB.Sess, svc_ident);
			THROW(fsks);
			THROW(KexGenerateKeys(rB.Sess));
			THROW(rB.Sess.Get(SSecretTagPool::tagClientIdent, &own_ident)); // !
			//if(fsks == fsksNewSession) 
			THROW(rB.Sess.Get(SSecretTagPool::tagSessionPublicKey, &sess_pub_key));
			tp.StartWriting(PPSCMD_SQ_ACQUAINTANCE, StyloQProtocol::psubtypeForward);
			tp.P.Put(SSecretTagPool::tagClientIdent, own_ident);
			tp.P.Put(SSecretTagPool::tagSessionPublicKey, sess_pub_key);
			THROW(rpe.SetupStyloQRpc(sess_pub_key, svc_ident, mqip.ConsumeParamList.CreateNewItem()));
			THROW(p_mqbc = PPMqbClient::CreateInstance(mqip));
			THROW(tp.FinishWriting(0));
			THROW(p_mqbc->Publish(rpe.ExchangeName, rpe.RoutingKey, &props, tp.constptr(), tp.GetAvailableSize()));
			{
				const clock_t _c = clock();
				THROW(p_mqbc->ConsumeMessage(env, 1500) > 0);
				p_mqbc->Ack(env.DeliveryTag, 0);
				THROW(tp.Read(env.Msg, 0));
				THROW(tp.CheckRepError());
				THROW(tp.GetH().Type == PPSCMD_SQ_ACQUAINTANCE && tp.GetH().Flags & tp.hfAck);
				tp.P.Get(SSecretTagPool::tagSessionPublicKey, &other_sess_public);
				THROW(KexGenerageSecret(rB.Sess, tp.P));
				// ������ ���� ���������� ������ ���� � � ��� � � �������!
				THROW(rpe_regular.SetupStyloQRpc(sess_pub_key, other_sess_public, 0));
				// ������������� ������� RoundTrimBlock, ������� ��� ������������ ��� ����������� �������
				if(SetRoundTripBlockReplyValues(rB, p_mqbc, rpe_regular)) {
					p_mqbc = 0;
				}
				ok = 1;
			}
			//else if(fsks == fsksSessionBySvcId) {
			//}
		}
	}
	CATCHZOK
	ZDELETE(p_mqbc);
	return ok;
}

int PPStyloQInterchange::Session_ClientRequest(RoundTripBlock & rB)
{
	int    ok = 1;
	SString temp_buf;
	SBinaryChunk svc_ident;
	SBinaryChunk svc_acsp;
	SBinaryChunk sess_pub_key;
	SBinaryChunk other_sess_public;
	SBinaryChunk own_ident;
	PPMqbClient * p_mqbc = 0;
	THROW(rB.Other.Get(SSecretTagPool::tagSvcAccessPoint, &svc_acsp));
	THROW(rB.Other.Get(SSecretTagPool::tagSvcIdent, &svc_ident));
	THROW(rB.Sess.Get(SSecretTagPool::tagSessionPublicKey, &sess_pub_key));
	THROW(rB.Sess.Get(SSecretTagPool::tagSessionPublicKeyOther, &other_sess_public));
	THROW(rB.Sess.Get(SSecretTagPool::tagClientIdent, &own_ident)); // !
	{
		temp_buf.Z().CatN(static_cast<const char *>(svc_acsp.PtrC()), svc_acsp.Len());
		InetUrl url(temp_buf);
		SString host;
		THROW(url.GetComponent(InetUrl::cHost, 0, host));
		if(oneof2(url.GetProtocol(), InetUrl::protAMQP, InetUrl::protAMQPS)) {
			StyloQProtocol tp;
			PPMqbClient::RoutingParamEntry rpe; // ������� �� �������, � ������� ������ ���� ����� ��������
			PPMqbClient::RoutingParamEntry rpe_regular; // ������� �� �������, � ������� ������ ����� � ���� ��������
			PPMqbClient::MessageProperties props;
			PPMqbClient::Envelope env;
			PPMqbClient::InitParam mqip;
			THROW(PPMqbClient::SetupInitParam(mqip, "styloq", 0));
			tp.StartWriting(PPSCMD_SQ_SESSION, StyloQProtocol::psubtypeForward);
			tp.P.Put(SSecretTagPool::tagSessionPublicKey, sess_pub_key);
			tp.P.Put(SSecretTagPool::tagClientIdent, own_ident);
			THROW(rpe.SetupStyloQRpc(sess_pub_key, svc_ident, mqip.ConsumeParamList.CreateNewItem()));
			THROW(p_mqbc = PPMqbClient::CreateInstance(mqip));
			THROW(tp.FinishWriting(0));
			THROW(p_mqbc->Publish(rpe.ExchangeName, rpe.RoutingKey, &props, tp.constptr(), tp.GetAvailableSize()));				
			{
				THROW(p_mqbc->ConsumeMessage(env, 1500) > 0);
				p_mqbc->Ack(env.DeliveryTag, 0);
				THROW(tp.Read(env.Msg, 0));
				THROW(tp.CheckRepError());
				THROW(tp.GetH().Type == PPSCMD_SQ_SESSION && tp.GetH().Flags & tp.hfAck);
				// ���� ���������� ������ ���� � � ��� � � �������!
				THROW(rpe_regular.SetupStyloQRpc(sess_pub_key, other_sess_public, 0));
				// ������������� ������� RoundTrimBlock, ������� ��� ������������ ��� ����������� �������
				if(SetRoundTripBlockReplyValues(rB, p_mqbc, rpe_regular)) {
					p_mqbc = 0;
				}
			}
		}
	}
	CATCHZOK
	ZDELETE(p_mqbc);
	return ok;
}

int PPStyloQInterchange::Command_ClientRequest(RoundTripBlock & rB, const char * pCmdJson, SString & rReply)
{
	rReply.Z();
	int    ok = 1;
	SBinaryChunk sess_secret;
	THROW(rB.Sess.Get(SSecretTagPool::tagSessionSecret, &sess_secret));
	THROW(!isempty(pCmdJson));
	{
		PPMqbClient::MessageProperties props;
		PPMqbClient::Envelope env;
		StyloQProtocol tp;
		StyloQProtocol tp_reply;
		SBinaryChunk temp_bch;
		tp.StartWriting(PPSCMD_SQ_COMMAND, StyloQProtocol::psubtypeForward);
		temp_bch.Put(pCmdJson, sstrlen(pCmdJson));
		tp.P.Put(SSecretTagPool::tagRawData, temp_bch);
		THROW(tp.FinishWriting(&sess_secret));
		THROW(rB.P_Mqbc && rB.P_MqbRpe);
		THROW(rB.P_Mqbc->Publish(rB.P_MqbRpe->ExchangeName, rB.P_MqbRpe->RoutingKey, &props, tp.constptr(), tp.GetAvailableSize()));
		THROW(rB.P_Mqbc->ConsumeMessage(env, 1500) > 0);
		rB.P_Mqbc->Ack(env.DeliveryTag, 0);
		THROW(tp_reply.Read(env.Msg, &sess_secret));
		THROW(tp_reply.CheckRepError()); // ������ ������ ������: ����� �������
		THROW(tp_reply.GetH().Type == PPSCMD_SQ_COMMAND && tp_reply.GetH().Flags & tp.hfAck);
		if(tp_reply.P.Get(SSecretTagPool::tagRawData, &temp_bch)) {
			rReply.CatN(static_cast<const char *>(temp_bch.PtrC()), temp_bch.Len());
		}
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::SetRoundTripBlockReplyValues(RoundTripBlock & rB, PPMqbClient * pMqbc, const PPMqbClient::RoutingParamEntry & rRpe)
{
	int    ok = 1;
	if(pMqbc && rRpe.ExchangeName.NotEmpty() && rRpe.RoutingKey.NotEmpty()) {
		if(rB.P_Mqbc != pMqbc)
			ZDELETE(rB.P_Mqbc);
		rB.P_Mqbc = pMqbc;
		ZDELETE(rB.P_MqbRpe);
		rB.P_MqbRpe = new PPMqbClient::RoutingParamEntry(rRpe);
		//p_mqbc = 0;
	}
	else
		ok = 0;
	return ok;
}

int PPStyloQInterchange::Verification_ClientRequest(RoundTripBlock & rB)
{
	//
	// ��� - ������������ ������ � ������������ � ������������ ������������� ����� ������.
	// ��� ���� SRP-����������� �������������� ��� ����������! 
	//
	int    ok = 1;
	SString temp_buf;
	StoragePacket svc_pack;
	SBinaryChunk svc_ident;
	PPMqbClient * p_mqbc = 0;
	PPMqbClient::RoutingParamEntry rpe; // ������� �� �������, � ������� ������ ���� ����� ��������
	PPMqbClient::RoutingParamEntry * p_rpe = 0;
	PPMqbClient::RoutingParamEntry * p_rpe_init = 0; // if !0 then the first call to the service will be made through it, else through p_rpe
	bool   kex_generated = false;
	SBinaryChunk cli_ident;
	SBinaryChunk cli_secret;
	SBinaryChunk sess_pub_key;
	SBinaryChunk other_sess_public;
	SBinaryChunk _sess_secret;
	SBinaryChunk * p_sess_secret = 0;
	assert((rB.P_Mqbc && rB.P_MqbRpe) || (!rB.P_Mqbc && !rB.P_MqbRpe));
	if(rB.P_Mqbc && rB.P_MqbRpe) {
		p_mqbc = rB.P_Mqbc;
		p_rpe = rB.P_MqbRpe;
		p_rpe_init = p_rpe;
	}
	THROW(rB.Other.Get(SSecretTagPool::tagSvcIdent, &svc_ident));
	if(rB.Sess.Get(SSecretTagPool::tagSessionSecret, &_sess_secret)) {
		p_sess_secret = &_sess_secret;
	}
	else {
		THROW(KexGenerateKeys(rB.Sess));
		THROW(rB.Sess.Get(SSecretTagPool::tagSessionPublicKey, &sess_pub_key));
		kex_generated = true;
	}
	THROW(rB.Sess.Get(SSecretTagPool::tagClientIdent, &cli_ident));
	THROW(rB.Sess.Get(SSecretTagPool::tagSecret, &cli_secret));
	if(!p_mqbc) {
		SBinaryChunk svc_acsp;
		THROW(rB.Other.Get(SSecretTagPool::tagSvcAccessPoint, &svc_acsp));
		temp_buf.Z().CatN(static_cast<const char *>(svc_acsp.PtrC()), svc_acsp.Len());
		InetUrl url(temp_buf);
		SString host;
		THROW(url.GetComponent(InetUrl::cHost, 0, host));
		if(oneof2(url.GetProtocol(), InetUrl::protAMQP, InetUrl::protAMQPS)) {
			PPMqbClient::InitParam mqip;
			THROW(PPMqbClient::SetupInitParam(mqip, "styloq", 0));
			if(rpe.SetupStyloQRpc(sess_pub_key, svc_ident, mqip.ConsumeParamList.CreateNewItem())) {
				THROW(p_mqbc = PPMqbClient::CreateInstance(mqip));
				p_rpe_init = &rpe;
			}
		}
	}
	{
		int    srp_protocol_fault = 0; // ���� !0 �� �������� ������ � �����������. ����� ������ �������������� ����������� �������.
		StyloQProtocol tp;
		SBinaryChunk temp_chunk;
		SBinaryChunk __m; // M
		SBinaryChunk srp_s;
		PPMqbClient::RoutingParamEntry rpe_regular; // ������� �� �������, � ������� ������ ����� � ���� ��������
		PPMqbClient::MessageProperties props;
		PPMqbClient::Envelope env;
		char * p_auth_ident = 0;
		temp_buf.EncodeMime64(cli_ident.PtrC(), cli_ident.Len());
		SlSRP::User usr(SlSRP::SRP_SHA1, SlSRP::SRP_NG_8192, temp_buf, cli_secret.PtrC(), cli_secret.Len(), /*n_hex*/0, /*g_hex*/0);
		{
			temp_chunk.Z(); // � ���� ����� temp_chunk �������� ��� ������ A
			usr.StartAuthentication(&p_auth_ident, temp_chunk);
			// User -> Host: (ident, __a) 
			tp.StartWriting(PPSCMD_SQ_SRPAUTH, 0);
			//tp.P.Put(SSecretTagPool::tag)
			if(kex_generated) {
				assert(sess_pub_key.Len()); // �� ��� ������ ��� ������������� (see above)
				tp.P.Put(SSecretTagPool::tagSessionPublicKey, sess_pub_key);
			}
			tp.P.Put(SSecretTagPool::tagSrpA, temp_chunk);
			tp.P.Put(SSecretTagPool::tagClientIdent, cli_ident);
			assert(!p_sess_secret || p_sess_secret->Len());
			assert(p_sess_secret || tp.P.Get(SSecretTagPool::tagSessionPublicKey, 0));
			tp.FinishWriting(p_sess_secret);
		}
		THROW(p_mqbc->Publish(p_rpe_init->ExchangeName, p_rpe_init->RoutingKey, &props, tp.constptr(), tp.GetAvailableSize()));
		THROW(p_mqbc->ConsumeMessage(env, 1500) > 0);
		p_mqbc->Ack(env.DeliveryTag, 0);
		THROW(tp.Read(env.Msg, 0));
		THROW(tp.CheckRepError()); // ������ ������ ������: ����� ������� - ����������� �� ��������
		THROW(tp.GetH().Type == PPSCMD_SQ_SRPAUTH && tp.GetH().Flags & tp.hfAck);
		//
		tp.P.Get(SSecretTagPool::tagSessionPublicKey, &other_sess_public);
		THROW(KexGenerageSecret(rB.Sess, tp.P));
		THROW(rB.Sess.Get(SSecretTagPool::tagSessionSecret, &_sess_secret));
		// ������ ���� ���������� ������ ���� � � ��� � � �������!
		{
			temp_chunk.Z(); // // � ���� ����� temp_chunk �������� ��� ������ B
			tp.P.Get(SSecretTagPool::tagSrpB, &temp_chunk);
			tp.P.Get(SSecretTagPool::tagSrpVerifierSalt, &srp_s);
			usr.ProcessChallenge(srp_s, temp_chunk, __m);
		}
		THROW(rpe_regular.SetupStyloQRpc(sess_pub_key, other_sess_public, 0));
		if(!__m.Len()) { // @error User SRP-6a safety check violation
			srp_protocol_fault = 1;
			tp.StartWriting(PPSCMD_SQ_SRPAUTH_S2, StyloQProtocol::psubtypeForwardError);
			// ����� ����� � ��� ������ �����
		}
		else {
			// User -> Host: (bytes_M) 
			tp.StartWriting(PPSCMD_SQ_SRPAUTH_S2, 0);
			tp.P.Put(SSecretTagPool::tagSrpM, __m);
			tp.P.Put(SSecretTagPool::tagClientIdent, cli_ident);
		}
		THROW(tp.FinishWriting(0)); // �������� �� ��, ��� � ��� ���� ������ ���� ����������, ���� roundtrip ��������� ��� �������� �������
		THROW(p_mqbc->Publish(rpe_regular.ExchangeName, rpe_regular.RoutingKey, &props, tp.constptr(), tp.GetAvailableSize()));
		THROW(!srp_protocol_fault);
		THROW(p_mqbc->ConsumeMessage(env, 1500) > 0);
		p_mqbc->Ack(env.DeliveryTag, 0);
		THROW(tp.Read(env.Msg, 0));
		THROW(tp.CheckRepError()); // ������ ������ ������: ����� ������� - ����������� �� ��������
		THROW(tp.GetH().Type == PPSCMD_SQ_SRPAUTH_S2 && tp.GetH().Flags & tp.hfAck);
		{
			temp_chunk.Z(); // � ���� ����� temp_chunk �������� ��� ������ HAMK
			THROW(tp.P.Get(SSecretTagPool::tagSrpHAMK, &temp_chunk));
			THROW(temp_chunk.Len() == usr.GetSessionKeyLength());
			usr.VerifySession(static_cast<const uchar *>(temp_chunk.PtrC()));
			if(!usr.IsAuthenticated()) { // @error Server authentication failed
				srp_protocol_fault = 1;
				tp.StartWriting(PPSCMD_SQ_SRPAUTH_ACK, StyloQProtocol::psubtypeForwardError);
				// ����� ����� � ��� ������ �����
			}
			else {
				tp.StartWriting(PPSCMD_SQ_SRPAUTH_ACK, StyloQProtocol::psubtypeForward);
			}
			tp.FinishWriting(0); // �������� �� ��, ��� � ��� ���� ������ ���� ����������, ���� roundtrip ��������� ��� �������� �������
		}
		THROW(p_mqbc->Publish(rpe_regular.ExchangeName, rpe_regular.RoutingKey, &props, tp.constptr(), tp.GetAvailableSize()));
		// ��� ���� ����������� ���������. ���� ��� OK �� ������ ���� �� ��� �������, ���� ���, �� ����� ������� - ������� �� �����
		THROW(!srp_protocol_fault);
		// ������������� ������� RoundTrimBlock, ������� ��� ������������ ��� ����������� �������
		if(SetRoundTripBlockReplyValues(rB, p_mqbc, rpe_regular)) {
			p_mqbc = 0;
		}
		{
			PPID   sess_id = 0;
			StoragePacket sess_pack;
			// ������ ���� ��������� ��������� ������ ���� � ��������� ��� �� ����������� ����� ������� ���������
			//
			// �������� assert'��� (�� THROW) ����������� ��-�� ����, ��� �� ������ ���������� ��������, ����� ��
			// ������ � ���� ������� ���� � �������������� ��������� (�� ���� ��� ������������� THROW ������ ���� ���� ������� ����).
			assert(rB.Sess.Get(SSecretTagPool::tagSessionSecret, &temp_chunk));
			assert(temp_chunk == _sess_secret);
			assert(sess_pub_key.Len());
			assert(other_sess_public.Len());
			assert(rB.Sess.Get(SSecretTagPool::tagSessionPrivateKey, 0));
			sess_pack.Pool.Put(SSecretTagPool::tagSessionPublicKey, sess_pub_key);
			{
				rB.Sess.Get(SSecretTagPool::tagSessionPrivateKey, &temp_chunk);
				sess_pack.Pool.Put(SSecretTagPool::tagSessionPrivateKey, temp_chunk);
			}
			sess_pack.Pool.Put(SSecretTagPool::tagSessionPublicKeyOther, other_sess_public);
			sess_pack.Pool.Put(SSecretTagPool::tagSessionSecret, _sess_secret);
			sess_pack.Pool.Put(SSecretTagPool::tagSvcIdent, svc_ident);
			THROW(StoreSession(&sess_id, &sess_pack, 1));
		}
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::Dump()
{
	int    ok = 1;
	SString temp_buf;
	SString out_buf;
	PPGetFilePath(PPPATH_OUT, "styloqsec.dump", temp_buf);
	SFile f_out(temp_buf, SFile::mWrite);
	if(f_out.IsValid()) {
		StyloQSecTbl::Key0 k0;
		StoragePacket spack;
		SBinaryChunk chunk;
		MEMSZERO(k0);
		out_buf.Z().Cat("ID").Tab().Cat("Kind").Tab().Cat("CorrespondID").Tab().Cat("BI").Tab().Cat("SessExpiration");
		f_out.WriteLine(out_buf.CR());
		if(T.search(0, &k0, spFirst)) do {
			if(ReadCurrentPacket(&spack)) {
				temp_buf.Z().EncodeMime64(spack.Rec.BI, sizeof(spack.Rec.BI));
				out_buf.Z().Cat(spack.Rec.ID).Tab().Cat(spack.Rec.Kind).Tab().Cat(spack.Rec.CorrespondID).Tab().
					Cat(temp_buf).Tab().Cat(spack.Rec.SessExpiration, DATF_ISO8601|DATF_CENTURY, 0);
				f_out.WriteLine(out_buf.CR());
				uint32 cid = 0;
				out_buf.Z();
				for(size_t pp = 0; spack.Pool.Enum(&pp, &cid, &chunk) > 0;) {
					chunk.Mime64(temp_buf);
					out_buf.Tab().Cat(cid).CatChar('=').Cat(temp_buf).CR();
				}
				f_out.WriteLine(out_buf.CR());
			}
		} while(T.search(0, &k0, spNext));
	}
	return ok;
}

int PPStyloQInterchange::SearchGlobalIdentEntry(const SBinaryChunk & rIdent, StoragePacket * pPack)
{
	assert(rIdent.Len() > 0 && rIdent.Len() <= sizeof(StyloQSecTbl::Key1));
	int    ok = -1;
	StyloQSecTbl::Key1 k1;
	THROW(rIdent.Len() > 0 && rIdent.Len() <= sizeof(StyloQSecTbl::Key1));
	MEMSZERO(k1);
	memcpy(k1.BI, rIdent.PtrC(), rIdent.Len());
	if(T.search(1, &k1, spEq)) {
		THROW(ReadCurrentPacket(pPack));
		ok = 1;			
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::SearchSession(const SBinaryChunk & rOtherPublic, StoragePacket * pPack)
{
	int    ok = -1;
	const binary160 bi = SlHash::Sha1(0, rOtherPublic.PtrC(), rOtherPublic.Len());
	StyloQSecTbl::Key1 k1;
	MEMSZERO(k1);
	assert(sizeof(bi) <= sizeof(k1.BI));
	memcpy(&k1.BI, &bi, sizeof(bi));
	if(T.search(1, &k1, spEq)) {
		THROW(ReadCurrentPacket(pPack));
		ok = 1;			
	}
	else if(BTRNFOUND) {
		// PPERR_SQ_SESSNFOUND  ������������ ������ �� ���������� ����� �� �������
		ok = -1;
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::GetOwnPeerEntry(StoragePacket * pPack)
{
	int    ok = -1;
	StyloQSecTbl::Key2 k2;
	MEMSZERO(k2);
	k2.Kind = kNativeService;
	if(T.search(2, &k2, spGe) && T.data.Kind == kNativeService) {
		THROW(ReadCurrentPacket(pPack));
		ok = 1;		
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::GetPeerEntry(PPID id, StoragePacket * pPack)
{
	int    ok = -1;
	StyloQSecTbl::Key0 k0;
	k0.ID = id;
	if(T.search(0, &k0, spEq)) {
		THROW(ReadCurrentPacket(pPack));
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::PutPeerEntry(PPID * pID, StoragePacket * pPack, int use_ta)
{
	int    ok = 1;
	const  PPID outer_id = pID ? *pID : 0;
	if(pPack) {
		assert(oneof4(pPack->Rec.Kind, kNativeService, kForeignService, kClient, kSession));
	}
	SBuffer cbuf;
	SSerializeContext sctx;
	bool   do_destroy_lob = false;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(outer_id) {
			StoragePacket preserve_pack;
			THROW(GetPeerEntry(outer_id, &preserve_pack) > 0);
			if(pPack) {
				pPack->Rec.ID = outer_id;
				THROW_DB(T.rereadForUpdate(0, 0));
				T.copyBufFrom(&pPack->Rec);
				THROW_SL(pPack->Pool.Serialize(+1, cbuf, &sctx));
				THROW(T.writeLobData(T.VT, cbuf.GetBuf(0), cbuf.GetAvailableSize()));
				THROW_DB(T.updateRec());
				do_destroy_lob = true;
			}
			else {
				THROW_DB(T.rereadForUpdate(0, 0));
				THROW_DB(T.deleteRec());
			}
		}
		else if(pPack) {
			if(pPack->Rec.Kind == kNativeService) {
				// � ������� ����� ���� �� ����� ����� ������ ���� kNativeService
				THROW(GetOwnPeerEntry(0) < 0); // @error ������� �������� ������ ������ ���� native-service
			}
			pPack->Rec.ID = 0;
			T.copyBufFrom(&pPack->Rec);
			THROW_SL(pPack->Pool.Serialize(+1, cbuf, &sctx));
			THROW(T.writeLobData(T.VT, cbuf.GetBuf(0), cbuf.GetAvailableSize()));
			THROW_DB(T.insertRec(0, pID));		
			do_destroy_lob = true;
		}
		THROW(tra.Commit());
	}
	CATCHZOK
	if(do_destroy_lob)
		T.destroyLobData(T.VT);
	return ok;
}

int PPStyloQInterchange::SetupPeerInstance(PPID * pID, int use_ta)
{
	int    ok = -1;
	PPID   id = 0;
	BN_CTX * p_bn_ctx = 0;
	BIGNUM * p_rn = 0;
	SString temp_buf;
	StoragePacket ex_pack;
	StoragePacket new_pack;
	StoragePacket * p_pack_to_export = 0;
	SBinaryChunk public_ident;
	if(GetOwnPeerEntry(&ex_pack) > 0) {
		; // ������ ��� ����������
		p_pack_to_export = &ex_pack;
	}
	else {
		//
		// �������������: 
		//   -- ����������� ������� ��������� ����� (SSecretTagPool::tagPrimaryRN)
		//   -- GUID ��� ���������� SRN ��� ��������� ���������� �������������� �� �������������� ������� (SSecretTagPool::tagAG)
		//   -- ���������� �������� ������������� �������, ��� ��������� ������������ ���������� ��������������, �� ������������ � ������� (SSecretTagPool::tagFPI)
		//
		const   int primary_rn_bits_width = 1024;
		uint8   temp[2048];
		p_bn_ctx = BN_CTX_new();
		p_rn = BN_new();
		{
			const size_t seed_size = 128;
			SObfuscateBuffer(temp, seed_size);
			RAND_seed(temp, seed_size);
		}
		int rn_len;
		do {
			BN_priv_rand(p_rn, primary_rn_bits_width, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
			rn_len = BN_bn2bin(p_rn, temp);
			THROW(rn_len > 0);
		} while(rn_len < primary_rn_bits_width / 8);
		assert(rn_len == primary_rn_bits_width / 8);
		new_pack.Pool.Put(SSecretTagPool::tagPrimaryRN, temp, rn_len);
		//
		do {
			BN_priv_rand(p_rn, 160, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
			rn_len = BN_bn2bin(p_rn, temp);
			THROW(rn_len > 0);
		} while(rn_len < 20);
		assert(rn_len == 20);
		{
			const SBinaryChunk fpi(temp, rn_len);
			new_pack.Pool.Put(SSecretTagPool::tagFPI, fpi);
			new_pack.Pool.Put(SSecretTagPool::tagAG, &S_GUID(SCtrGenerate_), sizeof(S_GUID)); // Autogeneration
			{
				public_ident.Z();
				THROW(GeneratePublicIdent(new_pack.Pool, fpi, SSecretTagPool::tagSvcIdent, 0, new_pack.Pool));
				const int r = new_pack.Pool.Get(SSecretTagPool::tagSvcIdent, &public_ident);
				assert(r);
				assert(public_ident.Len() <= sizeof(new_pack.Rec.BI));
				memcpy(new_pack.Rec.BI, public_ident.PtrC(), public_ident.Len());
				new_pack.Rec.Kind = kNativeService;
			}
		}
		THROW(PutPeerEntry(&id, &new_pack, 1));
		{
			//
			// ������������ �����������
			// 
			SBinaryChunk c1;
			SBinaryChunk c2;
			const uint32 test_tag_list[] = { SSecretTagPool::tagPrimaryRN, SSecretTagPool::tagSvcIdent, SSecretTagPool::tagAG, SSecretTagPool::tagFPI };
			{
				StoragePacket test_pack;
				if(GetPeerEntry(id, &test_pack) > 0) {
					for(uint i = 0; i < SIZEOFARRAY(test_tag_list); i++) {
						uint32 _tag = test_tag_list[i];
						if(new_pack.Pool.Get(_tag, &c1) && test_pack.Pool.Get(_tag, &c2)) {
							assert(c1 == c2);
							if(_tag == SSecretTagPool::tagSvcIdent) {
								assert(memcmp(c1.PtrC(), test_pack.Rec.BI, c1.Len()) == 0);
							}
						}
						else {
							assert(0);
						}
					}
				}
				else {
					assert(0);
				}
			}
			{
				StoragePacket test_pack;		
				if(GetOwnPeerEntry(&test_pack) > 0) {
					for(uint i = 0; i < SIZEOFARRAY(test_tag_list); i++) {
						uint32 _tag = test_tag_list[i];
						if(new_pack.Pool.Get(_tag, &c1) && test_pack.Pool.Get(_tag, &c2)) {
							assert(c1 == c2);
						}
						else {
							assert(0);
						}
					}
				}
				else {
					assert(0);
				}
			}
			{
				//
				// ����� ��������� �������������� ������������ ����������� �������������� � ������� �� ������ � ���� �� 
				// �������������� �������.
				//
				const char * p_svc_ident_mime = "Wn7M3JuxUaDpiCHlWiIStn+YYkQ="; // pft
				SBinaryChunk svc_ident_test;
				const int r = svc_ident_test.FromMime64(p_svc_ident_mime);
				SSecretTagPool last_test_pool;
				assert(r);
				for(uint gi = 0; gi < 10; gi++) {
					SSecretTagPool test_pool;
					THROW(GeneratePublicIdent(new_pack.Pool, svc_ident_test, SSecretTagPool::tagClientIdent, gcisfMakeSecret, test_pool));
					if(gi > 0) {
						SBinaryChunk bch_test;
						SBinaryChunk last_bch_test;
						test_pool.Get(SSecretTagPool::tagClientIdent, &bch_test);
						last_test_pool.Get(SSecretTagPool::tagClientIdent, &last_bch_test);
						assert(bch_test.Len() > 0);
						assert(last_bch_test.Len() > 0);
						assert(bch_test == last_bch_test);
						test_pool.Get(SSecretTagPool::tagSecret, &bch_test);
						last_test_pool.Get(SSecretTagPool::tagSecret, &last_bch_test);
						assert(bch_test.Len() > 0);
						assert(last_bch_test.Len() > 0);
						assert(bch_test == last_bch_test);
					}
					last_test_pool = test_pool;
				}
			}
		}
		p_pack_to_export = &new_pack;
		ok = 1;
	}
	if(p_pack_to_export) {
		PPGetFilePath(PPPATH_OUT, "styloq-instance.txt", temp_buf);
		SFile f_out(temp_buf, SFile::mWrite);
		public_ident.Z();
		p_pack_to_export->Pool.Get(SSecretTagPool::tagSvcIdent, &public_ident);
		public_ident.Mime64(temp_buf);
		f_out.WriteLine(temp_buf.CR());
	}
	CATCHZOK
	BN_free(p_rn);
	BN_CTX_free(p_bn_ctx);
	return ok;
}
//
// Registration {
//
//
PPStyloQInterchange::PPStyloQInterchange()
{
}

PPStyloQInterchange::~PPStyloQInterchange()
{
}
	
int PPStyloQInterchange::MakeInvitation(const Invitation & rInv, SString & rInvitationData)
{
	int    ok = 1;
	SString temp_buf;
	rInvitationData.Z();
	// prefix SVCPID SVCCAP URL [SVCCMD [SVCCMDPARAM]]
	THROW(!rInv.SvcPublicId.IsZero() && rInv.AccessPoint.NotEmpty());
	assert(rInv.Command.NotEmpty() || rInv.CommandParam.IsEmpty());
	THROW(rInv.Command.NotEmpty() || rInv.CommandParam.IsEmpty());
	rInvitationData.CatChar('A'); // prefix
	temp_buf.EncodeMime64(&rInv.SvcPublicId, sizeof(rInv.SvcPublicId));
	rInvitationData.Cat(temp_buf);
	{
		temp_buf.Z().EncodeMime64(&rInv.Capabitities, sizeof(rInv.Capabitities));
		rInvitationData.CatChar('&').Cat(temp_buf);
	}
	temp_buf.Z().EncodeMime64(rInv.AccessPoint, rInv.AccessPoint.Len());
	rInvitationData.CatChar('&').Cat(temp_buf);
	if(rInv.Command.NotEmpty()) {
		temp_buf.Z().EncodeMime64(rInv.Command, rInv.Command.Len());
		rInvitationData.CatChar('&').Cat(temp_buf);
		if(rInv.CommandParam.NotEmpty()) {
			temp_buf.Z().EncodeMime64(rInv.CommandParam, rInv.CommandParam.Len());
			rInvitationData.CatChar('&').Cat(temp_buf);
		}
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::FetchSessionKeys(SSecretTagPool & rSessCtx, const SBinaryChunk & rForeignIdent)
{
	const  int ec_curve_name_id = NID_X9_62_prime256v1;
	int    status = fsksError;
	bool   do_generate_keys = true;
	StoragePacket pack;
	StoragePacket corr_pack;
	if(SearchGlobalIdentEntry(rForeignIdent, &pack) > 0) {
		if(pack.Rec.Kind == kSession) {
			if(ExtractSessionFromPacket(pack, rSessCtx)) {
				status = fsksSessionById;
				do_generate_keys = false;
			}
		}
		else if(pack.Rec.Kind == kForeignService) {
			if(pack.Rec.CorrespondID && GetPeerEntry(pack.Rec.CorrespondID, &corr_pack) > 0) {
				if(corr_pack.Rec.Kind == kSession) {
					if(ExtractSessionFromPacket(pack, rSessCtx)) {
						status = fsksSessionBySvcId;
						do_generate_keys = false;
					}
				}
				else {
					// @error ����������������� ������� ����� ���� ������ ������
				}
			}
		}
		else if(pack.Rec.Kind == kClient) {
			if(pack.Rec.CorrespondID && GetPeerEntry(pack.Rec.CorrespondID, &corr_pack) > 0) {
				if(corr_pack.Rec.Kind == kSession) {
					if(ExtractSessionFromPacket(pack, rSessCtx)) {
						status = fsksSessionByCliId;
						do_generate_keys = false;
					}
				}
				else {
					// @error ����������������� ������� ����� ���� ������ ������
				}
			}
		}
	}
	if(do_generate_keys) {
		status = fsksNewSession;
	}
	/*CATCH
		status = fsksError;
	ENDCATCH*/
	return status;
}

int PPStyloQInterchange::KexGenerateKeys(SSecretTagPool & rSessCtx)
{
	int    ok = 1;
	EC_KEY * p_key = 0;
	SBinaryChunk public_key;
	SBinaryChunk private_key;
	BN_CTX * p_bn_ctx = BN_CTX_new();
	THROW(p_key = EC_KEY_new_by_curve_name(ec_curve_name_id)); // Failed to create key curve
	THROW(EC_KEY_generate_key(p_key) == 1); // Failed to generate key
	{
		const EC_GROUP * p_ecg = EC_KEY_get0_group(p_key);
		const EC_POINT * p_public_ecpt = EC_KEY_get0_public_key(p_key);
		const BIGNUM   * p_private_bn = EC_KEY_get0_private_key(p_key);
		THROW(p_private_bn);
		const size_t octlen = EC_POINT_point2oct(p_ecg, p_public_ecpt, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, p_bn_ctx);
		THROW(public_key.Ensure(octlen));
		EC_POINT_point2oct(p_ecg, p_public_ecpt, POINT_CONVERSION_UNCOMPRESSED, static_cast<uchar *>(public_key.Ptr()), public_key.Len(), p_bn_ctx);
		int bn_len = BN_num_bytes(p_private_bn);
		THROW(private_key.Ensure(bn_len));
		BN_bn2bin(p_private_bn, static_cast<uchar *>(private_key.Ptr()));
	}
	rSessCtx.Put(SSecretTagPool::tagSessionPrivateKey, private_key);
	rSessCtx.Put(SSecretTagPool::tagSessionPublicKey, public_key);
	CATCHZOK
	EC_KEY_free(p_key);
	BN_CTX_free(p_bn_ctx);
	return ok;
}

int PPStyloQInterchange::KexGenerageSecret(SSecretTagPool & rSessCtx, const SSecretTagPool & rOtherCtx)
{
	int    ok = 1;
	SBinaryChunk my_public;
	SBinaryChunk other_public;
	SBinaryChunk my_private;
	BN_CTX * p_bn_ctx = 0;
	EC_POINT * p_public_other = 0;
	EC_KEY * p_private_my = 0;
	BIGNUM * p_private_bn = 0;//EC_KEY_get0_private_key(p_key);
	void * p_secret = 0;
	THROW(rOtherCtx.Get(SSecretTagPool::tagSessionPublicKey, &other_public));
	THROW(rSessCtx.Get(SSecretTagPool::tagSessionPublicKey, &my_public));
	THROW(rSessCtx.Get(SSecretTagPool::tagSessionPrivateKey, &my_private));
	assert(other_public.Len() && my_public.Len() && my_private.Len());
	{
		p_bn_ctx = BN_CTX_new();
		p_private_my = EC_KEY_new_by_curve_name(ec_curve_name_id);
		const EC_GROUP * p_ecg2 = EC_KEY_get0_group(p_private_my);
		p_public_other = EC_POINT_new(p_ecg2);
		p_private_bn = BN_new();//EC_KEY_get0_private_key(p_key);
		EC_POINT_oct2point(p_ecg2, p_public_other, static_cast<uchar *>(other_public.Ptr()), other_public.Len(), p_bn_ctx);
		BN_bin2bn(static_cast<const uchar *>(my_private.PtrC()), my_private.Len(), p_private_bn);
		EC_KEY_set_private_key(p_private_my, p_private_bn);
		const int field_size = EC_GROUP_get_degree(EC_KEY_get0_group(p_private_my));
		size_t secret_len = (field_size + 7) / 8;
		THROW(p_secret = (uchar *)OPENSSL_malloc(secret_len)); // Failed to allocate memory for secret
		secret_len = ECDH_compute_key(p_secret, secret_len, p_public_other, p_private_my, NULL);
		THROW(secret_len > 0);
		rSessCtx.Put(SSecretTagPool::tagSessionSecret, p_secret, secret_len);
	}
	CATCHZOK
	BN_free(p_private_bn);
	EC_POINT_free(p_public_other);
	EC_KEY_free(p_private_my);
	BN_CTX_free(p_bn_ctx);
	OPENSSL_free(p_secret);
	return ok;
}

int PPStyloQInterchange::InitRoundTripBlock(RoundTripBlock & rB)
{
	int    ok = 1;
	SBinaryChunk svc_ident;
	SBinaryChunk temp_bch;
	THROW(GetOwnPeerEntry(&rB.StP) > 0);
	if(rB.Other.Get(SSecretTagPool::tagSvcIdent, &svc_ident)) {
		StoragePacket svc_pack;
		if(SearchGlobalIdentEntry(svc_ident, &svc_pack) > 0) {
			if(svc_pack.Rec.Kind == kForeignService) {
				rB.InnerSvcID = svc_pack.Rec.ID;
				if(svc_pack.Rec.CorrespondID) {
					StoragePacket corr_pack;
					if(GetPeerEntry(svc_pack.Rec.CorrespondID, &corr_pack) > 0) {
						if(corr_pack.Rec.Kind == kSession) {
							LDATETIME _now = getcurdatetime_();
							if(!corr_pack.Rec.SessExpiration || cmp(corr_pack.Rec.SessExpiration, _now) > 0) {
								LongArray cid_list;
								svc_pack.Pool.Get(SSecretTagPool::tagClientIdent, &temp_bch);
								rB.Sess.Put(SSecretTagPool::tagClientIdent, temp_bch);
								cid_list.addzlist(SSecretTagPool::tagSessionPrivateKey, SSecretTagPool::tagSessionPublicKey, SSecretTagPool::tagSessionSecret, 
									SSecretTagPool::tagSvcIdent, SSecretTagPool::tagSessionPublicKeyOther, 0);
								THROW_SL(rB.Sess.CopyFrom(corr_pack.Pool, cid_list, true));
								rB.InnerSessID = corr_pack.Rec.ID;
							}
						}
						else {
							// @error ���-�� �� ��� � ����� ������ ��� � ����������: ������ ���� �� ������!
						}
					}
				}
				else {
					THROW(GeneratePublicIdent(rB.StP.Pool, svc_ident, SSecretTagPool::tagClientIdent, gcisfMakeSecret, rB.Sess));
				}
			}
			else {
				// @error ���-�� �� ��� � ����� ������ ��� � ����������: ������ ���� �� ������!
			}
		}				
		else {
			THROW(GeneratePublicIdent(rB.StP.Pool, svc_ident, SSecretTagPool::tagClientIdent, gcisfMakeSecret, rB.Sess));
		}
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::KexServiceReply(SSecretTagPool & rSessCtx, SSecretTagPool & rCli/*RoundTripBlock & rB*/)
{
	int    ok = 1;
	SBinaryChunk cli_acsp;
	SBinaryChunk cli_ident;
	rCli.Get(SSecretTagPool::tagSvcAccessPoint, &cli_acsp);
	THROW(rCli.Get(SSecretTagPool::tagClientIdent, &cli_ident));
	int  fsks = FetchSessionKeys(rSessCtx, cli_ident);
	THROW(fsks);
	THROW(KexGenerateKeys(rSessCtx));
	//if(fsks == fsksNewSession) 
	{
		SBinaryChunk my_public;
		rSessCtx.Get(SSecretTagPool::tagSessionPublicKey, &my_public);
		THROW(my_public.Len());
		THROW(KexGenerageSecret(rSessCtx, rCli));
	}
	CATCHZOK
	return ok;
}
//

int Test_PPStyloQInterchange_Invitation()
{
	int    ok = 1;
	int    debug_flag = 0;
	SString temp_buf;
	PPStyloQInterchange ic;
	//
	PPIniFile ini_file;
	PPID   own_peer_id = 0;
	StyloQProtocol::Test();
	int    spir = ic.SetupPeerInstance(&own_peer_id, 1);
	temp_buf.Z();
	ini_file.Get(PPINISECT_CONFIG, "styloqtestside", temp_buf);
	ic.Dump(); // @debug
	if(temp_buf.IsEqiAscii("server")) {
		PPStyloQInterchange::RunStyloQServer();
	}
	else if(temp_buf.IsEqiAscii("client")) {
		const char * p_svc_ident_mime = "Lkekoviu1J2nw1O7/R66LYvpAtA="; // pft
		SBinaryChunk svc_ident;
		if(svc_ident.FromMime64(p_svc_ident_mime)) {
			//SSecretTagPool svc_pool;
			//SSecretTagPool sess_pool;
			PPStyloQInterchange::RoundTripBlock rtb(svc_ident.PtrC(), svc_ident.Len(), "amqp://213.166.70.221");
			if(ic.InitRoundTripBlock(rtb)) {
				//svc_pool.Put(SSecretTagPool::tagSvcIdent, svc_ident);
				//const char * p_accsp = "amqp://213.166.70.221";
				//svc_pool.Put(SSecretTagPool::tagSvcAccessPoint, p_accsp, strlen(p_accsp)+1);
				/* (������� ��������, �� ��� ���� �������� �����������) */
				if(rtb.InnerSessID) {
					if(ic.Session_ClientRequest(rtb) > 0) {
						SString cmd_buf;
						SString cmd_reply_buf;
						SJson * p_query = new SJson(SJson::tOBJECT);
						SJson * p_reply = 0;
						p_query->Insert("cmd", json_new_string("ECHO"));
						p_query->Insert("arg1", json_new_string("arg1-value"));
						p_query->Insert("arg2", json_new_string("arg2-value"));
						json_tree_to_string(p_query, cmd_buf);
						if(ic.Command_ClientRequest(rtb, cmd_buf, cmd_reply_buf)) {
							SString svc_result;
							int  reply_is_ok = 0;
							if(json_parse_document(&p_reply, cmd_reply_buf.cptr()) == JSON_OK) {
								for(SJson * p_cur = p_reply; p_cur; p_cur = p_cur->P_Next) {
									if(p_cur->Type == SJson::tOBJECT) {								
										for(const SJson * p_obj = p_cur->P_Child; p_obj; p_obj = p_obj->P_Next) {
											if(p_obj->Text.IsEqiAscii("reply")) {
												if(p_obj->P_Child->Text == "ECHO")
													reply_is_ok++;
											}
											else if(p_obj->Text.IsEqiAscii("arg1")) {
												if(p_obj->P_Child->Text == "arg1-value")
													reply_is_ok++;
											}
											else if(p_obj->Text.IsEqiAscii("arg2")) {
												if(p_obj->P_Child->Text == "arg2-value")
													reply_is_ok++;
											}
											else if(p_obj->Text.IsEqiAscii("result")) {
												svc_result = p_obj->P_Child->Text;
											}
										}
									}
								}
							}
							debug_flag = 1;
						}
						delete p_query;
						delete p_reply;
					}
				}
				else if(rtb.InnerSvcID) {
					if(ic.Verification_ClientRequest(rtb) > 0) {
						debug_flag = 1;
					}
				}
				else {
					if(ic.KexClientRequest(rtb) > 0) {
						ic.Registration_ClientRequest(rtb);
					}
				}
			}
		}
	}
#if 0 // @model {
	//__KeyGenerationEc();
	//
	{
		PPStyloQInterchange::Invitation inv_source;
		PPStyloQInterchange::Invitation inv_result;
		inv_source.SvcPublicId.Generate();
		inv_source.AccessPoint = "AMQP://192.168.0.1/test";
		inv_source.Capabitities = 0;
		inv_source.Command = "comein";
		inv_source.CommandParam = "yourpromo=100";
		ic.MakeInvitation(inv_source, temp_buf);
		if(!ic.AcceptInvitation(temp_buf, inv_result))
			ok = 0;
		else if(!inv_result.IsEqual(inv_source))
			ok = 0;
		{
			PPBarcode::BarcodeImageParam bip;
			bip.ColorFg = SClrBlack;
			bip.ColorBg = SClrWhite;
			bip.Code = temp_buf;
			bip.Std = BARCSTD_QR;
			bip.Size.Set(400, 400);
			bip.OutputFormat = SFileFormat::Png;
			PPGetFilePath(PPPATH_OUT, "styloq-invitation.png", bip.OutputFileName);
			if(PPBarcode::CreateImage(bip)) {
				//
				TSCollection <PPBarcode::Entry> bce_list;
				if(PPBarcode::RecognizeImage(bip.OutputFileName, bce_list)) {
					for(uint i = 0; i < bce_list.getCount(); i++) {
						temp_buf = bce_list.at(i)->Code;
					}
				}
			}
		}
	}
	{
		PPStyloQInterchange::Invitation inv_source;
		PPStyloQInterchange::Invitation inv_result;
		inv_source.SvcPublicId.Generate();
		inv_source.AccessPoint = "https://dummy-host.com";
		inv_source.Capabitities = 0;
		ic.MakeInvitation(inv_source, temp_buf);
		if(!ic.AcceptInvitation(temp_buf, inv_result))
			ok = 0;
		else if(!inv_result.IsEqual(inv_source))
			ok = 0;
	}
	{
		PPStyloQInterchange::Invitation inv_source;
		PPStyloQInterchange::Invitation inv_result;
		inv_source.SvcPublicId.Generate();
		inv_source.AccessPoint = "https://riptutorial.com/openssl/example/16738/save-private-key";
		inv_source.Capabitities = 0;
		ic.MakeInvitation(inv_source, temp_buf);
		if(!ic.AcceptInvitation(temp_buf, inv_result))
			ok = 0;
		else if(!inv_result.IsEqual(inv_source))
			ok = 0;
	}
	{
		PPStyloQInterchange::Invitation inv_source;
		PPStyloQInterchange::Invitation inv_result;
		inv_source.SvcPublicId.Generate();
		inv_source.AccessPoint = "https://riptutorial.com/openssl/example/16738/save-private-key";
		inv_source.Capabitities = 0;
		inv_source.Command = "some-command";
		inv_source.CommandParam = "xyz";
		ic.MakeInvitation(inv_source, temp_buf);
		if(!ic.AcceptInvitation(temp_buf, inv_result))
			ok = 0;
		else if(!inv_result.IsEqual(inv_source))
			ok = 0;
	}
#endif // } 0
	return ok;
}

class StyloQServerSession : public PPThread {
	PPStyloQInterchange * P_Ic;
public:
	StyloQServerSession(const DbLoginBlock & rLB, const PPMqbClient::InitParam & rMqbParam, const StyloQProtocol & rTPack) : 
		PPThread(kStyloQSession, 0, 0), LB(rLB), StartUp_MqbParam(rMqbParam), TPack(rTPack), P_Ic(0)
	{
		StartUp_MqbParam.ConsumeParamList.freeAll();
	}
	~StyloQServerSession()
	{
		delete P_Ic;
		Logout();
	}
	int SrpAuth()
	{
		int    ok = 1;
		int    debug_mark = 0;
		PPMqbClient * p_mqbc = 0;
		//PPStyloQInterchange * p_ic = 0;
		PPMqbClient::MessageProperties props;
		PPMqbClient::RoutingParamEntry rpe;
		StyloQProtocol tp;
		StyloQProtocol reply_tp;
		PPMqbClient::Envelope env;
		PPID   id = 0; // ���������� ������������� ������ ������� � DBMS
		SString temp_buf;
		PPStyloQInterchange::StoragePacket storage_pack;
		SBinaryChunk other_public;
		SBinaryChunk cli_ident;
		SBinaryChunk my_public;
		SBinaryChunk srp_s;
		SBinaryChunk srp_v;
		SBinaryChunk __a; // A
		SBinaryChunk __b; // B
		SBinaryChunk __m; // M
		SBinaryChunk temp_bc;
		THROW(TPack.P.Get(SSecretTagPool::tagSessionPublicKey, &other_public));
		THROW(TPack.P.Get(SSecretTagPool::tagClientIdent, &cli_ident));
		THROW(Login()); // @error (Inner service error: unable to login
		THROW(SETIFZ(P_Ic, new PPStyloQInterchange));
		B.Other.Put(SSecretTagPool::tagClientIdent, cli_ident);
		THROW(P_Ic->KexServiceReply(B.Sess, TPack.P));
		B.Sess.Get(SSecretTagPool::tagSessionPublicKey, &my_public);
		assert(my_public.Len());
		reply_tp.StartWriting(PPSCMD_SQ_SRPAUTH, StyloQProtocol::psubtypeReplyOk);
		THROW(TPack.P.Get(SSecretTagPool::tagSrpA, &__a));
		THROW(TPack.P.Get(SSecretTagPool::tagClientIdent, &cli_ident));
		THROW(P_Ic->SearchGlobalIdentEntry(cli_ident, &storage_pack) > 0);
		THROW(storage_pack.Rec.Kind == PPStyloQInterchange::kClient);
		THROW(storage_pack.Pool.Get(SSecretTagPool::tagClientIdent, &temp_bc) && temp_bc == cli_ident); // @error (I don't know you)
		THROW(storage_pack.Pool.Get(SSecretTagPool::tagSrpVerifier, &srp_v) && storage_pack.Pool.Get(SSecretTagPool::tagSrpVerifierSalt, &srp_s)); // @error (I havn't got your registration data)
		temp_buf.EncodeMime64(cli_ident.PtrC(), cli_ident.Len());
		{
			SlSRP::Verifier srp_ver(SlSRP::SRP_SHA1, SlSRP::SRP_NG_8192, temp_buf, srp_s, srp_v, __a, __b, /*n_hex*/0, /*g_hex*/0);
			const uchar * p_bytes_HAMK = 0;
			THROW(__b.Len()); // @error (Verifier SRP-6a safety check violated)
			// Host -> User: (bytes_s, bytes_B) 
			reply_tp.P.Put(SSecretTagPool::tagSrpB, __b);
			reply_tp.P.Put(SSecretTagPool::tagSrpVerifierSalt, srp_s);
			//}
			reply_tp.P.Put(SSecretTagPool::tagSessionPublicKey, my_public);
			props.Z();
			assert(StartUp_MqbParam.ConsumeParamList.getCount() == 0);
			THROW(rpe.SetupStyloQRpc(my_public, other_public, StartUp_MqbParam.ConsumeParamList.CreateNewItem()));
			THROW(reply_tp.FinishWriting(0));
			THROW(p_mqbc = PPMqbClient::CreateInstance(StartUp_MqbParam));
			THROW(p_mqbc->Publish(rpe.ExchangeName, rpe.RoutingKey, &props, reply_tp.constptr(), reply_tp.GetAvailableSize()));
			//do_process_loop = true;
			THROW(p_mqbc->ConsumeMessage(env, 1500) > 0);
			p_mqbc->Ack(env.DeliveryTag, 0);
			THROW(tp.Read(env.Msg, 0));
			THROW(tp.CheckRepError());
			THROW(tp.GetH().Type == PPSCMD_SQ_SRPAUTH_S2);
			THROW(tp.P.Get(SSecretTagPool::tagSrpM, &__m));
			srp_ver.VerifySession(static_cast<const uchar *>(__m.PtrC()), &p_bytes_HAMK);
			THROW(p_bytes_HAMK); // @error User authentication failed!
			{
				// Host -> User: (HAMK) 
				const SBinaryChunk srp_hamk(p_bytes_HAMK, srp_ver.GetSessionKeyLength());
				reply_tp.StartWriting(PPSCMD_SQ_SRPAUTH_S2, StyloQProtocol::psubtypeReplyOk);
				reply_tp.P.Put(SSecretTagPool::tagSrpHAMK, srp_hamk);
				reply_tp.FinishWriting(0);
				THROW(p_mqbc->Publish(rpe.ExchangeName, rpe.RoutingKey, &props, reply_tp.constptr(), reply_tp.GetAvailableSize()));
				// ��������� ��������� �� �������: ���� �� ������ OK - ������� ����������� �����������
				THROW(p_mqbc->ConsumeMessage(env, 1500) > 0);
				p_mqbc->Ack(env.DeliveryTag, 0);
				THROW(tp.Read(env.Msg, 0));
				THROW(tp.CheckRepError());
				THROW(tp.GetH().Type == PPSCMD_SQ_SRPAUTH_ACK);
				//
				// ������ ��� - ����������� ���������. ��������� ������ � ������ ����� ����� �������� ��������� �� �������
				{
					PPID   sess_id = 0;
					SBinaryChunk temp_chunk;
					SBinaryChunk _sess_secret;
					PPStyloQInterchange::StoragePacket sess_pack;
					// ������ ���� ��������� ��������� ������ ���� � ��������� ��� �� ����������� ����� ������� ���������
					//
					// �������� assert'��� (�� THROW) ����������� ��-�� ����, ��� �� ������ ���������� ��������, ����� ��
					// ������ � ���� ������� ���� � �������������� ��������� (�� ���� ��� ������������� THROW ������ ���� ���� ������� ����).
					B.Sess.Get(SSecretTagPool::tagSessionSecret, &_sess_secret);
					assert(_sess_secret.Len());
					assert(my_public.Len());
					assert(other_public.Len());
					assert(B.Sess.Get(SSecretTagPool::tagSessionPrivateKey, 0));
					sess_pack.Pool.Put(SSecretTagPool::tagSessionPublicKey, my_public);
					{
						B.Sess.Get(SSecretTagPool::tagSessionPrivateKey, &temp_chunk);
						sess_pack.Pool.Put(SSecretTagPool::tagSessionPrivateKey, temp_chunk);
					}
					sess_pack.Pool.Put(SSecretTagPool::tagSessionPublicKeyOther, other_public);
					sess_pack.Pool.Put(SSecretTagPool::tagSessionSecret, _sess_secret);
					sess_pack.Pool.Put(SSecretTagPool::tagClientIdent, cli_ident);
					THROW(P_Ic->StoreSession(&sess_id, &sess_pack, 1));
				}
				if(P_Ic->SetRoundTripBlockReplyValues(B, p_mqbc, rpe)) {
					p_mqbc = 0;
				}
			}
		}
		CATCHZOK
		delete p_mqbc;
		return ok;
	}
	int Acquaintance()
	{
		int    ok = 1;
		PPMqbClient * p_mqbc = 0;
		SBinaryChunk other_public;
		SBinaryChunk cli_ident;
		SBinaryChunk my_public;
		StyloQProtocol reply_tp;
		PPMqbClient::RoutingParamEntry rpe;
		PPMqbClient::MessageProperties props;
		THROW(TPack.P.Get(SSecretTagPool::tagSessionPublicKey, &other_public));
		THROW(TPack.P.Get(SSecretTagPool::tagClientIdent, &cli_ident));
		assert(other_public.Len());
		assert(cli_ident.Len());
		THROW(Login());
		assert(P_Ic == 0);
		THROW_SL(P_Ic = new PPStyloQInterchange);
		B.Other.Put(SSecretTagPool::tagClientIdent, cli_ident);
		THROW(P_Ic->KexServiceReply(B.Sess, TPack.P));
		THROW(B.Sess.Get(SSecretTagPool::tagSessionPublicKey, &my_public));
		assert(my_public.Len());
		reply_tp.StartWriting(PPSCMD_SQ_ACQUAINTANCE, StyloQProtocol::psubtypeReplyOk);
		reply_tp.P.Put(SSecretTagPool::tagSessionPublicKey, my_public);
		props.Z();
		assert(StartUp_MqbParam.ConsumeParamList.getCount() == 0);
		THROW(rpe.SetupStyloQRpc(my_public, other_public, StartUp_MqbParam.ConsumeParamList.CreateNewItem()));
		THROW(reply_tp.FinishWriting(0));
		THROW(p_mqbc = PPMqbClient::CreateInstance(StartUp_MqbParam));
		THROW(p_mqbc->Publish(rpe.ExchangeName, rpe.RoutingKey, &props, reply_tp.constptr(), reply_tp.GetAvailableSize()));
		if(P_Ic->SetRoundTripBlockReplyValues(B, p_mqbc, rpe)) {
			p_mqbc = 0;
		}
		CATCHZOK
		delete p_mqbc;
		return ok;
	}
	int Session()
	{
		int    ok = 1;
		PPMqbClient * p_mqbc = 0;
		SBinaryChunk temp_chunk;
		SBinaryChunk other_public;
		SBinaryChunk my_public;
		SBinaryChunk cli_ident;
		SBinaryChunk sess_secret;
		SBinaryChunk * p_sess_secret = 0;
		StyloQProtocol reply_tp;
		PPMqbClient::RoutingParamEntry rpe;
		PPMqbClient::MessageProperties props;
		PPStyloQInterchange::StoragePacket pack;
		THROW_PP(TPack.P.Get(SSecretTagPool::tagSessionPublicKey, &other_public), PPERR_SQ_INPHASNTSESSPUBKEY); // PPERR_SQ_INPHASNTSESSPUBKEY �������� ������ ������ �� �������� ���������� �����
		THROW_PP(TPack.P.Get(SSecretTagPool::tagClientIdent, &cli_ident), PPERR_SQ_INPHASNTCLIIDENT);           // PPERR_SQ_INPHASNTCLIIDENT   �������� ������ ������ �� �������� �������������� �����������
		assert(other_public.Len());
		assert(cli_ident.Len());
		THROW(Login());
		assert(P_Ic == 0);
		THROW_SL(P_Ic = new PPStyloQInterchange);
		THROW(P_Ic->SearchSession(other_public, &pack) > 0); 
		THROW_PP(pack.Pool.Get(SSecretTagPool::tagSessionPublicKey, &my_public), PPERR_SQ_INRSESSHASNTPUBKEY); // PPERR_SQ_INRSESSHASNTPUBKEY      ����������� ������ �� �������� ���������� �����
		THROW_PP(pack.Pool.Get(SSecretTagPool::tagSessionPublicKeyOther, &temp_chunk), PPERR_SQ_INRSESSHASNTOTHERPUBKEY); // PPERR_SQ_INRSESSHASNTOTHERPUBKEY ����������� ������ �� �������� ���������� ����� �����������
		THROW_PP(temp_chunk == other_public, PPERR_SQ_INRSESPUBKEYNEQTOOTR);         // PPERR_SQ_INRSESPUBKEYNEQTOOTR   ��������� ���� ����������� ������ �� ����� ����������� �� �����������
		THROW_PP(pack.Pool.Get(SSecretTagPool::tagClientIdent, &temp_chunk), PPERR_SQ_INRSESSHASNTCLIIDENT); // PPERR_SQ_INRSESSHASNTCLIIDENT   ����������� ������ �� �������� �������������� �����������
		THROW_PP(temp_chunk == cli_ident, PPERR_SQ_INRSESCLIIDENTNEQTOOTR);          // PPERR_SQ_INRSESCLIIDENTNEQTOOTR �������������� ����������� ����������� ������ �� ����� ����������� �� �����������
		THROW_PP(pack.Pool.Get(SSecretTagPool::tagSessionSecret, &sess_secret), PPERR_SQ_INRSESSHASNTSECRET); // PPERR_SQ_INRSESSHASNTSECRET     ����������� ������ �� �������� ������� 
		B.Sess.Put(SSecretTagPool::tagSessionSecret, sess_secret);
		p_sess_secret = &sess_secret;
		B.Other.Put(SSecretTagPool::tagClientIdent, cli_ident);
		THROW(rpe.SetupStyloQRpc(my_public, other_public, StartUp_MqbParam.ConsumeParamList.CreateNewItem()));
		reply_tp.StartWriting(PPSCMD_SQ_SESSION, StyloQProtocol::psubtypeReplyOk);
		THROW(reply_tp.FinishWriting(0));
		THROW(p_mqbc = PPMqbClient::CreateInstance(StartUp_MqbParam));
		THROW(p_mqbc->Publish(rpe.ExchangeName, rpe.RoutingKey, &props, reply_tp.constptr(), reply_tp.GetAvailableSize()));
		if(P_Ic->SetRoundTripBlockReplyValues(B, p_mqbc, rpe)) {
			p_mqbc = 0;
		}
		CATCHZOK
		ZDELETE(p_mqbc);
		return ok;
	}
	int    ProcessCommand()
	{
		int    ok = 1;
		return ok;
	}
	virtual void Run()
	{
		//PPMqbClient * p_mqbc = 0;
		bool do_process_loop = false;
		StyloQProtocol tp;
		StyloQProtocol reply_tp;
		PPMqbClient::MessageProperties props;
		//PPMqbClient::RoutingParamEntry rpe;
		//
		// ������ ���� ������ �������������� � ������������� ������. ��� - ���� �������� ������������� � ������ ���������� � ��� ��� ���
		// ���� �� �� ����� ����� ������������.
		// ����� �������� �������: PPSCMD_SQ_ACQUAINTANCE PPSCMD_SQ_SRPAUTH PPSCMD_SQ_SESSION
		// ������� PPSCMD_SQ_SRPREGISTER ����� ����������� ��������� ������� ���������� (���������� �������������� ������).
		//
		switch(TPack.GetH().Type) {
			case PPSCMD_SQ_ACQUAINTANCE:
				if(Acquaintance()) {
					assert(B.Sess.Get(SSecretTagPool::tagSessionSecret, 0));
					do_process_loop = true; // OK
				}
				break;
			case PPSCMD_SQ_SESSION: // ������� ��������� ���������� �� �������� ������, ������� ���� ����������� �� ���������� ������ ������
				if(Session()) {
					assert(B.Sess.Get(SSecretTagPool::tagSessionSecret, 0));
					do_process_loop = true; // OK
				}
				break;
			case PPSCMD_SQ_SRPAUTH: // ������� ��������� ���������� ������� SRP-����������� �� ����������, ������������� ����� 
				if(SrpAuth()) {
					assert(B.Sess.Get(SSecretTagPool::tagSessionSecret, 0));
					do_process_loop = true; // OK
				}
				break;
			default:
				// ������������ ������������ �������
				break;
		}
		if(do_process_loop) {
			//
			// ���� ����� �� ������������ ������� ������ �������, �� ������ ��������� � ����� ������� � ��������.
			// ���� ����� ������ ���� ������ �� ���������� ��������� ������� (���� ����, ��� �� "��������" ����������� �� ��������).
			// ���������� ������� �������� � ���������� ������ � ��������� � ��� ��������.
			//
			assert(P_Ic);
			if(P_Ic) {
				SBinaryChunk sess_secret;
				PPMqbClient::Envelope env;
				if(B.Sess.Get(SSecretTagPool::tagSessionSecret, &sess_secret)) {
					while(do_process_loop) {
						assert(B.P_Mqbc && B.P_MqbRpe);
						if(B.P_Mqbc->ConsumeMessage(env, 150) > 0) {
							B.P_Mqbc->Ack(env.DeliveryTag, 0);
							if(tp.Read(env.Msg, &sess_secret)) {
								if(tp.GetH().Type == PPSCMD_SQ_SRPREGISTER) {
									int32 reply_status = 0;
									SString reply_status_text;
									reply_tp.Z();
									if(P_Ic->Registration_ServiceReply(B, tp)) {
										reply_tp.StartWriting(PPSCMD_SQ_SRPREGISTER, StyloQProtocol::psubtypeReplyOk);
										reply_status_text = "Wellcome!";
									}
									else {
										reply_tp.StartWriting(PPSCMD_SQ_SRPREGISTER, StyloQProtocol::psubtypeReplyError);
										reply_status_text = "Something went wrong";
									}
									{
										reply_tp.P.Put(SSecretTagPool::tagReplyStatus, &reply_status, sizeof(reply_status));
										if(reply_status_text.NotEmpty()) 
											reply_tp.P.Put(SSecretTagPool::tagReplyStatusText, reply_status_text.cptr(), reply_status_text.Len()+1);
									}
									reply_tp.FinishWriting(&sess_secret);
									props.Z();
									int pr = B.P_Mqbc->Publish(B.P_MqbRpe->ExchangeName, B.P_MqbRpe->RoutingKey, &props, reply_tp.constptr(), reply_tp.GetAvailableSize());
								}
								else if(tp.GetH().Type == PPSCMD_SQ_COMMAND) {
									SBinaryChunk cmd_bch;
									SString cmd_buf;
									if(tp.P.Get(SSecretTagPool::tagRawData, &cmd_bch)) {
										SJson * p_js_cmd = 0;
										SJson * p_js_reply = 0;
										cmd_buf.CatN(static_cast<const char *>(cmd_bch.PtrC()), cmd_bch.Len());
										if(json_parse_document(&p_js_cmd, cmd_buf.cptr()) == JSON_OK) {
											p_js_reply = new SJson(SJson::tOBJECT);
											for(SJson * p_cur = p_js_cmd; p_cur; p_cur = p_cur->P_Next) {
												if(p_cur->Type == SJson::tOBJECT) {								
													for(const SJson * p_obj = p_cur->P_Child; p_obj; p_obj = p_obj->P_Next) {
														if(p_obj->Text.IsEqiAscii("cmd")) {
															p_js_reply->InsertString("reply", p_obj->P_Child->Text);
														}
														else {
															if(p_obj->Text.NotEmpty() && p_obj->P_Child && p_obj->P_Child->Text.NotEmpty()) {
																p_js_reply->InsertString(p_obj->Text, p_obj->P_Child->Text);
															}
														}
													}
												}
											}
											p_js_reply->InsertString("result", "OK");
										}
										json_tree_to_string(p_js_reply, cmd_buf);
										ZDELETE(p_js_cmd);
										ZDELETE(p_js_reply);
									}
									else 
										cmd_buf = "Error";
									reply_tp.Z();
									reply_tp.StartWriting(PPSCMD_SQ_COMMAND, StyloQProtocol::psubtypeReplyOk);
									cmd_bch.Put(cmd_buf.cptr(), cmd_buf.Len());
									reply_tp.P.Put(SSecretTagPool::tagRawData, cmd_bch);
									reply_tp.FinishWriting(&sess_secret);
									props.Z();
									int pr = B.P_Mqbc->Publish(B.P_MqbRpe->ExchangeName, B.P_MqbRpe->RoutingKey, &props, reply_tp.constptr(), reply_tp.GetAvailableSize());
								}
							}
						}
						else
							SDelay(200);
					}
				}
			}
		}
		ZDELETE(P_Ic);
	}
private:
	int    Login()
	{
		int    ok = 1;
		char   secret[64];
		SString db_symb;
		if(LB.GetAttr(DbLoginBlock::attrDbSymb, db_symb)) {
			PPVersionInfo vi = DS.GetVersionInfo();
			THROW(vi.GetSecret(secret, sizeof(secret)));
			THROW(DS.Login(db_symb, PPSession::P_JobLogin, secret, PPSession::loginfSkipLicChecking));
			memzero(secret, sizeof(secret));
		}
		CATCHZOK
		return ok;
	}
	void   Logout()
	{
		DS.Logout();
	}
	DbLoginBlock LB;
	PPMqbClient::InitParam StartUp_MqbParam;
	StyloQProtocol TPack;
	PPStyloQInterchange::RoundTripBlock B;
};

class StyloQServer : public PPThread {
	PPMqbClient::InitParam StartUp_MqbParam; 
	DbLoginBlock LB;
public:
	StyloQServer(const DbLoginBlock & rLB, const PPMqbClient::InitParam & rMqbParam) : PPThread(kStyloQServer, 0, 0), StartUp_MqbParam(rMqbParam), LB(rLB)
	{
	}
	virtual void Run()
	{
		const int   do_debug_log = 0; // @debug
		const long  pollperiod_mqc = 500;
		EvPollTiming pt_mqc(pollperiod_mqc, false);
		EvPollTiming pt_purge(3600000, true); // ���� ������� �� ���� ��������� ��� �������. ������ registerImmediate = 1
		const int  use_sj_scan_alg2 = 0;
		SString msg_buf, temp_buf;
		PPMqbClient * p_mqb_cli = PPMqbClient::CreateInstance(StartUp_MqbParam); // @v11.0.9
		if(p_mqb_cli) {
			PPMqbClient::Envelope mqb_envelop;
			const long __cycle_hs = (p_mqb_cli ? 37 : 293); // ������ ������� � ����� ����� ������� (37)
			int    queue_stat_flags_inited = 0;
			StyloQProtocol tpack;
			Evnt   stop_event(SLS.GetStopEventName(temp_buf), Evnt::modeOpen);
			for(int stop = 0; !stop;) {
				uint   h_count = 0;
				HANDLE h_list[32];
				h_list[h_count++] = stop_event;
				//
				STimer __timer;  // ������ ��� ������ ������� �� ���������� ������ ���������� �������
				__timer.Set(getcurdatetime_().addhs(__cycle_hs), 0);
				h_list[h_count++] = __timer;
				uint   r = ::WaitForMultipleObjects(h_count, h_list, 0, /*CycleMs*//*INFINITE*/60000);
				switch(r) {
					case (WAIT_OBJECT_0 + 0): // stop event
						if(do_debug_log) {
							PPLogMessage(PPFILNAM_DEBUG_AEQ_LOG, "StopEvent", LOGMSGF_DBINFO|LOGMSGF_TIME|LOGMSGF_USER);
						}
						stop = 1; // quit loop
						break;
					case WAIT_TIMEOUT:
						// ���� �� �����-�� �������� �������� �������, �� ������������ ���� ��-�����
						// ��������������, ��� ��� ������� ������ ������������!
						if(do_debug_log) {
							PPLogMessage(PPFILNAM_DEBUG_AEQ_LOG, "TimeOut", LOGMSGF_DBINFO|LOGMSGF_TIME|LOGMSGF_USER);
						}
						break;
					case (WAIT_OBJECT_0 + 1): // __timer event
						{
							if(pt_purge.IsTime()) {
								;
							}
							if(p_mqb_cli && pt_mqc.IsTime()) {
								while(p_mqb_cli->ConsumeMessage(mqb_envelop, 200) > 0) {
									p_mqb_cli->Ack(mqb_envelop.DeliveryTag, 0);
									if(tpack.Read(mqb_envelop.Msg, 0)) {
										StyloQServerSession * p_new_sess = new StyloQServerSession(LB, StartUp_MqbParam, tpack);
										if(p_new_sess)
											p_new_sess->Start(0);
									}
								}
							}
						}
						break;
					case WAIT_FAILED:
						if(do_debug_log) {
							PPLogMessage(PPFILNAM_DEBUG_AEQ_LOG, "QueueFailed", LOGMSGF_DBINFO|LOGMSGF_TIME|LOGMSGF_USER);
						}
						// error
						break;
				}
			}
		}
		delete p_mqb_cli;
		DBS.CloseDictionary();
	}
};

/*static*/int PPStyloQInterchange::RunStyloQServer()
{
	int    ok = 1;
	StyloQServer * p_srv = 0;
	PPID   own_peer_id = 0;
	DbProvider * p_dict = CurDict;
	if(p_dict) {
		SString temp_buf;
		PPStyloQInterchange ic;
		PPStyloQInterchange::StoragePacket sp;
		THROW(ic.SetupPeerInstance(&own_peer_id, 1));
		THROW(ic.GetOwnPeerEntry(&sp) > 0);
		{
			SBinaryChunk _ident;
			sp.Pool.Get(SSecretTagPool::tagSvcIdent, &_ident);
			PPMqbClient::InitParam mqb_init_param;
			THROW(PPMqbClient::SetupInitParam(mqb_init_param, "styloq", 0));
			{
				PPMqbClient::RoutingParamEntry rpe;
				if(rpe.SetupStyloQRpcListener(_ident)) {
					PPMqbClient::RoutingParamEntry * p_new_entry = mqb_init_param.ConsumeParamList.CreateNewItem();
					ASSIGN_PTR(p_new_entry, rpe);
				}
			}
			{
				DbLoginBlock dlb;
				PPIniFile ini_file;
				PPDbEntrySet2 dbes;
				int    db_id = 0;
				p_dict->GetDbSymb(temp_buf);
				THROW(ini_file.IsValid());
				THROW(dbes.ReadFromProfile(&ini_file, 0));
				THROW_SL(db_id = dbes.GetBySymb(temp_buf, &dlb));
				{
					p_srv = new StyloQServer(dlb, mqb_init_param);
					if(p_srv) {
						p_srv->Start(0);
					}
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

#if 0 // @model {
static void _EcdhCryptModelling()
{
	//#define NISTP256 NID_X9_62_prime256v1
	//#define NISTP384 NID_secp384r1
	//#define NISTP521 NID_secp521r1
	int    ok = 1;
	BN_CTX * p_bn_ctx = BN_CTX_new();
	EC_KEY * p_key_cli = 0;
	EC_KEY * p_key_svc = 0;
	uchar * p_secret_cli = 0;
	uchar * p_secret_svc = 0;
	const EC_POINT * p_public_cli = 0;
	const EC_POINT * p_public_svc = 0;
	SBinaryChunk public_key_cli;
	SBinaryChunk public_key_bn_cli;
	SBinaryChunk public_key_svc;
	SBinaryChunk public_key_bn_svc;
	size_t secret_len_cli = 0;
	size_t secret_len_svc = 0;
	{
		THROW(p_key_cli = EC_KEY_new_by_curve_name(ec_curve_name_id)); // Failed to create key curve
		THROW(EC_KEY_generate_key(p_key_cli) == 1); // Failed to generate key
	}	
	{
		THROW(p_key_svc = EC_KEY_new_by_curve_name(ec_curve_name_id)); // Failed to create key curve
		THROW(EC_KEY_generate_key(p_key_svc) == 1); // Failed to generate key
	}
	//
	{
		const EC_GROUP * p_ecg = EC_KEY_get0_group(p_key_cli);
		p_public_cli = EC_KEY_get0_public_key(p_key_cli);
		size_t octlen = EC_POINT_point2oct(p_ecg, p_public_cli, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, p_bn_ctx);
		THROW(public_key_cli.Ensure(octlen));
		EC_POINT_point2oct(p_ecg, p_public_cli, POINT_CONVERSION_UNCOMPRESSED, static_cast<uchar *>(public_key_cli.Ptr()), public_key_cli.Len(), p_bn_ctx);
	}
	{
		const EC_GROUP * p_ecg = EC_KEY_get0_group(p_key_svc);
		p_public_svc = EC_KEY_get0_public_key(p_key_svc);
		size_t octlen = EC_POINT_point2oct(p_ecg, p_public_svc, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, p_bn_ctx);
		THROW(public_key_svc.Ensure(octlen));
		EC_POINT_point2oct(p_ecg, p_public_svc, POINT_CONVERSION_UNCOMPRESSED, static_cast<uchar *>(public_key_svc.Ptr()), public_key_svc.Len(), p_bn_ctx);
	}
	//unsigned char * get_secret(EC_KEY *key, const EC_POINT *peer_pub_key, size_t *secret_len)
	{
		EC_GROUP * p_ecg2 = EC_GROUP_new_by_curve_name(ec_curve_name_id);
		EC_POINT * p_public_svc2 = EC_POINT_new(p_ecg2);
		EC_POINT_oct2point(p_ecg2, p_public_svc2, static_cast<uchar *>(public_key_svc.Ptr()), public_key_svc.Len(), p_bn_ctx);
		const int field_size = EC_GROUP_get_degree(EC_KEY_get0_group(p_key_cli));
		secret_len_cli = (field_size + 7) / 8;
		THROW(p_secret_cli = (uchar *)OPENSSL_malloc(secret_len_cli)); // Failed to allocate memory for secret
		secret_len_cli = ECDH_compute_key(p_secret_cli, secret_len_cli, p_public_svc2, p_key_cli, NULL);
		THROW(secret_len_cli > 0);
		EC_POINT_free(p_public_svc2);
		EC_GROUP_free(p_ecg2);
	}
	{
		EC_GROUP * p_ecg2 = EC_GROUP_new_by_curve_name(ec_curve_name_id);
		EC_POINT * p_public_cli2 = EC_POINT_new(p_ecg2);
		EC_POINT_oct2point(p_ecg2, p_public_cli2, static_cast<uchar *>(public_key_cli.Ptr()), public_key_cli.Len(), p_bn_ctx);
		const int field_size = EC_GROUP_get_degree(EC_KEY_get0_group(p_key_svc));
		secret_len_svc = (field_size + 7) / 8;
		THROW(p_secret_svc = (uchar *)OPENSSL_malloc(secret_len_svc)); // Failed to allocate memory for secret
		secret_len_svc = ECDH_compute_key(p_secret_svc, secret_len_svc, p_public_cli2, p_key_svc, NULL);
		THROW(secret_len_svc > 0);
		EC_POINT_free(p_public_cli2);
		EC_GROUP_free(p_ecg2);
	}
	{
		assert(secret_len_cli == secret_len_svc);
		for(size_t i = 0; i < secret_len_cli; i++)
			assert(p_secret_cli[i] == p_secret_svc[i]);
	}
	CATCHZOK
	EC_KEY_free(p_key_cli);
	EC_KEY_free(p_key_svc);
	OPENSSL_free(p_secret_cli);
	OPENSSL_free(p_secret_svc);
	BN_CTX_free(p_bn_ctx);
}

static int __KeyGenerationEc()
{
	int    ok = 1;
	//uint8  prv_key[256];
	//uint8  pub_key[256];
	size_t prv_key_size = 0;
	size_t pub_key_size = 0;
	BIO * outbio = NULL;
	EVP_PKEY * pkey   = NULL;
	_EcdhCryptModelling();
	// 
	// These function calls initialize openssl for correct work.
	// 
	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	// 
	// Create the Input/Output BIO's.
	// 
	outbio = BIO_new(BIO_s_file());
	outbio = BIO_new_fp(stdout, BIO_NOCLOSE);
	// 
	// Create a EC key sructure, setting the group type from NID
	// 
	int    eccgrp = OBJ_txt2nid("secp521r1");
	EC_KEY * p_ecc = EC_KEY_new_by_curve_name(eccgrp);
	// 
	// For cert signing, we use  the OPENSSL_EC_NAMED_CURVE flag*
	// 
	EC_KEY_set_asn1_flag(p_ecc, OPENSSL_EC_NAMED_CURVE);
	// 
	// Create the public/private EC key pair here
	// 
	THROW(EC_KEY_generate_key(p_ecc)); // Error generating the ECC key
	// 
	// Converting the EC key into a PKEY structure let us
	// handle the key just like any other key pair.
	// 
	pkey = EVP_PKEY_new();
	THROW(EVP_PKEY_assign_EC_KEY(pkey, p_ecc)); // Error assigning ECC key to EVP_PKEY structure
	//int grprkr = EVP_PKEY_get_raw_private_key(/*pkey*/p_ecc, prv_key, &prv_key_size);
	//int grpukr = EVP_PKEY_get_raw_public_key(pkey, pub_key, &pub_key_size);
	{
		// 
		// Now we show how to extract EC-specifics from the key
		// 
		p_ecc = EVP_PKEY_get1_EC_KEY(pkey);
		const EC_GROUP * ecgrp = EC_KEY_get0_group(p_ecc);
		// 
		// Here we print the key length, and extract the curve type.
		// 
		BIO_printf(outbio, "ECC Key size: %d bit\n", EVP_PKEY_bits(pkey));
		BIO_printf(outbio, "ECC Key type: %s\n", OBJ_nid2sn(EC_GROUP_get_curve_name(ecgrp)));
	}
	//
	// Here we print the private/public key data in PEM format
	// 
	THROW(PEM_write_bio_PrivateKey(outbio, pkey, NULL, NULL, 0, 0, NULL)); // Error writing private key data in PEM format
	THROW(PEM_write_bio_PUBKEY(outbio, pkey)); // Error writing public key data in PEM format
	CATCHZOK
	EVP_PKEY_free(pkey);
	EC_KEY_free(p_ecc);
	BIO_free_all(outbio);
	return ok;
}
#endif // } @model