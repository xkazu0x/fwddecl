#ifndef KRUEGER_OS_GFX_C
#define KRUEGER_OS_GFX_C

////////////////////////////////////////////////////
// NOTE: Event Functions (Helpers, Implemented Once)

internal Os_Event *
os_event_list_push(Arena *arena, Os_Event_List *list, Os_Event_Type type) {
  Os_Event *event = push_struct(arena, Os_Event);
  queue_push(list->first, list->last, event);
  event->type = type;
  list->count += 1;
  return(event);
}

#endif // KRUEGER_OS_GFX_C
