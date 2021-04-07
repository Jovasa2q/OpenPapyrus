// STYLOQ.CPP
// Copyright (c) A.Sobolev 2021
//
#include <pp.h>
#pragma hdrstop
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
*/
//
// @construction
//
class PPStyloQInterchange {
public:
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
	// Descr: ���������� ����������� ��������
	//
	int    MakeInvitation(const Invitation & rSource, SString & rInvitationData);
	//
	// Descr: ������ ����������� ������� ��������
	//
	int    AcceptInvitation(const char * pInvitationData, Invitation & rInv);
	int    SendPacket(void * pState, TransportPacket & rPack);
	int    AcceptPacket(void * pState, TransportPacket & rPack);
private:
	StyloQSecTbl T;
};

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

int Test_PPStyloQInterchange_Invitation()
{
	int    ok = 1;
	SString temp_buf;
	PPStyloQInterchange ic;
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

