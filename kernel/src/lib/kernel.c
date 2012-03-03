#include <value.h>
#include <vm.h>
#include <gc.h>
#include <exception.h>
#include <string.h>
#include "console.h"
#include "lib.h"

static VAL Kernel;

static VAL Kernel_load_image(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_STRING) {
        js_throw_message(vm, "Kernel.loadImage() expects string");
    }
    js_string_t* str = js_to_js_string_t(argv[0]);
    js_image_t* image = js_image_parse(str->buff, str->length);
    if(!image) {
        js_throw_message(vm, "Couldn't parse image");
    }
    js_vm_exec(vm, image, 0, vm->global_scope, js_value_null(), 0, NULL);
    return js_value_true();
}

static VAL Kernel_memory_usage(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_make_double(js_gc_memory_usage());
}

static VAL Kernel_run_gc(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_gc_run();
    return js_value_undefined();
}

extern int _binary_src_realmode_bin_start;

static VAL Kernel_real_exec(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_STRING) {
        js_throw_message(vm, "Kernel.realExec() expects string");
    }
    js_string_t* str = js_to_js_string_t(argv[0]);
    // copy 16 bit code to 0x9000
    memcpy((void*)0x9000, str->buff, str->length);
    
    // call 32<->16 bit handler which in turn calls into 0x9000
    memcpy((void*)0x8000, &_binary_src_realmode_bin_start, 0x1000);
    ((void(*)())0x8000)();
    
    return js_value_undefined();
}

static VAL Kernel_memcpy(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t dest, src, len;
    js_scan_args(vm, argc, argv, "III", &dest, &src, &len);
    memcpy((void*)dest, (void*)src, len);
    return js_value_undefined();
}

static VAL Kernel_peek8(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr;
    js_scan_args(vm, argc, argv, "I", &addr);
    return js_value_make_double(*(uint8_t*)addr);
}

static VAL Kernel_peek16(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr;
    js_scan_args(vm, argc, argv, "I", &addr);
    return js_value_make_double(*(uint16_t*)addr);
}

static VAL Kernel_peek32(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr;
    js_scan_args(vm, argc, argv, "I", &addr);
    return js_value_make_double(*(uint32_t*)addr);
}

void lib_kernel_init(js_vm_t* vm)
{
    Kernel = js_make_object(vm);
    js_gc_register_global(&Kernel, sizeof(Kernel));
    js_object_put(vm->global_scope->global_object, js_cstring("Kernel"), Kernel);
    
    js_object_put(Kernel, js_cstring("loadImage"), js_value_make_native_function(vm, NULL, js_cstring("loadImage"), Kernel_load_image, NULL));
    js_object_put(Kernel, js_cstring("memoryUsage"), js_value_make_native_function(vm, NULL, js_cstring("memoryUsage"), Kernel_memory_usage, NULL));
    js_object_put(Kernel, js_cstring("runGC"), js_value_make_native_function(vm, NULL, js_cstring("runGC"), Kernel_run_gc, NULL));
    js_object_put(Kernel, js_cstring("realExec"), js_value_make_native_function(vm, NULL, js_cstring("realExec"), Kernel_real_exec, NULL));
    js_object_put(Kernel, js_cstring("memcpy"), js_value_make_native_function(vm, NULL, js_cstring("memcpy"), Kernel_memcpy, NULL));
    js_object_put(Kernel, js_cstring("peek8"), js_value_make_native_function(vm, NULL, js_cstring("peek8"), Kernel_peek8, NULL));
    js_object_put(Kernel, js_cstring("peek16"), js_value_make_native_function(vm, NULL, js_cstring("peek16"), Kernel_peek16, NULL));
    js_object_put(Kernel, js_cstring("peek32"), js_value_make_native_function(vm, NULL, js_cstring("peek32"), Kernel_peek32, NULL));
}