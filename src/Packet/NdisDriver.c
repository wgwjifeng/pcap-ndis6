//////////////////////////////////////////////////////////////////////
// Project: pcap-ndis6
// Description: WinPCAP fork with NDIS6.x support 
// License: MIT License, read LICENSE file in project root for details
//
// Copyright (c) 2017 ChangeDynamix, LLC
// All Rights Reserved.
// 
// https://changedynamix.io/
// 
// Author: Mikhail Burilov
// 
// Based on original WinPcap source code - https://www.winpcap.org/
// Copyright(c) 1999 - 2005 NetGroup, Politecnico di Torino(Italy)
// Copyright(c) 2005 - 2007 CACE Technologies, Davis(California)
// All rights reserved.
//////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <windows.h>

#include "..\shared\win_bpf.h"

#include "Packet32.h"
#include "NdisDriver.h"
#include "..\shared\CommonDefs.h"
#include <stdio.h>

#include <string>

#include "..\shared\StrUtils.h"

#ifdef DEBUG_CONSOLE
#define DEBUG_PRINT(x,...) printf(x, __VA_ARGS__)
#else
#define DEBUG_PRINT(x,...)
#endif

BOOL NdisDriver_ControlDevice(
    __in        HANDLE  DeviceHandle,
    __in        DWORD   ControlCode,
    __in_opt    LPVOID  InBuffer,
    __in_opt    DWORD   InBufferSize,
    __out_opt   LPVOID  OutBuffer,
    __out_opt   DWORD   OutBufferSize,
    __out_opt   LPDWORD BytesReturned = NULL,
    __out_opt   LPDWORD ErrorCode = NULL);

LPPCAP_NDIS NdisDriverOpen()
{
    LPPCAP_NDIS     Result = nullptr;
    HANDLE          FileHandle = INVALID_HANDLE_VALUE;
    std::wstring    DeviceName = UTILS::STR::FormatW(
        L"\\\\.\\%s%s",
        ADAPTER_ID_PREFIX_W,
        ADAPTER_NAME_FORLIST_W);

    FileHandle = CreateFileW(
        DeviceName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        NULL);
    RETURN_VALUE_IF_FALSE(
        FileHandle != INVALID_HANDLE_VALUE,
        nullptr);
    try
    {
        Result = reinterpret_cast<LPPCAP_NDIS>(sizeof(PCAP_NDIS));
        if (Assigned(Result))
        {
            Result->Handle = FileHandle;
        }
    }
    catch (...)
    {
    }
    if (!Assigned(Result))
    {
        CloseHandle(FileHandle);
    }

    return Result;
};

void NdisDriverClose(
    __in    LPPCAP_NDIS Ndis)
{
    RETURN_IF_FALSE(Assigned(Ndis));

    if (Ndis->Handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(Ndis->Handle);
    }

    free(Ndis);
};

PPCAP_NDIS_ADAPTER NdisDriverOpenAdapter(
    __in            PCAP_NDIS   *ndis,
    __in    const   char        *szAdapterId)
{
    PPCAP_NDIS_ADAPTER  Adapter = nullptr;
    HANDLE              FileHandle = INVALID_HANDLE_VALUE;

    RETURN_VALUE_IF_FALSE(
        Assigned(ndis),
        nullptr);

    std::wstring    DeviceName = UTILS::STR::FormatW(
        L"\\\\.\\%s%S",
        ADAPTER_ID_PREFIX_W,
        szAdapterId);

    FileHandle = CreateFileW(
        DeviceName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        NULL);
    RETURN_VALUE_IF_FALSE(
        FileHandle != INVALID_HANDLE_VALUE,
        nullptr);
    try
    {
        Adapter = (PPCAP_NDIS_ADAPTER)malloc(sizeof(PCAP_NDIS_ADAPTER));
        if (Assigned(Adapter))
        {
            RtlZeroMemory(Adapter, sizeof(PCAP_NDIS_ADAPTER));

            Adapter->Handle = FileHandle;
        }
    }
    catch(...)
    {

    }
    if (!Assigned(Adapter))
    {
        CloseHandle(FileHandle);
    }

    return Adapter;
};

std::wstring NdisDriverGetAdapterEventName(
    __in            PCAP_NDIS           *Ndis,
    __in            PCAP_NDIS_ADAPTER   *Adapter)
{
    char    NameBuffer[1024];
    DWORD   BytesReturned = 0;

    RETURN_VALUE_IF_FALSE(
        (Assigned(Ndis)) &&
        (Assigned(Adapter)),
        L"");

    RtlZeroMemory(NameBuffer, sizeof(NameBuffer));

    RETURN_VALUE_IF_FALSE(
        NdisDriver_ControlDevice(
            Adapter->Handle,
            static_cast<DWORD>(IOCTL_GET_EVENT_NAME),
            nullptr,
            0,
            reinterpret_cast<LPVOID>(NameBuffer),
            static_cast<DWORD>(sizeof(NameBuffer) - 1),
            &BytesReturned),
        L"");

    return UTILS::STR::FormatW(
        L"Global\\%S",
        NameBuffer);
};

void NdisDriverCloseAdapter(
    __in    LPPCAP_NDIS_ADAPTER Adapter)
{
    RETURN_IF_FALSE(Assigned(Adapter));

    if (Adapter->Handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(Adapter->Handle);
    }

    free(Adapter);
};

BOOL NdisDriver_ControlDevice(
    __in        HANDLE  DeviceHandle,
    __in        DWORD   ControlCode,
    __in_opt    LPVOID  InBuffer,
    __in_opt    DWORD   InBufferSize,
    __out_opt   LPVOID  OutBuffer,
    __out_opt   DWORD   OutBufferSize,
    __out_opt   LPDWORD BytesReturned,
    __out_opt   LPDWORD ErrorCode)
{
    BOOL Result = FALSE;
    RETURN_VALUE_IF_FALSE(
        (DeviceHandle != NULL) &&
        (DeviceHandle != INVALID_HANDLE_VALUE),
        FALSE);

    DWORD BytesCnt = 0;
    if (BytesReturned == NULL)
    {
        BytesReturned = &BytesCnt;
    }

    Result = DeviceIoControl(
        DeviceHandle,
        ControlCode,
        InBuffer,
        InBufferSize,
        OutBuffer,
        OutBufferSize,
        BytesReturned,
        NULL);
    if (Assigned(ErrorCode))
    {
        *ErrorCode = GetLastError();
    }
    return Result;
};

BOOL NdisDriverNextPacket(
    __in        LPPCAP_NDIS_ADAPTER Adapter,
    __out       LPVOID              *Buffer,
    __in        size_t              Size,
    __out       PDWORD              BytesReceived,
    __out_opt   PULONGLONG          ProcessId)
{
    RETURN_VALUE_IF_FALSE(
        (Assigned(Adapter)) &&
        (Assigned(BytesReceived)),
        FALSE);
    RETURN_VALUE_IF_FALSE(
        Adapter->Handle != INVALID_HANDLE_VALUE,
        FALSE);

    *BytesReceived = 0;

    RETURN_VALUE_IF_FALSE(
        Size >= sizeof(bpf_hdr),
        FALSE);

    if (Adapter->BufferedPackets == 0)
    {
        DWORD   BytesRead = 0;
        DWORD   ErrorCode = 0;

        RETURN_VALUE_IF_FALSE(
            NdisDriver_ControlDevice(
                Adapter->Handle,
                static_cast<DWORD>(IOCTL_READ_PACKETS),
                nullptr,
                0,
                Adapter->ReadBuffer,
                READ_BUFFER_SIZE,
                &BytesRead,
                &ErrorCode),
            FALSE);

        Adapter->BufferOffset = 0;
        
        DWORD   CurrentSize = 0;

        for (PUCHAR CurrentPtr = Adapter->ReadBuffer;
             (CurrentSize < BytesRead) && (CurrentSize < READ_BUFFER_SIZE);
             CurrentPtr = Adapter->ReadBuffer + CurrentSize)
        {
            pbpf_hdr2   bpf = reinterpret_cast<pbpf_hdr2>(CurrentPtr);
            CurrentSize += bpf->bh_datalen + bpf->bh_hdrlen;

            Adapter->BufferedPackets++;
        }
    }

    if (Adapter->BufferedPackets == 0)
    {
        *BytesReceived = 0;
    }

    if (Adapter->BufferedPackets > 0)
    {
        pbpf_hdr2   bpf = reinterpret_cast<pbpf_hdr2>(Adapter->ReadBuffer + Adapter->BufferOffset);
        ULONG       RequiredSize = sizeof(bpf_hdr) + bpf->bh_datalen;
        bpf_hdr     Header;
        PUCHAR      CurrentPtr;

        RETURN_VALUE_IF_FALSE(
            Size >= RequiredSize,
            FALSE);

        RtlZeroMemory(
            reinterpret_cast<LPVOID>(&Header),
            sizeof(Header));

        Header.bh_caplen = bpf->bh_caplen;
        Header.bh_datalen = bpf->bh_datalen;
        Header.bh_hdrlen = static_cast<u_short>(sizeof(Header));
        Header.bh_tstamp = bpf->bh_tstamp;

        CurrentPtr = reinterpret_cast<PUCHAR>(*Buffer);

        RtlCopyMemory(
            reinterpret_cast<LPVOID>(CurrentPtr),
            reinterpret_cast<LPVOID>(&Header),
            sizeof(Header));

        CurrentPtr += sizeof(Header);

        RtlCopyMemory(
            reinterpret_cast<LPVOID>(CurrentPtr),
            reinterpret_cast<LPVOID>(Adapter->ReadBuffer + Adapter->BufferOffset + bpf->bh_hdrlen),
            bpf->bh_datalen);

        *BytesReceived = RequiredSize;

        if (Assigned(ProcessId))
        {
            *ProcessId = bpf->ProcessId;
        }

        Adapter->BufferedPackets--;
        Adapter->BufferOffset += bpf->bh_hdrlen + bpf->bh_datalen;
    }

    return TRUE;
};

// Get adapter list
LPPCAP_NDIS_ADAPTER_LIST NdisDriverGetAdapterList(PCAP_NDIS* ndis)
{
    ULONG                       AdaptersCount = 0;
    LPPCAP_NDIS_ADAPTER_LIST    List = nullptr;
    SIZE_T                      SizeRequired = 0;
    BOOL                        Failed = FALSE;

    RETURN_VALUE_IF_FALSE(
        Assigned(ndis),
        nullptr);

    RETURN_VALUE_IF_FALSE(
        NdisDriver_ControlDevice(
            ndis->Handle,
            static_cast<DWORD>(IOCTL_GET_ADAPTERS_COUNT),
            nullptr,
            0UL,
            reinterpret_cast<LPVOID>(&AdaptersCount),
            static_cast<DWORD>(sizeof(AdaptersCount))),
        nullptr);

    SizeRequired =
        sizeof(PCAP_NDIS_ADAPTER_LIST) +
        (AdaptersCount - 1) * sizeof(PCAP_NDIS_ADAPTER_INFO);

    List = reinterpret_cast<LPPCAP_NDIS_ADAPTER_LIST>(malloc(SizeRequired));
    RETURN_VALUE_IF_FALSE(
        Assigned(List),
        nullptr);
    __try
    {
        Failed = !NdisDriver_ControlDevice(
            ndis->Handle,
            static_cast<DWORD>(IOCTL_GET_ADAPTERS),
            nullptr,
            0,
            reinterpret_cast<LPVOID>(List),
            SizeRequired);
    }
    __finally
    {
        if (Failed)
        {
            free(reinterpret_cast<void *>(List));
            List = nullptr;
        }
    }
    
    return List;
};

void NdisDriverFreeAdapterList(
    __in    LPPCAP_NDIS_ADAPTER_LIST    List)
{
    RETURN_IF_FALSE(Assigned(List));

    free(List);
};