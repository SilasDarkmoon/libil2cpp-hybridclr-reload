#ifndef DO_API_NO_RETURN
#define DO_API_NO_RETURN(r, n, p) DO_API(r,n,p)
#endif

DO_API(int, il2cpp_init, (const char* domain_name));
DO_API(int, il2cpp_init_utf16, (const Il2CppChar * domain_name));
DO_API(void, il2cpp_shutdown, ());
DO_API(void, il2cpp_set_config_dir, (const char *config_path));
DO_API(void, il2cpp_set_data_dir, (const char *data_path));
DO_API(void, il2cpp_set_temp_dir, (const char *temp_path));
DO_API(void, il2cpp_set_commandline_arguments, (int argc, const char* const argv[], const char* basedir));
DO_API(void, il2cpp_set_commandline_arguments_utf16, (int argc, const Il2CppChar * const argv[], const char* basedir));
DO_API(void, il2cpp_set_config_utf16, (const Il2CppChar * executablePath));
DO_API(void, il2cpp_set_config, (const char* executablePath));

DO_API(void, il2cpp_set_memory_callbacks, (Il2CppMemoryCallbacks * callbacks));
DO_API(void, il2cpp_memory_pool_set_region_size, (size_t size));
DO_API(size_t, il2cpp_memory_pool_get_region_size, ());
DO_API(const Il2CppImageAdapter*, il2cpp_get_corlib, ());
DO_API(void, il2cpp_add_internal_call, (const char* name, Il2CppMethodPointer method));
DO_API(Il2CppMethodPointer, il2cpp_resolve_icall, (const char* name));

DO_API(void*, il2cpp_alloc, (size_t size));
DO_API(void, il2cpp_free, (void* ptr));

// array
DO_API(Il2CppClassAdapter*, il2cpp_array_class_get, (Il2CppClassAdapter * element_class, uint32_t rank));
DO_API(uint32_t, il2cpp_array_length, (Il2CppArray * array));
DO_API(uint32_t, il2cpp_array_get_byte_length, (Il2CppArray * array));
DO_API(Il2CppArray*, il2cpp_array_new, (Il2CppClassAdapter * elementTypeInfo, il2cpp_array_size_t length));
DO_API(Il2CppArray*, il2cpp_array_new_specific, (Il2CppClassAdapter * arrayTypeInfo, il2cpp_array_size_t length));
DO_API(Il2CppArray*, il2cpp_array_new_full, (Il2CppClassAdapter * array_class, il2cpp_array_size_t * lengths, il2cpp_array_size_t * lower_bounds));
DO_API(Il2CppClassAdapter*, il2cpp_bounded_array_class_get, (Il2CppClassAdapter * element_class, uint32_t rank, bool bounded));
DO_API(int, il2cpp_array_element_size, (const Il2CppClassAdapter * array_class));

// assembly
DO_API(const Il2CppImageAdapter*, il2cpp_assembly_get_image, (const Il2CppAssemblyAdapter * assembly));

// class
DO_API(void, il2cpp_class_for_each, (void(*klassReportFunc)(Il2CppClassAdapter* klass, void* userData), void* userData));
DO_API(const Il2CppTypeAdapter*, il2cpp_class_enum_basetype, (Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_is_inited, (const Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_is_generic, (const Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_is_inflated, (const Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_is_assignable_from, (Il2CppClassAdapter * klass, Il2CppClassAdapter * oklass));
DO_API(bool, il2cpp_class_is_subclass_of, (Il2CppClassAdapter * klass, Il2CppClassAdapter * klassc, bool check_interfaces));
DO_API(bool, il2cpp_class_has_parent, (Il2CppClassAdapter * klass, Il2CppClassAdapter * klassc));
DO_API(Il2CppClassAdapter*, il2cpp_class_from_il2cpp_type, (const Il2CppTypeAdapter * type));
DO_API(Il2CppClassAdapter*, il2cpp_class_from_name, (const Il2CppImageAdapter * image, const char* namespaze, const char *name));
DO_API(Il2CppClassAdapter*, il2cpp_class_from_system_type, (Il2CppReflectionType * type));
DO_API(Il2CppClassAdapter*, il2cpp_class_get_element_class, (Il2CppClassAdapter * klass));
DO_API(const EventInfoAdapter*, il2cpp_class_get_events, (Il2CppClassAdapter * klass, void* *iter));
DO_API(FieldInfoAdapter*, il2cpp_class_get_fields, (Il2CppClassAdapter * klass, void* *iter));
DO_API(Il2CppClassAdapter*, il2cpp_class_get_nested_types, (Il2CppClassAdapter * klass, void* *iter));
DO_API(Il2CppClassAdapter*, il2cpp_class_get_interfaces, (Il2CppClassAdapter * klass, void* *iter));
DO_API(const PropertyInfoAdapter*, il2cpp_class_get_properties, (Il2CppClassAdapter * klass, void* *iter));
DO_API(const PropertyInfoAdapter*, il2cpp_class_get_property_from_name, (Il2CppClassAdapter * klass, const char *name));
DO_API(FieldInfoAdapter*, il2cpp_class_get_field_from_name, (Il2CppClassAdapter * klass, const char *name));
DO_API(const MethodInfoAdapter*, il2cpp_class_get_methods, (Il2CppClassAdapter * klass, void* *iter));
DO_API(const MethodInfoAdapter*, il2cpp_class_get_method_from_name, (Il2CppClassAdapter * klass, const char* name, int argsCount));
DO_API(const char*, il2cpp_class_get_name, (Il2CppClassAdapter * klass));
DO_API(void, il2cpp_type_get_name_chunked, (const Il2CppTypeAdapter * type, void(*chunkReportFunc)(void* data, void* userData), void* userData));
DO_API(const char*, il2cpp_class_get_namespace, (Il2CppClassAdapter * klass));
DO_API(Il2CppClassAdapter*, il2cpp_class_get_parent, (Il2CppClassAdapter * klass));
DO_API(Il2CppClassAdapter*, il2cpp_class_get_declaring_type, (Il2CppClassAdapter * klass));
DO_API(int32_t, il2cpp_class_instance_size, (Il2CppClassAdapter * klass));
DO_API(size_t, il2cpp_class_num_fields, (const Il2CppClassAdapter * enumKlass));
DO_API(bool, il2cpp_class_is_valuetype, (const Il2CppClassAdapter * klass));
DO_API(int32_t, il2cpp_class_value_size, (Il2CppClassAdapter * klass, uint32_t * align));
DO_API(bool, il2cpp_class_is_blittable, (const Il2CppClassAdapter * klass));
DO_API(int, il2cpp_class_get_flags, (const Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_is_abstract, (const Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_is_interface, (const Il2CppClassAdapter * klass));
DO_API(int, il2cpp_class_array_element_size, (const Il2CppClassAdapter * klass));
DO_API(Il2CppClassAdapter*, il2cpp_class_from_type, (const Il2CppTypeAdapter * type));
DO_API(const Il2CppTypeAdapter*, il2cpp_class_get_type, (Il2CppClassAdapter * klass));
DO_API(uint32_t, il2cpp_class_get_type_token, (Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_has_attribute, (Il2CppClassAdapter * klass, Il2CppClassAdapter * attr_class));
DO_API(bool, il2cpp_class_has_references, (Il2CppClassAdapter * klass));
DO_API(bool, il2cpp_class_is_enum, (const Il2CppClassAdapter * klass));
DO_API(const Il2CppImageAdapter*, il2cpp_class_get_image, (Il2CppClassAdapter * klass));
DO_API(const char*, il2cpp_class_get_assemblyname, (const Il2CppClassAdapter * klass));
DO_API(int, il2cpp_class_get_rank, (const Il2CppClassAdapter * klass));
DO_API(uint32_t, il2cpp_class_get_data_size, (const Il2CppClassAdapter * klass));
DO_API(void*, il2cpp_class_get_static_field_data, (const Il2CppClassAdapter * klass));

// testing only
DO_API(size_t, il2cpp_class_get_bitmap_size, (const Il2CppClassAdapter * klass));
DO_API(void, il2cpp_class_get_bitmap, (Il2CppClassAdapter * klass, size_t * bitmap));

// stats
DO_API(bool, il2cpp_stats_dump_to_file, (const char *path));
DO_API(uint64_t, il2cpp_stats_get_value, (Il2CppStat stat));

// domain
DO_API(Il2CppDomainAdapter*, il2cpp_domain_get, ());
DO_API(const Il2CppAssemblyAdapter*, il2cpp_domain_assembly_open, (Il2CppDomainAdapter * domain, const char* name));
DO_API(const Il2CppAssemblyAdapter* const*, il2cpp_domain_get_assemblies, (const Il2CppDomainAdapter * domain, size_t * size));

// exception
DO_API_NO_RETURN(void, il2cpp_raise_exception, (Il2CppExceptionAdapter*));
DO_API(Il2CppExceptionAdapter*, il2cpp_exception_from_name_msg, (const Il2CppImageAdapter * image, const char *name_space, const char *name, const char *msg));
DO_API(Il2CppExceptionAdapter*, il2cpp_get_exception_argument_null, (const char *arg));
DO_API(void, il2cpp_format_exception, (const Il2CppExceptionAdapter * ex, char* message, int message_size));
DO_API(void, il2cpp_format_stack_trace, (const Il2CppExceptionAdapter * ex, char* output, int output_size));
DO_API(void, il2cpp_unhandled_exception, (Il2CppExceptionAdapter*));
DO_API(void, il2cpp_native_stack_trace, (const Il2CppExceptionAdapter * ex, uintptr_t** addresses, int* numFrames, char** imageUUID, char** imageName));

// field
DO_API(int, il2cpp_field_get_flags, (FieldInfoAdapter * field));
DO_API(const char*, il2cpp_field_get_name, (FieldInfoAdapter * field));
DO_API(Il2CppClassAdapter*, il2cpp_field_get_parent, (FieldInfoAdapter * field));
DO_API(size_t, il2cpp_field_get_offset, (FieldInfoAdapter * field));
DO_API(const Il2CppTypeAdapter*, il2cpp_field_get_type, (FieldInfoAdapter * field));
DO_API(void, il2cpp_field_get_value, (Il2CppObject * obj, FieldInfoAdapter * field, void *value));
DO_API(Il2CppObject*, il2cpp_field_get_value_object, (FieldInfoAdapter * field, Il2CppObject * obj));
DO_API(bool, il2cpp_field_has_attribute, (FieldInfoAdapter * field, Il2CppClassAdapter * attr_class));
DO_API(void, il2cpp_field_set_value, (Il2CppObject * obj, FieldInfoAdapter * field, void *value));
DO_API(void, il2cpp_field_static_get_value, (FieldInfoAdapter * field, void *value));
DO_API(void, il2cpp_field_static_set_value, (FieldInfoAdapter * field, void *value));
DO_API(void, il2cpp_field_set_value_object, (Il2CppObject * instance, FieldInfoAdapter * field, Il2CppObject * value));
DO_API(bool, il2cpp_field_is_literal, (FieldInfoAdapter * field));
// gc
DO_API(void, il2cpp_gc_collect, (int maxGenerations));
DO_API(int32_t, il2cpp_gc_collect_a_little, ());
DO_API(void, il2cpp_gc_start_incremental_collection , ());
DO_API(void, il2cpp_gc_disable, ());
DO_API(void, il2cpp_gc_enable, ());
DO_API(bool, il2cpp_gc_is_disabled, ());
DO_API(void, il2cpp_gc_set_mode, (Il2CppGCMode mode));
DO_API(int64_t, il2cpp_gc_get_max_time_slice_ns, ());
DO_API(void, il2cpp_gc_set_max_time_slice_ns, (int64_t maxTimeSlice));
DO_API(bool, il2cpp_gc_is_incremental, ());
DO_API(int64_t, il2cpp_gc_get_used_size, ());
DO_API(int64_t, il2cpp_gc_get_heap_size, ());
DO_API(void, il2cpp_gc_wbarrier_set_field, (Il2CppObject * obj, void **targetAddress, void *object));
DO_API(bool, il2cpp_gc_has_strict_wbarriers, ());
DO_API(void, il2cpp_gc_set_external_allocation_tracker, (void(*func)(void*, size_t, int)));
DO_API(void, il2cpp_gc_set_external_wbarrier_tracker, (void(*func)(void**)));
DO_API(void, il2cpp_gc_foreach_heap, (void(*func)(void* data, void* userData), void* userData));
DO_API(void, il2cpp_stop_gc_world, ());
DO_API(void, il2cpp_start_gc_world, ());
DO_API(void*, il2cpp_gc_alloc_fixed, (size_t size));
DO_API(void, il2cpp_gc_free_fixed, (void* address));
// gchandle
DO_API(uint32_t, il2cpp_gchandle_new, (Il2CppObject * obj, bool pinned));
DO_API(uint32_t, il2cpp_gchandle_new_weakref, (Il2CppObject * obj, bool track_resurrection));
DO_API(Il2CppObject*, il2cpp_gchandle_get_target , (uint32_t gchandle));
DO_API(void, il2cpp_gchandle_free, (uint32_t gchandle));
DO_API(void , il2cpp_gchandle_foreach_get_target, (void(*func)(void* data, void* userData), void* userData));

// vm runtime info
DO_API(uint32_t, il2cpp_object_header_size, ());
DO_API(uint32_t, il2cpp_array_object_header_size, ());
DO_API(uint32_t, il2cpp_offset_of_array_length_in_array_object_header, ());
DO_API(uint32_t, il2cpp_offset_of_array_bounds_in_array_object_header, ());
DO_API(uint32_t, il2cpp_allocation_granularity, ());

// liveness
DO_API(void*, il2cpp_unity_liveness_allocate_struct, (Il2CppClassAdapter * filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata, il2cpp_liveness_reallocate_callback reallocate));
DO_API(void, il2cpp_unity_liveness_calculation_from_root, (Il2CppObject * root, void* state));
DO_API(void, il2cpp_unity_liveness_calculation_from_statics, (void* state));
DO_API(void, il2cpp_unity_liveness_finalize, (void* state));
DO_API(void, il2cpp_unity_liveness_free_struct, (void* state));

// method
DO_API(const Il2CppTypeAdapter*, il2cpp_method_get_return_type, (const MethodInfoAdapter * method));
DO_API(Il2CppClassAdapter*, il2cpp_method_get_declaring_type, (const MethodInfoAdapter * method));
DO_API(const char*, il2cpp_method_get_name, (const MethodInfoAdapter * method));
DO_API(const MethodInfoAdapter*, il2cpp_method_get_from_reflection, (const Il2CppReflectionMethod * method));
DO_API(Il2CppReflectionMethod*, il2cpp_method_get_object, (const MethodInfoAdapter * method, Il2CppClassAdapter * refclass));
DO_API(bool, il2cpp_method_is_generic, (const MethodInfoAdapter * method));
DO_API(bool, il2cpp_method_is_inflated, (const MethodInfoAdapter * method));
DO_API(bool, il2cpp_method_is_instance, (const MethodInfoAdapter * method));
DO_API(uint32_t, il2cpp_method_get_param_count, (const MethodInfoAdapter * method));
DO_API(const Il2CppTypeAdapter*, il2cpp_method_get_param, (const MethodInfoAdapter * method, uint32_t index));
DO_API(Il2CppClassAdapter*, il2cpp_method_get_class, (const MethodInfoAdapter * method));
DO_API(bool, il2cpp_method_has_attribute, (const MethodInfoAdapter * method, Il2CppClassAdapter * attr_class));
DO_API(uint32_t, il2cpp_method_get_flags, (const MethodInfoAdapter * method, uint32_t * iflags));
DO_API(uint32_t, il2cpp_method_get_token, (const MethodInfoAdapter * method));
DO_API(const char*, il2cpp_method_get_param_name, (const MethodInfoAdapter * method, uint32_t index));

// profiler
#if IL2CPP_ENABLE_PROFILER

DO_API(void, il2cpp_profiler_install, (Il2CppProfiler * prof, Il2CppProfileFunc shutdown_callback));
DO_API(void, il2cpp_profiler_set_events, (Il2CppProfileFlags events));
DO_API(void, il2cpp_profiler_install_enter_leave, (Il2CppProfileMethodFuncAdapter enter, Il2CppProfileMethodFuncAdapter fleave));
DO_API(void, il2cpp_profiler_install_allocation, (Il2CppProfileAllocFuncAdapter callback));
DO_API(void, il2cpp_profiler_install_gc, (Il2CppProfileGCFunc callback, Il2CppProfileGCResizeFunc heap_resize_callback));
DO_API(void, il2cpp_profiler_install_fileio, (Il2CppProfileFileIOFunc callback));
DO_API(void, il2cpp_profiler_install_thread, (Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end));

#endif

// property
DO_API(uint32_t, il2cpp_property_get_flags, (PropertyInfoAdapter * prop));
DO_API(const MethodInfoAdapter*, il2cpp_property_get_get_method, (PropertyInfoAdapter * prop));
DO_API(const MethodInfoAdapter*, il2cpp_property_get_set_method, (PropertyInfoAdapter * prop));
DO_API(const char*, il2cpp_property_get_name, (PropertyInfoAdapter * prop));
DO_API(Il2CppClassAdapter*, il2cpp_property_get_parent, (PropertyInfoAdapter * prop));

// object
DO_API(Il2CppClassAdapter*, il2cpp_object_get_class, (Il2CppObject * obj));
DO_API(uint32_t, il2cpp_object_get_size, (Il2CppObject * obj));
DO_API(const MethodInfoAdapter*, il2cpp_object_get_virtual_method, (Il2CppObject * obj, const MethodInfoAdapter * method));
DO_API(Il2CppObject*, il2cpp_object_new, (const Il2CppClassAdapter * klass));
DO_API(void*, il2cpp_object_unbox, (Il2CppObject * obj));

DO_API(Il2CppObject*, il2cpp_value_box, (Il2CppClassAdapter * klass, void* data));

// monitor
DO_API(void, il2cpp_monitor_enter, (Il2CppObject * obj));
DO_API(bool, il2cpp_monitor_try_enter, (Il2CppObject * obj, uint32_t timeout));
DO_API(void, il2cpp_monitor_exit, (Il2CppObject * obj));
DO_API(void, il2cpp_monitor_pulse, (Il2CppObject * obj));
DO_API(void, il2cpp_monitor_pulse_all, (Il2CppObject * obj));
DO_API(void, il2cpp_monitor_wait, (Il2CppObject * obj));
DO_API(bool, il2cpp_monitor_try_wait, (Il2CppObject * obj, uint32_t timeout));

// runtime
DO_API(Il2CppObject*, il2cpp_runtime_invoke, (const MethodInfoAdapter * method, void *obj, void **params, Il2CppExceptionAdapter **exc));
DO_API(Il2CppObject*, il2cpp_runtime_invoke_convert_args, (const MethodInfoAdapter * method, void *obj, Il2CppObject **params, int paramCount, Il2CppExceptionAdapter **exc));
DO_API(void, il2cpp_runtime_class_init, (Il2CppClassAdapter * klass));
DO_API(void, il2cpp_runtime_object_init, (Il2CppObject * obj));

DO_API(void, il2cpp_runtime_object_init_exception, (Il2CppObject * obj, Il2CppExceptionAdapter** exc));

DO_API(void, il2cpp_runtime_unhandled_exception_policy_set, (Il2CppRuntimeUnhandledExceptionPolicy value));

// string
DO_API(int32_t, il2cpp_string_length, (Il2CppString * str));
DO_API(Il2CppChar*, il2cpp_string_chars, (Il2CppString * str));
DO_API(Il2CppString*, il2cpp_string_new, (const char* str));
DO_API(Il2CppString*, il2cpp_string_new_len, (const char* str, uint32_t length));
DO_API(Il2CppString*, il2cpp_string_new_utf16, (const Il2CppChar * text, int32_t len));
DO_API(Il2CppString*, il2cpp_string_new_wrapper, (const char* str));
DO_API(Il2CppString*, il2cpp_string_intern, (Il2CppString * str));
DO_API(Il2CppString*, il2cpp_string_is_interned, (Il2CppString * str));

// thread
DO_API(Il2CppThread*, il2cpp_thread_current, ());
DO_API(Il2CppThread*, il2cpp_thread_attach, (Il2CppDomainAdapter * domain));
DO_API(void, il2cpp_thread_detach, (Il2CppThread * thread));

DO_API(Il2CppThread**, il2cpp_thread_get_all_attached_threads, (size_t * size));
DO_API(bool, il2cpp_is_vm_thread, (Il2CppThread * thread));

// stacktrace
DO_API(void, il2cpp_current_thread_walk_frame_stack, (Il2CppFrameWalkFuncAdapter func, void* user_data));
DO_API(void, il2cpp_thread_walk_frame_stack, (Il2CppThread * thread, Il2CppFrameWalkFuncAdapter func, void* user_data));
DO_API(bool, il2cpp_current_thread_get_top_frame, (Il2CppStackFrameInfoAdapter * frame));
DO_API(bool, il2cpp_thread_get_top_frame, (Il2CppThread * thread, Il2CppStackFrameInfoAdapter * frame));
DO_API(bool, il2cpp_current_thread_get_frame_at, (int32_t offset, Il2CppStackFrameInfoAdapter * frame));
DO_API(bool, il2cpp_thread_get_frame_at, (Il2CppThread * thread, int32_t offset, Il2CppStackFrameInfoAdapter * frame));
DO_API(int32_t, il2cpp_current_thread_get_stack_depth, ());
DO_API(int32_t, il2cpp_thread_get_stack_depth, (Il2CppThread * thread));
DO_API(void, il2cpp_override_stack_backtrace, (Il2CppBacktraceFunc stackBacktraceFunc));

// type
DO_API(Il2CppObject*, il2cpp_type_get_object, (const Il2CppTypeAdapter * type));
DO_API(int, il2cpp_type_get_type, (const Il2CppTypeAdapter * type));
DO_API(Il2CppClassAdapter*, il2cpp_type_get_class_or_element_class, (const Il2CppTypeAdapter * type));
DO_API(char*, il2cpp_type_get_name, (const Il2CppTypeAdapter * type));
DO_API(bool, il2cpp_type_is_byref, (const Il2CppTypeAdapter * type));
DO_API(uint32_t, il2cpp_type_get_attrs, (const Il2CppTypeAdapter * type));
DO_API(bool, il2cpp_type_equals, (const Il2CppTypeAdapter * type, const Il2CppTypeAdapter * otherType));
DO_API(char*, il2cpp_type_get_assembly_qualified_name, (const Il2CppTypeAdapter * type));
DO_API(char*, il2cpp_type_get_reflection_name, (const Il2CppTypeAdapter * type));
DO_API(bool, il2cpp_type_is_static, (const Il2CppTypeAdapter * type));
DO_API(bool, il2cpp_type_is_pointer_type, (const Il2CppTypeAdapter * type));

// image
DO_API(const Il2CppAssemblyAdapter*, il2cpp_image_get_assembly, (const Il2CppImageAdapter * image));
DO_API(const char*, il2cpp_image_get_name, (const Il2CppImageAdapter * image));
DO_API(const char*, il2cpp_image_get_filename, (const Il2CppImageAdapter * image));
DO_API(const MethodInfoAdapter*, il2cpp_image_get_entry_point, (const Il2CppImageAdapter * image));

DO_API(size_t, il2cpp_image_get_class_count, (const Il2CppImageAdapter * image));
DO_API(const Il2CppClassAdapter*, il2cpp_image_get_class, (const Il2CppImageAdapter * image, size_t index));

// Memory information
DO_API(Il2CppManagedMemorySnapshot*, il2cpp_capture_memory_snapshot, ());
DO_API(void, il2cpp_free_captured_memory_snapshot, (Il2CppManagedMemorySnapshot * snapshot));

DO_API(void, il2cpp_set_find_plugin_callback, (Il2CppSetFindPlugInCallback method));

// Logging
DO_API(void, il2cpp_register_log_callback, (Il2CppLogCallback method));

// Debugger
DO_API(void, il2cpp_debugger_set_agent_options, (const char* options));
DO_API(bool, il2cpp_is_debugger_attached, ());
DO_API(void, il2cpp_register_debugger_agent_transport, (Il2CppDebuggerTransport * debuggerTransport));

// Debug metadata
DO_API(bool, il2cpp_debug_get_method_info, (const MethodInfoAdapter*, Il2CppMethodDebugInfo * methodDebugInfo));

// TLS module
DO_API(void, il2cpp_unity_install_unitytls_interface, (const void* unitytlsInterfaceStruct));

// custom attributes
DO_API(Il2CppCustomAttrInfo*, il2cpp_custom_attrs_from_class, (Il2CppClassAdapter * klass));
DO_API(Il2CppCustomAttrInfo*, il2cpp_custom_attrs_from_method, (const MethodInfoAdapter * method));
DO_API(Il2CppCustomAttrInfo*, il2cpp_custom_attrs_from_field, (const FieldInfoAdapter * field));

DO_API(Il2CppObject*, il2cpp_custom_attrs_get_attr, (Il2CppCustomAttrInfo * ainfo, Il2CppClassAdapter * attr_klass));
DO_API(bool, il2cpp_custom_attrs_has_attr, (Il2CppCustomAttrInfo * ainfo, Il2CppClassAdapter * attr_klass));
DO_API(Il2CppArray*,  il2cpp_custom_attrs_construct, (Il2CppCustomAttrInfo * cinfo));

DO_API(void, il2cpp_custom_attrs_free, (Il2CppCustomAttrInfo * ainfo));

// Il2CppClass user data for GetComponent optimization
DO_API(void, il2cpp_class_set_userdata, (Il2CppClassAdapter * klass, void* userdata));
DO_API(int, il2cpp_class_get_userdata_offset, ());

DO_API(void, il2cpp_set_default_thread_affinity, (int64_t affinity_mask));

// Android
DO_API(void, il2cpp_unity_set_android_network_up_state_func, (Il2CppAndroidUpStateFunc func));
