// STYLOQ.CPP
// Copyright (c) A.Sobolev 2021
//
#include <pp.h>
#pragma hdrstop
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
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
//
class PPStyloQInterchange {
public:
	// 1 - native service, 2 - foreign service, 3 - client, 4 - client-session, 5 - service-session
	enum {
		kUndef = 0,
		kNativeService = 1, // ����������� �������������. ������������ ��� ������ ����, ������� ����������, ������� ������� �� ����� ��������� //
		kForeignService,
		kClient,
		kClientSession,
		kServiceSession
	};
	struct Service {
		PPID   ID;
		SString Appellation;
		S_GUID PublicIdent;
	};
	struct Account {
		PPID   ID;
		PPID   PersonID;
		long   Flags;
		S_GUID PublicIdent;
	};
	struct Command {
		
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
	struct TransportPacket {
		SSecretTagPool P;
	};
	class Client {
	public:
		Client()
		{
		}
		~Client()
		{
		}
	};
	class Server {
	public:
		Server()
		{
		}
		~Server()
		{
		}
	};
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
	//
	// Descr: ������ ����������� ������� ��������
	//
	int    AcceptInvitation(const char * pInvitationData, Invitation & rInv);
	int    SendPacket(void * pState, TransportPacket & rPack);
	int    AcceptPacket(void * pState, TransportPacket & rPack);

	int    Registration_ClientInit(void * pState, const void * pSvcIdent, size_t svcIdentLen, TransportPacket & rPack);
	int    Registration_ServiceReply(void * pState, TransportPacket & rPack);
	int    Registration_ClientSendVerification(void * pState, const void * pSvcIdent, size_t svcIdentLen, TransportPacket & rPack);
	int    Registration_ServiceAckVerification(void * pState, TransportPacket & rPack);

	int    Auth_ClientInit(void * pState, TransportPacket & rPack); // User -> Host: (username, bytes_A) 
	int    Auth_ServiceChallenge(void * pState, TransportPacket & rPack); // Host -> User: (bytes_s, bytes_B) 
	int    Auth_ClientVerification(void * pState, TransportPacket & rPack); // User -> Host: (bytes_M) 
	int    Auth_ServiceAck(void * pState, TransportPacket & rPack); // Host -> User: (HAMK) 

	int    Session_ClientInit(void * pState, TransportPacket & rPack);
	int    Session_ServiceReply(void * pState, TransportPacket & rPack);
private:
	struct StoragePacket {
		StyloQSecTbl::Rec Rec;
		SSecretTagPool Pool;
	};
	int    ReadCurrentPacket(StoragePacket * pPack);
	int    PutPeerEntry(PPID * pID, StoragePacket * pPack, int use_ta);
	int    GetPeerEntry(PPID id, StoragePacket * pPack);
	int    GetOwnPeerEntry(StoragePacket * pPack);
	int    SearchGlobalIdentEntry(const void * pIdent, size_t identLen, StoragePacket * pPack);
	enum {
		gcisfMakeSecret = 0x0001
	};
	int    GetClientIdentForService(const SSecretTagPool & rOwnPool, const void * pSvcId, size_t svcIdLen, long flags, SSecretTagPool & rPool);
	StyloQSecTbl T;
};

int PPStyloQInterchange::ReadCurrentPacket(StoragePacket * pPack)
{
	int    ok = -1;
	if(pPack) {
		SBuffer sbuf;
		SSerializeContext sctx;
		T.copyBufTo(&pPack->Rec);
		T.readLobData(T.VT, sbuf);
		T.destroyLobData(T.VT);
		THROW(pPack->Pool.Serialize(-1, sbuf, &sctx));
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::SearchGlobalIdentEntry(const void * pIdent, size_t identLen, StoragePacket * pPack)
{
	assert(identLen > 0 && identLen <= sizeof(StyloQSecTbl::Key1));
	int    ok = -1;
	StyloQSecTbl::Key1 k1;
	THROW(identLen > 0 && identLen <= sizeof(StyloQSecTbl::Key1));
	MEMSZERO(k1);
	memcpy(k1.BI, pIdent, identLen);
	if(T.search(1, &k1, spEq)) {
		THROW(ReadCurrentPacket(pPack));
		ok = 1;			
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
		assert(oneof5(pPack->Rec.Kind, kNativeService, kForeignService, kClient, kClientSession, kServiceSession));
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
				T.copyBufFrom(&pPack->Rec);
				THROW_SL(pPack->Pool.Serialize(+1, cbuf, &sctx));
				THROW(T.writeLobData(T.VT, cbuf.GetBuf(0), cbuf.GetAvailableSize()));
				THROW_DB(T.rereadForUpdate(0, 0));
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
	StoragePacket ex_pack;
	if(GetOwnPeerEntry(&ex_pack) > 0) {
		; // ������ ��� ����������
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
		StoragePacket pack;
		p_bn_ctx = BN_CTX_new();
		p_rn = BN_new();
		{
			const size_t seed_size = 128;
			SLS.GetTLA().Rg.ObfuscateBuffer(temp, seed_size);
			RAND_seed(temp, seed_size);
		}
		int rn_len;
		do {
			BN_priv_rand(p_rn, primary_rn_bits_width, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
			rn_len = BN_bn2bin(p_rn, temp);
			THROW(rn_len > 0);
		} while(rn_len < primary_rn_bits_width / 8);
		assert(rn_len == primary_rn_bits_width / 8);
		pack.Pool.Put(SSecretTagPool::tagPrimaryRN, temp, rn_len);
		//
		do {
			BN_priv_rand(p_rn, 160, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
			rn_len = BN_bn2bin(p_rn, temp);
			THROW(rn_len > 0);
		} while(rn_len < 20);
		assert(rn_len == 20);
		pack.Pool.Put(SSecretTagPool::tagFPI, temp, rn_len);
		//
		{
			S_GUID ag;
			ag.Generate();
			pack.Pool.Put(SSecretTagPool::tagAG, &ag, sizeof(ag));
		}
		{
			SBinaryChunk public_ident;
			THROW(GetClientIdentForService(pack.Pool, temp, rn_len, 0, pack.Pool));
			int  r = pack.Pool.Get(SSecretTagPool::tagPublicIdent, &public_ident);
			assert(r);
			assert(public_ident.Len() <= sizeof(pack.Rec.BI));
			memcpy(pack.Rec.BI, public_ident.PtrC(), public_ident.Len());
			pack.Rec.Kind = kNativeService;
		}
		THROW(PutPeerEntry(&id, &pack, 1));
		{
			SBinaryChunk c1;
			SBinaryChunk c2;
			const uint32 test_tag_list[] = { SSecretTagPool::tagPrimaryRN, SSecretTagPool::tagPublicIdent, SSecretTagPool::tagAG, SSecretTagPool::tagFPI };
			{
				StoragePacket test_pack;
				if(GetPeerEntry(id, &test_pack) > 0) {
					for(uint i = 0; i < SIZEOFARRAY(test_tag_list); i++) {
						uint32 _tag = test_tag_list[i];
						if(pack.Pool.Get(_tag, &c1) && test_pack.Pool.Get(_tag, &c2)) {
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
				StoragePacket test_pack;		
				if(GetOwnPeerEntry(&test_pack) > 0) {
					for(uint i = 0; i < SIZEOFARRAY(test_tag_list); i++) {
						uint32 _tag = test_tag_list[i];
						if(pack.Pool.Get(_tag, &c1) && test_pack.Pool.Get(_tag, &c2)) {
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
		}
		ok = 1;
	}
	CATCHZOK
	BN_free(p_rn);
	BN_CTX_free(p_bn_ctx);
	return ok;
}

int PPStyloQInterchange::GetClientIdentForService(const SSecretTagPool & rOwnPool, const void * pSvcId, size_t svcIdLen, long flags, SSecretTagPool & rPool)
{
	int    ok = 1;
	SBinaryChunk prmrn;
	SBinaryChunk ag;
	THROW(rOwnPool.Get(SSecretTagPool::tagPrimaryRN, &prmrn));
	THROW(rOwnPool.Get(SSecretTagPool::tagAG, &ag));
	prmrn.Cat(pSvcId, svcIdLen);
	prmrn.Cat(ag.PtrC(), ag.Len());
	{
		binary160 hash = SlHash::Sha1(0, prmrn.PtrC(), prmrn.Len());
		assert(sizeof(hash) == 20);
		rPool.Put(SSecretTagPool::tagPublicIdent, &hash, sizeof(hash));
		if(flags & gcisfMakeSecret) {
			SBinaryChunk secret;
			secret.Cat(ag.PtrC(), ag.Len());
			secret.Cat(pSvcId, svcIdLen);
			binary160 secret_hash = SlHash::Sha1(0, secret.PtrC(), secret.Len());
			assert(sizeof(secret_hash) == 20);
			rPool.Put(SSecretTagPool::tagSecret, &secret_hash, sizeof(secret_hash));
		}
	}
	CATCHZOK
	return ok;
}
//
// Registration {
//
struct PPStyloQInterchange_RegistrationContext {
};

struct PPStyloQInterchangeState {
	PPStyloQInterchangeState() : Phase(phaseUndef), State(0), P_Usr(0), P_Vrf(0)
	{
	}
	~PPStyloQInterchangeState()
	{
		delete P_Usr;
		delete P_Vrf;
	}
	enum {
		phaseUndef = 0,         // �������������� ���� 
		phaseClientSrpInit,     // ������ ������ ������� (ident, A) 
		phaseClientSrpM,        // ������ ������ ������� (M) or ERROR
		phaseClientSrpAck,      // ������ ������ ������� ACK or ERROR
		phaseSvcSrpChallenge,   // ������ ������ ������� (salt, B) or ERROR
		phaseSvcSrpHamc,        // ������ ������ ������� (HAMC) or ERROR
		// (phaseClientSrpInit) > (phaseSvcSrpChallenge) > (phaseClientSrpM) > (phaseSvcSrpHamc) > (phaseClientSrpAck)
	};
	enum {
		stError = 0x0001 // �� ����������� �������� ��������� �� ������
	};
	int    Phase;
	uint   State;
	SlSRP::User * P_Usr;     // SRP ���������� ����� 
	SlSRP::Verifier * P_Vrf; // SRP ��������� �����
};

struct InterchangeRoundTrip {
	
};

int PPStyloQInterchange::Registration_ClientInit(void * pState, const void * pSvcIdent, size_t svcIdentLen, TransportPacket & rPack)
{
	int    ok = 0;
	PPStyloQInterchange_RegistrationContext * p_ctx = 0;
	SSecretTagPool tp;
	SString temp_buf;
	SBinaryChunk cli_ident;
	SBinaryChunk cli_secret;
	StoragePacket own_peer;
	THROW(GetOwnPeerEntry(&own_peer) > 0);
	if(GetClientIdentForService(own_peer.Pool, pSvcIdent, svcIdentLen, gcisfMakeSecret, tp)) {
		SBinaryChunk __s;
		SBinaryChunk __v;
		THROW(tp.Get(SSecretTagPool::tagPublicIdent, &cli_ident));
		THROW(tp.Get(SSecretTagPool::tagSecret, &cli_secret));
		temp_buf.EncodeMime64(cli_ident.PtrC(), cli_ident.Len());
		SlSRP::CreateSaltedVerificationKey2(SlSRP::SRP_SHA1, SlSRP::SRP_NG_8192, temp_buf, PTR8C(cli_secret.PtrC()), cli_secret.Len(), __s, __v, 0, 0);
		rPack.P.Put(SSecretTagPool::tagPublicIdent, cli_ident);
		rPack.P.Put(SSecretTagPool::tagSrpVerifierSalt, __s);
		rPack.P.Put(SSecretTagPool::tagSrpVerifier, __v);
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::Registration_ServiceReply(void * pState, TransportPacket & rPack)
{
	int    ok = 0;
	return ok;
}

int PPStyloQInterchange::Registration_ClientSendVerification(void * pState, const void * pSvcIdent, size_t svcIdentLen, TransportPacket & rPack)
{
	int    ok = 0;
	SString temp_buf;
	StoragePacket svc_pack;
	if(SearchGlobalIdentEntry(pSvcIdent, svcIdentLen, &svc_pack) > 0) {
		SBinaryChunk cli_ident;
		SBinaryChunk cli_secret;		
		THROW(svc_pack.Pool.Get(SSecretTagPool::tagPublicIdent, &cli_ident));
		THROW(svc_pack.Pool.Get(SSecretTagPool::tagSecret, &cli_secret));
		{
			SBinaryChunk __a; // A
			char * p_auth_ident = 0;
			temp_buf.EncodeMime64(cli_ident.PtrC(), cli_ident.Len());
			SlSRP::User usr(SlSRP::SRP_SHA1, SlSRP::SRP_NG_8192, temp_buf, cli_secret.PtrC(), cli_secret.Len(), /*n_hex*/0, /*g_hex*/0);
			usr.StartAuthentication(&p_auth_ident, __a);
			// User -> Host: (ident, __a) 
		}
	}
	CATCHZOK
	return ok;
}

int PPStyloQInterchange::Registration_ServiceAckVerification(void * pState, TransportPacket & rPack)
{
	int    ok = 0;
	return ok;
}
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

int PPStyloQInterchange::AcceptInvitation(const char * pInvitationData, Invitation & rInv)
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
		if(tokn == 1) { // prefix and svcid
			const char first_symb = temp_buf.C(0);
			size_t actual_size = 0;
			THROW(first_symb == 'A'); // @error invalid invitation prefix
			temp_buf.ShiftLeft();
			temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
			THROW(actual_size == sizeof(rInv.SvcPublicId)); // @error invalid service public id
			memcpy(&rInv.SvcPublicId, temp_binary, actual_size);
		}
		else if(tokn == 2) { // capabilities
			size_t actual_size = 0;
			temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
			THROW(actual_size == sizeof(rInv.Capabitities)); // @error invalid capabilities
			memcpy(&rInv.Capabitities, temp_binary, actual_size);
		}
		else if(tokn == 3) { // access point
			size_t actual_size = 0;
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
			size_t actual_size = 0;
			temp_buf.DecodeMime64(temp_binary, temp_binary.GetSize(), &actual_size);
			temp_buf.Z().CatN(temp_binary, actual_size);
			rInv.Command.CatN(temp_binary, actual_size);
		}
		else if(tokn == 5) { // command params
			size_t actual_size = 0;
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

//
static int __KeyGenerationEc()
{
	int    ok = 1;
	uint8  prv_key[256];
	uint8  pub_key[256];
	size_t prv_key_size = 0;
	size_t pub_key_size = 0;
	BIO * outbio = NULL;
	EVP_PKEY * pkey   = NULL;
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
//

int Test_PPStyloQInterchange_Invitation()
{
	int    ok = 1;
	SString temp_buf;
	PPStyloQInterchange ic;
	//
	PPID   own_peer_id = 0;
	int    spir = ic.SetupPeerInstance(&own_peer_id, 1);
	__KeyGenerationEc();
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
	return ok;
}

