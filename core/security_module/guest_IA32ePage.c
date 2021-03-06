/*!	\file guest_IA32ePage.c
	\brief defines functions that are related to IA32e paging mode (for guest)

*/
#include <guest_IA32ePage.h>
#include <monitor_util.h>
#include <monitor_types.h>

void traverseIA32ePages(const VMID_t vmid, const APPID_t appID, const GPA_t startGPAofPageTable, void* (*do_something)(const VMID_t vmID, const APPID_t appID, GPA_t gpa))
{
	int pml4Index;
	for(pml4Index = 0 ; pml4Index < MAX_GUEST_PML4_INDEX ; pml4Index++)
	{
		GPA_t currentPML4EntryGPA;
		HPA_t currentPML4EntryHPA;
		PGT_ENTRY_t *pCurrentPML4Entry, currentPML4Entry;
		GPA_t startGPAofPDPT;

		currentPML4EntryGPA = (startGPAofPageTable | (pml4Index << GUEST_PML4_INDEX_SHIFT));
		currentPML4EntryHPA = gpaToHPA(currentPML4EntryGPA, 0);
		if(!currentPML4EntryHPA)
		{
			continue;
		}
		pCurrentPML4Entry = (PGT_ENTRY_t*)mapHPAintoHVA(currentPML4EntryHPA,sizeof(PGT_ENTRY_t));
		if(!pCurrentPML4Entry)
		{
			continue;
		}
		currentPML4Entry = *pCurrentPML4Entry;
		unmapHPAintoHVA((void*)pCurrentPML4Entry,sizeof(PGT_ENTRY_t));

		startGPAofPDPT = currentPML4Entry & EPT_PML4_ENTRY_MASK;
		
		if(startGPAofPDPT && (currentPML4Entry & 0x01) && !(currentPML4Entry & 0x80))
		{
			printf("%llx\n",currentPML4Entry);
			traverseIA32ePDPT(vmid, appID, startGPAofPDPT,do_something);
		}
	}
}

inline void traverseIA32ePDPT(const VMID_t vmid, const APPID_t appID, const GPA_t startGPAofPDPT, void* (*do_something)(const VMID_t vmID, const APPID_t appID, GPA_t gpa))
{
	int pdpIndex;
	for(pdpIndex = 0 ; pdpIndex < MAX_GUEST_PDP_INDEX ; pdpIndex++)
	{
		GPA_t currentPDPTEntryGPA;
		HPA_t currentPDPTEntryHPA;
		PGT_ENTRY_t *pCurrentPDPTEntry, currentPDPTEntry;
		GPA_t startGPAofPDT;

		currentPDPTEntryGPA = (startGPAofPDPT | (pdpIndex << GUEST_PDP_INDEX_SHIFT));
		currentPDPTEntryHPA = gpaToHPA(currentPDPTEntryGPA, 0);
		if(!currentPDPTEntryHPA)
		{
			continue;
		}
		pCurrentPDPTEntry = (PGT_ENTRY_t*)mapHPAintoHVA(currentPDPTEntryHPA,sizeof(PGT_ENTRY_t));
		if(!pCurrentPDPTEntry)
		{
			continue;
		}
		currentPDPTEntry = *pCurrentPDPTEntry;
		unmapHPAintoHVA((void*)pCurrentPDPTEntry,sizeof(PGT_ENTRY_t));

		startGPAofPDT = currentPDPTEntry & EPT_PDP_ENTRY_MASK;	

		if(startGPAofPDT && (currentPDPTEntry & 0x01) && !(currentPDPTEntry & 0x80))
		{
			traverseIA32ePDT(vmid, appID, startGPAofPDT, do_something);
		}
	}
}


inline void traverseIA32ePDT(const VMID_t vmid, const APPID_t appID, const GPA_t startGPAofPDT, void* (*do_something)(const VMID_t vmID, const APPID_t appID, GPA_t gpa))
{
	int pdIndex;
	for(pdIndex = 0 ; pdIndex < MAX_GUEST_PD_INDEX ; pdIndex++)
	{
		GPA_t currentPDTEntryGPA;
		HPA_t currentPDTEntryHPA;
		PGT_ENTRY_t *pCurrentPDTEntry, currentPDTEntry;
		GPA_t startGPAofPT;

		currentPDTEntryGPA = (startGPAofPDT | (pdIndex << GUEST_PD_INDEX_SHIFT));
		currentPDTEntryHPA = gpaToHPA(currentPDTEntryGPA, 0);
		if(!currentPDTEntryHPA)
		{
			continue;
		}
		pCurrentPDTEntry = (PGT_ENTRY_t*)mapHPAintoHVA(currentPDTEntryHPA,sizeof(PGT_ENTRY_t));
		if(!pCurrentPDTEntry)
		{
			continue;
		}
		currentPDTEntry = *pCurrentPDTEntry;
		unmapHPAintoHVA((void*)pCurrentPDTEntry,sizeof(PGT_ENTRY_t));

		startGPAofPT = currentPDTEntry & EPT_PD_ENTRY_MASK;
		
		if(startGPAofPT && (currentPDTEntry & 0x01) && !(currentPDTEntry & 0x80))
		{
			traverseIA32ePT(vmid, appID, startGPAofPT,do_something);			
		}

	}
}

inline void traverseIA32ePT(const VMID_t vmid, const APPID_t appID, const GPA_t startGPAofPT, void* (*do_something)(const VMID_t vmID, const APPID_t appID, GPA_t gpa))
{
	int ptIndex;
	for(ptIndex = 0 ; ptIndex < MAX_GUEST_PT_INDEX ; ptIndex++)
	{
		GPA_t currentPTEntryGPA;
		HPA_t currentPTEntryHPA;
		PGT_ENTRY_t *pCurrentPTEntry, currentPTEntry;
		GPA_t pageGPA;

		currentPTEntryGPA = (startGPAofPT | (ptIndex << GUEST_PT_INDEX_SHIFT));
		currentPTEntryHPA = gpaToHPA(currentPTEntryGPA, 0);
		if(!currentPTEntryHPA)
		{
			continue;
		}
		pCurrentPTEntry = (PGT_ENTRY_t*)mapHPAintoHVA(currentPTEntryHPA,sizeof(PGT_ENTRY_t));
		if(!pCurrentPTEntry)
		{
			continue;
		}
		currentPTEntry = *pCurrentPTEntry;
		unmapHPAintoHVA((void*)pCurrentPTEntry,sizeof(PGT_ENTRY_t));

		pageGPA = currentPTEntry & EPT_PT_ENTRY_MASK;
		if(do_something && (currentPTEntry & 0x01))
		{

			(*do_something)(vmid, appID, pageGPA);			
		}

	}
}