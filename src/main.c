#include "base/krueger_base.h"
#include "platform/krueger_platform.h"

#include "base/krueger_base.c"
#include "platform/krueger_platform.c"

int
main(void) {
  platform_core_init();

  Thread_Context *thread_context = thread_context_alloc();
  thread_context_select(thread_context);

  log_info("goodbye world");

  platform_core_shutdown();
  return(0);
}
