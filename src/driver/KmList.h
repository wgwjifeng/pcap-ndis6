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
// Author: Andrey Fedorinin
//////////////////////////////////////////////////////////////////////

#ifndef KM_LIST_H
#define KM_LIST_H

#include "KmLock.h"

typedef struct _KM_LIST
{
    KM_LOCK         Lock;
    LIST_ENTRY      Head;
    ULARGE_INTEGER  Count;
} KM_LIST, *PKM_LIST;

typedef void(__stdcall _KM_LIST_ITEM_CALLBACK)(
    __in    PKM_LIST    List,
    __in    PLIST_ENTRY Item);

/*
    KM_LIST_ITEM_CALLBACK callback routine.
    
    Purpose:
        The routine is used whenever a Km_List_* routine needs
        to pass a list item to the client code

    Parameters:
        List    - Pointer to KM_LIST structure.
        Item    - Pointer to LIST_ENTRY structure.
                  Use CONTAINING_RECORD macro to cast this pointer to the one
                  used in the client code.

    Return values:
        None.
*/
typedef _KM_LIST_ITEM_CALLBACK  KM_LIST_ITEM_CALLBACK, *PKM_LIST_ITEM_CALLBACK;

/*
    KM_LIST_ITEM_COMPARISON_CALLBACK routine.

    Purpose:
        The routine is used for comparison of items
        stored in a list to an item definition passed
        to the search routine (Km_List_FindItem/Ex).
        
    Parameters:
        List            - Pointer to KM_LIST structure representing the list.
        ItemDefinition  - Untyped pointer representing item definition 
                          the callback routine should compare the Item to.
                          This value is optional, client-specific and can NULL.
        Item            - Pointer to LIST_ENTRY structure representing the
                          item that should be compared to the item definition
                          specified in ItemDefinition parameter.

    Return values:
        0               - The Item is equal to the ItemDefinition.
        Other values    - The Item is not equal to the ItemDefinition.
*/
typedef int(__stdcall _KM_LIST_ITEM_COMPARISON_CALLBACK)(
    __in    PKM_LIST    List,
    __in    PVOID       ItemDefinition,
    __in    PLIST_ENTRY Item);
typedef _KM_LIST_ITEM_COMPARISON_CALLBACK KM_LIST_ITEM_COMPARISON_CALLBACK, *PKM_LIST_ITEM_COMPARISON_CALLBACK;

/// <summary>
/// Initializes a KM_LIST_ structure.
/// </summary>
/// <remarks>
/// <para>The routine returns STATUS_SUCCESS upon success</para>
/// <para>or an appropriate other NTSTATUS values otherwise</para>
/// </remarks>
/// <param name="List">Pointer to KM_LIST structure representing the list to initialize</param>
/// <returns>
/// Returns STATUS_SUCCESS upon success or other NTSTATUS values otherwise.
/// </returns>
NTSTATUS __stdcall Km_List_Initialize(
    __in    PKM_LIST    List);

/*
    Km_List_AddItemEx routine.

    Purpose:
        Adds a LIST_ENTRY structure to the list.

    Parameters:
        List        - Pointer to KM_LIST structure representing the list.
        Item        - Pointer to LIST_ENTRY structure representing the item to add.
        CheckParams - A boolean value that specifies whether the routine
                      should check the parameters it's being supplied by the caller.
        LockList    - A boolean value that specifies whether the routine
                      should perform the operation in a thread-safe way.
        
    Return values:
        STATUS_SUCCESS              - The item was added successfully.
        STATUS_INVALID_PARAMETER_1  - The List parameter is NULL.
        STATUS_INVALID_PARAMETER_2  - The Item parameter is NULL.
*/
/// <summary>
/// Km_List_AddItemEx routine.
/// </summary>
/// <param name='List'>Pointer to KM_LIST structure representing the list</param>
/// <param name='Item'>Pointer to LIST_ENTRY structure representing the item to add</param>
/// <param name='CheckParams'>A boolean value that specifies whether the routine should check the parameters it's being supplied by the caller</param>
/// <param name='LockList'>A boolean value that specifies whether the routine should perform the operation in a thread-safe way</param>
/// <returns>
///     <para>STATUS_SUCCESS upon success</para>
///     <para>STATUS_INVALID_PARAMETER_1 if the List paramter is NULL</para>
///     <para>STATUS_INVALID_PARAMETER_2 if the Item parameter is NULL</para>
/// </returns>
NTSTATUS __stdcall Km_List_AddItemEx(
    __in    PKM_LIST    List,
    __in    PLIST_ENTRY Item,
    __in    BOOLEAN     CheckParams,
    __in    BOOLEAN     LockList);

/*
    Km_List_AddListEx routine.

    Purpose:
        Adds entries from source KM_LIST to destination one.
        The source list is cleared after the routine succeedes.

    Parameters:
        Destinationlist     - Pointer to KM_LIST structure representing the source list.
        SourceList          - Pointer to KM_LIST structure representing the destination list.
        CheckParams         - A boolean value that specifies whether the routine
                              should check the parameters it's being supplied by the caller.
        LockSourceList      - A boolean value that specifies whether the routine should
                              lock the source list.
        LockDestinationList - A boolean value that specifies whether the routine should
                              lock the destination list.

    Return values:
        STATUS_SUCCESS              - The routine succeeded.
        STATUS_INVALID_PARAMETER_1  - The DestinationList parameter is NULL.
        STATUS_INVALID_PARAMETER_2  - The SourceList parameter is NULL.
        STATUS_NO_MORE_ENTRIES      - The SourceList is empty.
        STATUS_UNSUCCESSFUL         - Routine failed.
*/
/// <summary>
/// Moves the items from a doubly-linked list into a KM_LIST.
/// </summary>
/// 
NTSTATUS __stdcall Km_List_AddListEx(
    __in    PKM_LIST    DestinationList,
    __in    PKM_LIST    SourceList,
    __in    BOOLEAN     CheckParams,
    __in    BOOLEAN     LockSourceList,
    __in    BOOLEAN     LockDestinationList);

/*
    Km_List_AddListEx routine.

    Purpose:
        Adds entries from source doubly-linked list destination KM_LIST.
        The source list is cleared after the routine succeedes.

    Parameters:
        Destinationlist     - Pointer to KM_LIST structure representing the source list.
        SourceList          - Pointer to the source doubly-linked list head.
        CheckParams         - A boolean value that specifies whether the routine
                              should check the parameters it's being supplied by the caller.
        LockDestinationList - A boolean value that specifies whether the routine should
                              lock the destination list.

    Return values:
        STATUS_SUCCESS              - The routine succeeded.
        STATUS_INVALID_PARAMETER_1  - The DestinationList parameter is NULL.
        STATUS_INVALID_PARAMETER_2  - The SourceList parameter is NULL.
        STATUS_NO_MORE_ENTRIES      - The SourceList is empty.
        STATUS_UNSUCCESSFUL         - Routine failed.
*/
NTSTATUS __stdcall Km_List_AddLinkedListEx(
    __in    PKM_LIST    DestinationList,
    __in    PLIST_ENTRY SourceList,
    __in    BOOLEAN     CheckParams,
    __in    BOOLEAN     LockDestinationList);

/*
    Km_List_RemoveItemEx routine.

    Purpose:
        Removes an item from the list.

    Parameters:
        List        - Pointer to KM_LIST structure representing the list.
        Item        - Pointer to LIST_ENTRY structure representing the item to remove.
                      Please note, that the routine does not perform a check on whether 
                      the Item is in the list.
                      Make sure to use the Km_List_FindItemEx routine first if you need
                      to make sure that a particular item is in the list prior to trying
                      to remove it.
        CheckParams - A boolean value that specifies whether the routine
                      should check the parameters it's being supplied by the caller.
        LockList    - A boolean value that specifies whether the routine
                      should perform the operation in a thread-safe way.

    Return values:
        STATUS_SUCCESS              - The item was removed successfully.
        STATUS_INVALID_PARAMETER_1  - The List parameter is NULL.
        STATUS_INVALID_PARAMETER_2  - The Item parameter is NULL.
        STATUS_NO_MORE_ENTRIES      - The source list is empty.
        STATUS_UNSUCCESSFUL         - The routine failed.
*/
NTSTATUS __stdcall Km_List_RemoveItemEx(
    __in    PKM_LIST    List,
    __in    PLIST_ENTRY Item,
    __in    BOOLEAN     CheckParams,
    __in    BOOLEAN     LockList);

/*
    Km_List_GetCountEx routine.

    Purpose:
        Retrieves the number of items stored in the list.

    Parameters:
        List        - Pointer to KM_LIST structure representing the list.
        Count       - Pointer to ULARGE_INTEGER structure the routine should
                      store the number of items in.
        CheckParams - A boolean value that specifies whether the routine
                      should check the parameters it's being supplied by the caller.
        LockList    - A boolean value that specifies whether the routine
                      should perform the operation in a thread-safe way.

    Return values:
        STATUS_SUCCESS              - The function succeeded.
        STATUS_INVALID_PARAMETER_1  - The List parameter is NULL.
        STATUS_INVALID_PARAMETER_2  - The Count parameter is NULL.
*/
NTSTATUS __stdcall Km_List_GetCountEx(
    __in    PKM_LIST        List,
    __out   PULARGE_INTEGER Count,
    __in    BOOLEAN         CheckParams,
    __in    BOOLEAN         LockList);

/*
    Km_List_ClearEx routine.

    Purpose:
        Clears the list.

    Parameters:
        List            - Pointer to KM_LIST structure representing the list.
        ItemCallback    - An optional callback routine to call for each
                          item being removed.
        CheckParams     - A boolean value that specifies whether the routine
                          should check the parameters it's being supplied by the caller.
        LockList        - A boolean value that specifies whether the routine
                          should perform the operation in a thread-safe way.

    Return values:
        STATUS_SUCCESS              - The function succeeded.
        STATUS_INVALID_PARAMETER_1  - The List parameter is NULL.

    Remarks:
        * The callback function is being executed at the same IRQL as the
          caller of the Km_List_ClearEx routine.
        * The list is already empty at the moment the item callback routine
          is being called.
*/
NTSTATUS __stdcall Km_List_ClearEx(
    __in        PKM_LIST                List,
    __in_opt    PKM_LIST_ITEM_CALLBACK  ItemCallback,
    __in        BOOLEAN                 CheckParams,
    __in        BOOLEAN                 LockList);

/*
    Km_List_FindItemEx routine.

    Purpose:
        Searches the List for the first occurence of the
        item that matches ItemDefinition.

    Parameters:
        List            - Pointer to KM_LIST structure representing the list.
        ItemDefinition  - A caller-supplied caller-specific value that
                          is being passed to the CmpCallback callback.
        CmpCallback     - A caller-supplied callback routine.
                          The routine is being called for each item in the list
                          till it returns 0 or till all the items were examined.
        FoundItem       - An optional pointer to PLIST_ENTRY.
                          Contains the found item upon success.
        CheckParams     - A boolean value that specifies whether the routine
                          should check the parameters it's being supplied by the caller.
        LockList        - A boolean value that specifies whether the routine
                          should perform the operation in a thread-safe way.

    Return values:
        STATUS_SUCCESS              - The routine succeeded.
        STATUS_INVALID_PARAMETER_1  - The List parameter is NULL.
        STATUS_INVALID_PARAMETER_3  - The CmpCallback parameter is NULL.
        STATUS_NOT_FOUND            - No item was found.

    Remarks:
        * The CmpCallback callback function is being executed at IRQL DISPATCH_LEVEL.
*/
NTSTATUS __stdcall Km_List_FindItemEx(
    __in        PKM_LIST                            List,
    __in        PVOID                               ItemDefinition,
    __in        PKM_LIST_ITEM_COMPARISON_CALLBACK   CmpCallback,
    __out_opt   PLIST_ENTRY                         *FoundItem,
    __in        BOOLEAN                             CheckParams,
    __in        BOOLEAN                             LockList);

/*
    Km_List_ExtractEntriesEx routine.

    Purpose:
        Extracts one or more entries from the list
        begining with the first entry.

    Parameters:
        List            - Pointer to KM_LIST structure representing the list.
        DestinationList - Pointer to LIST_ENTRY structure representing
                          the destination list.
        Count           - Pointer to ULARGE_INTEGER structure
                          with the number of entries to extract on input
                          and number of items extracted on output.
                          If the number of items is 0 the 
                          function fails. If Count equals to MAXULONGLONG
                          value then the function extracts all the entries
                          from the list.
        CheckParams     - A boolean value that specifies whether the routine
                          should check the parameters it's being supplied by the caller.
        LockList        - A boolean value that specifies whether the routine
                          should perform the operation in a thread-safe way.

    Return values:
        STATUS_SUCCESS              - The routine succeeded.
        STATUS_INVALID_PARAMETER_1  - The List parameter is NULL.
        STATUS_INVALID_PARAMETER_2  - The DestinationList parameters is NULL.
        STATUS_INVALID_PARAMETER_3  - The Count parameter is NULL or the
                                      value it points to is zero.
*/
NTSTATUS __stdcall Km_List_ExtractEntriesEx(
    __in    PKM_LIST        List,
    __in    PLIST_ENTRY     DestinationList,
    __inout PULARGE_INTEGER Count,
    __in    BOOLEAN         CheckParams,
    __in    BOOLEAN         LockList);

/*
    Km_List_ExtractEntriesEx routine.

    Purpose:
        Extracts list head.

    Parameters:
        List          - Pointer to KM_LIST structure representing the list.
        Entry         - Pointer to the variable to receive pointer to LIST_ENTRY structure.
        CheckParams - A boolean value that specifies whether the routine
                      should check the parameters it's being supplied by the caller.
        LockList    - A boolean value that specifies whether the routine
                      should perform the operation in a thread-safe way.

    Return values:
        STATUS_SUCCESS              - The routine succeeded.
        STATUS_INVALID_PARAMETER_1  - The List parameter is NULL.
        STATUS_INVALID_PARAMETER_2  - The Entry parameter is NULL.
        STATUS_NO_MORE_ENTRIES      - There list is empty.
*/
NTSTATUS __stdcall Km_List_RemoveListHeadEx(
    __in    PKM_LIST    List,
    __out   PLIST_ENTRY *Entry,
    __in    BOOLEAN     CheckParams,
    __in    BOOLEAN     LockList);

/*
    Km_List_Lock routine.

    Purpose:
        Locks the list.
        This is useful when there's a need to perform several operations
        on the list and the caller needs to ensure there's 
        no other thread that can get access to the list inbetween.

    Params:
        List    - Pointer to KM_LIST structure representing the list.

    Return values:
        STATUS_SUCCESS              - The list was successfully locked by the calling thread.
        STATUS_INVALID_PARAMETER    - The List parameter is NULL.        
*/
NTSTATUS __stdcall Km_List_Lock(
    __in    PKM_LIST    List);

/*
    Km_List_Unlock routine.

    Purpose:
        Unlocks a previously locked list.

    Params:
        List    - Pointer to KM_LIST structure representing the list.

    Return values:
        STATUS_SUCCESS              - The list was successfully unlocked.
        STATUS_INVALID_PARAMETER    - The List parameter is NULL.
*/
NTSTATUS __stdcall Km_List_Unlock(
    __in    PKM_LIST    List);

#define Km_List_AddItem(List, Item)         Km_List_AddItemEx(List, Item, TRUE, TRUE)
#define Km_List_RemoveItem(List, Item)      Km_List_RemoveItemEx(List, Item, TRUE, TRUE)
#define Km_List_GetCount(List, CountPtr)    Km_List_GetCountEx(List, CountPtr, TRUE, TRUE)
#define Km_List_Clear(List, ItemCallback)   Km_List_ClearEx(List, ItemCallback, TRUE, TRUE)

#define Km_List_FindItem(List, ItemDefinition, CmpCallback, FoundItemPtr) \
    Km_List_FindItemEx( \
        List, \
        ItemDefinition, \
        CmpCallback, \
        FoundItemPtr, \
        TRUE, \
        TRUE)

#endif