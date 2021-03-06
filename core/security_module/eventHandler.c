/**	\file eventHandler.c
	\brief Catching & handling of memory-related events.
 *
 */

#include <eventHandler.h>
#include <monitor_types.h>
#include <guest_state.h>
#include <memory_ownership_table.h>

void hypercall_handler()
{
	GPA_t pagetable_start_GPA;
	pagetable_start_GPA = get_page_table_base_GPA();

	if(pagetable_start_GPA)
	{
		printf("Protection start\n");
		protectCurrentApplication();
	}
}


