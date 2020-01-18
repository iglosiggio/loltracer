#ifndef __JITDUMP_H__
#define __JITDUMP_H__
void jitdump_open();
size_t jitdump_emit_load(const char* name, void* addr, size_t size, size_t offset);
void jitdump_close();
#endif /* __JITDUMP_H__ */
