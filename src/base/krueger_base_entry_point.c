#ifndef KRUEGER_BASE_ENTRY_POINT_C
#define KRUEGER_BASE_ENTRY_POINT_C

internal void
base_entry_point(int argc, char **argv) {
  Thread_Context *tctx = thread_context_alloc();
  thread_context_select(tctx);
  os_core_init();
#if defined(KRUEGER_OS_GFX_H) && !defined(OS_GFX_INIT_MANUAL)
  os_graphics_init();
#endif
#if defined(KRUEGER_OPENGL_H) && !defined(OGL_INIT_MANUAL)
  ogl_init();
#endif
  entry_point(argc, argv);
  os_core_shutdown();
  thread_context_release(tctx);
}

#endif // KRUEGER_BASE_ENTRY_POINT_C
