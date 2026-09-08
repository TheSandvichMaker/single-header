//
// Constants
//

@const HYDRA_MAX_SWAPCHAINS               = 128
@const HYDRA_MAX_SWAPCHAIN_TEXTURES       = 8
@const HYDRA_MAX_FRAMES_IN_FLIGHT         = 3
@const HYDRA_MAX_DEBUG_NAME_LENGTH        = 64
@const HYDRA_MAX_PIPELINES                = 65536
@const HYDRA_MAX_RESOURCE_LAYOUTS         = 65536
@const HYDRA_MAX_ACCELERATION_STRUCTURES  = 65536
@const HYDRA_MAX_BUFFERS                  = 65536
@const HYDRA_MAX_TEXTURES                 = 65536
@const HYDRA_MAX_TEXTURE_VIEWS            = 65536
@const HYDRA_MAX_SAMPLERS                 = 1024 
@const HYDRA_MAX_TIMING_HEAPS             = 128
@const HYDRA_MAX_COLOR_ATTACHMENTS        = 8
@const HYDRA_MAX_SHADER_RESOURCE_BINDINGS = 64
@const HYDRA_MAX_TEXTURE_COPY_REGIONS     = 16
@const HYDRA_MAX_RESOURCE_BUNDLES         = 3
@const HYDRA_SHADER_KIND_COUNT            = 3
@const HYDRA_WHOLE_BUFFER_SIZE            = 0x7FFFFFFFFFFFFFFF

//
// Typedefs
//

@alias Device_Address = u64

//
// Enums
//

@enum Error {
	none                           = 0
	failed                         = 1
	not_implemented                = 2
	allocator_out_of_memory        = 3
	allocator_invalid_pointer      = 4
	allocator_invalid_argument     = 5
	allocator_mode_not_implemented = 6
	invalid_argument               = 7
	out_of_handles                 = 8
	invalid_handle                 = 9
	unsupported_format             = 10
	swapchain_out_of_date          = 11
	not_ready                      = 12
	over_capacity                  = 13
}

@enum Validation_Severity {
	debug   = 0
	info    = 1
	warning = 2
	error   = 3
}

@enum Allocator_Mode {
	allocate = 0
	free     = 1
}

// Device

@enum Device_Object_Kind {
	none            = 0
	buffer          = 1
	texture         = 2
	texture_view    = 3
	sampler         = 4
	pipeline        = 5
	swapchain       = 6
	resource_bundle = 7
	timing_heap     = 8
	resource_layout = 9
}

@enum Backend {
	none   = 0
	vulkan = 1
	d3d12  = 2
	metal  = 3
	mock   = 4
}

@enum Queue_Kind {
	graphics = 0
	compute  = 1
	upload   = 2
}

@enum Pixel_Format {
	// This list of formats is primarily based on webgpu's as inspired by sokol gfx which cites broad support across hardware for all these formats.
	unknown             = 0

	// 8 bit formats
	r8_unorm            = 1
	r8_snorm            = 2
	r8_uint             = 3
	r8_sint             = 4

	// 16 bit formats
	r16_unorm           = 5
	r16_snorm           = 6
	r16_uint            = 7
	r16_sint            = 8
	r16_float           = 9
	r8g8_unorm          = 10
	r8g8_snorm          = 11
	r8g8_uint           = 12
	r8g8_sint           = 13

	// 32 bit formats
	r32_uint            = 14
	r32_sint            = 15
	r32_float           = 16
	r16g16_unorm        = 17
	r16g16_snorm        = 18
	r16g16_uint         = 19
	r16g16_sint         = 20
	r16g16_float        = 21
	r8g8b8a8_unorm      = 22
	r8g8b8a8_unorm_srgb = 23
	r8g8b8a8_snorm      = 24
	r8g8b8a8_uint       = 25
	r8g8b8a8_sint       = 26
	b8g8r8a8_unorm      = 27
	b8g8r8a8_unorm_srgb = 28
	r10g10b10a2_unorm   = 29
	r11g11b10_float     = 30
	r9g9b9e5_shared_exp = 31

	// 64 bit formats
	r32g32_uint         = 32
	r32g32_sint         = 33
	r32g32_float        = 34
	r16g16b16a16_unorm  = 35
	r16g16b16a16_snorm  = 36
	r16g16b16a16_uint   = 37
	r16g16b16a16_sint   = 38
	r16g16b16a16_float  = 39

	// 96 bit formats
	r32g32b32_float     = 40

	// 128 bit formats
	r32g32b32a32_uint   = 41
	r32g32b32a32_sint   = 42
	r32g32b32a32_float  = 43

	// depth/stencil formats
	d16_unorm           = 44
	d32_float           = 45
	d32_float_s8_uint   = 46
	d24_unorm_s8_uint   = 47

	// block-compressed formats
	bc1_rgba_unorm      = 48
	bc1_rgba_unorm_srgb = 49
	bc2_rgba_unorm      = 50
	bc2_rgba_unorm_srgb = 51
	bc3_rgba_unorm      = 52
	bc3_rgba_unorm_srgb = 53
	bc4_r_unorm         = 54
	bc4_r_snorm         = 55
	bc5_rg_unorm        = 56
	bc5_rg_snorm        = 57
	bc6h_rgb_ufloat     = 58
	bc6h_rgb_float      = 59
	bc7_rgba_unorm      = 60
	bc7_rgba_unorm_srgb = 61

	// video formats
	nv12                = 62 // also known as g8_b8r8_2plane_420_unorm in Vulkan
	// TODO(daniel): Other video formats?
	// TODO(daniel): Other compressed formats?
}

@enum(type=u8) Index_Format {
	u16 = 0
	u32 = 1
}

@enum Present_Mode {
	discard    = 0 // maps to DXGI_SWAP_EFFECT_FLIP_DISCARD or VK_PRESENT_MODE_MAILBOX_KHR
	sequential = 1 // maps to DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL or VK_PRESENT_MODE_FIFO_KHR
}

@enum Memory_Kind {
	default     = 0
	upload      = 1
	readback    = 2
	gpu_upload  = 3
	memoryless  = 4 // Falls back to default if not available
}

@enum Resource_Kind {
	buffer  = 0
	texture = 1
}

@hidden(c=true)
@enum Buffer_Flags_Enum {
	allow_uav                                = 0
	allow_index                              = 1
	allow_indirect_argument                  = 2
	allow_acceleration_structure_build_input = 3
	allow_acceleration_structure             = 4
}

@bit_set Buffer_Flags {
	enum = Buffer_Flags_Enum
	type = u32
}

@enum Texture_Dimension {
	_1d  = 0
	_2d  = 1
	_3d  = 2
	cube = 3
}

@hidden(c=true)
@enum Texture_Flags_Enum {
	allow_uav            = 0
	allow_rendertarget   = 1
	allow_depth_stencil  = 2
	allow_shader_atomics = 3
	deny_srv             = 4
}

@bit_set Texture_Flags {
	enum = Texture_Flags_Enum
	type = u32
}

@hidden(c=true)
@enum Texture_Aspect {
	default = 0
	srgb    = 1
	stencil = 2
	plane_0 = 3
	plane_1 = 4
	plane_2 = 5
	custom  = 31
}

@bit_set Texture_Aspect_Mask {
	enum = Texture_Aspect
	type = u32
}

// Shaders Pipelines

@enum Attachment_Mode {
	none       = 0
	read       = 1
	read_write = 2
}

@hidden(c=true)
@enum Shader_Kind_Enum {
	vertex  = 0
	pixel   = 1
	compute = 2
}

@bit_set Shader_Kind {
	enum = Shader_Kind_Enum
	type = u32
}

@enum Descriptor_Kind {
	none                   = 0
	cbv                    = 1
	buffer_srv             = 2
	texture_srv            = 3
	acceleration_structure = 4
	buffer_uav             = 5
	texture_uav            = 6
	sampler                = 7
	FIRST_SRV              = buffer_srv
	LAST_SRV               = acceleration_structure
	FIRST_UAV              = buffer_uav
	LAST_UAV               = texture_uav
}

@hidden(c=true)
@enum Resource_Layout_Flags_Enum {
	vulkan_use_push_descriptors = 0
}

@bit_set Resource_Layout_Flags {
	enum = Resource_Layout_Flags_Enum
	type = u32
}

@enum Register {
	b0  =        0,  s0  = 1000 + 0,
	b1  =        1,  s1  = 1000 + 1,
	b2  =        2,  s2  = 1000 + 2,
	b3  =        3,  s3  = 1000 + 3,
	b4  =        4,  s4  = 1000 + 4,
	b5  =        5,  s5  = 1000 + 5,
	b6  =        6,  s6  = 1000 + 6,
	b7  =        7,  s7  = 1000 + 7,
	b8  =        8,  s8  = 1000 + 8,
	b9  =        9,  s9  = 1000 + 9,
	b10 =        10, s10 = 1000 + 10,
	b11 =        11, s11 = 1000 + 11,
	b12 =        12, s12 = 1000 + 12,
	b13 =        13, s13 = 1000 + 13,
	b14 =        14, s14 = 1000 + 14,
	b15 =        15, s15 = 1000 + 15,

	t0  = 2000 + 0,  u0  = 3000 + 0,
	t1  = 2000 + 1,  u1  = 3000 + 1,
	t2  = 2000 + 2,  u2  = 3000 + 2,
	t3  = 2000 + 3,  u3  = 3000 + 3,
	t4  = 2000 + 4,  u4  = 3000 + 4,
	t5  = 2000 + 5,  u5  = 3000 + 5,
	t6  = 2000 + 6,  u6  = 3000 + 6,
	t7  = 2000 + 7,  u7  = 3000 + 7,
	t8  = 2000 + 8,  u8  = 3000 + 8,
	t9  = 2000 + 9,  u9  = 3000 + 9,
	t10 = 2000 + 10, u10 = 3000 + 10,
	t11 = 2000 + 11, u11 = 3000 + 11,
	t12 = 2000 + 12, u12 = 3000 + 12,
	t13 = 2000 + 13, u13 = 3000 + 13,
	t14 = 2000 + 14, u14 = 3000 + 14,
	t15 = 2000 + 15, u15 = 3000 + 15,

	bMAX = b15,
	sMAX = s15,
	tMAX = t15,
	uMAX = u15,
}

@enum Load_Op {
	dont_care = 0
	load      = 1
	clear     = 2
}

@enum Store_Op {
	dont_care = 0
	store     = 1
}

@enum Clear_Value_Kind {
	default  = 0
	float_32 = 1
	uint_32  = 2
}

@enum Pipeline_Kind {
	graphics   = 0
	compute    = 1
}

@enum Primitive_Topology_Class {
	none     = 0
	point    = 1
	line     = 2
	triangle = 4
}

@enum(type=u8) Primitive_Topology {
	none               = 0
	point_list         = 1
	line_list          = 2
	line_strip         = 3
	triangle_list      = 4
	triangle_strip     = 5
	line_list_adj      = 6
	line_strip_adj     = 7
	triangle_list_adj  = 8
	triangle_strip_adj = 9
}

@enum Fill_Mode {
	solid     = 0
	wireframe = 1
}

@enum Cull_Mode {
	none  = 0
	back  = 1
	front = 2
}

@enum Winding {
	counter_clockwise = 0
	clockwise         = 1
}

@enum Comparison_Func {
	never            = 0
	less             = 1
	equal            = 2
	less_or_equal    = 3
	greater          = 4
	not_equal        = 5
	greater_or_equal = 6
	always           = 7
}

@enum Blend {
	zero             = 0
	one              = 1
	src_color        = 2
	inv_src_color    = 3
	dest_color       = 4
	inv_dest_color   = 5
	src_alpha        = 6
	inv_src_alpha    = 7
	dest_alpha       = 8
	inv_dest_alpha   = 9
	src_alpha_sat    = 10
	blend_factor     = 11
	inv_blend_factor = 12
	src1_color       = 13
	inv_src1_color   = 14
	src1_alpha       = 15
	inv_src1_alpha   = 16
	alpha_factor     = 17
	inv_alpha_factor = 18
}

@enum Op {
	add          = 0
	subtract     = 1
	rev_subtract = 2
	min          = 3
	max          = 4
}

@enum Logic_Op {
	clear         = 0
	set           = 1
	copy          = 2
	copy_inverted = 3
	noop          = 4
	invert        = 5
	and           = 6
	nand          = 7
	or            = 8
	nor           = 9
	xor           = 10
	equiv         = 11
	and_reverse   = 12
	and_inverted  = 13
	or_reverse    = 14
	or_inverted   = 15
}

@enum Stencil_Op {
	keep     = 0
	zero     = 1
	replace  = 2
	incr_sat = 3
	decr_sat = 4
	invert   = 5
	incr     = 6
	decr     = 7
}

@enum Color_Write_Mask_Enum {
	red   = 0
	green = 1
	blue  = 2
	alpha = 3
}

@bit_set Color_Write_Mask {
	enum = Color_Write_Mask
	type = u8
}

// Commands

@enum Command_Kind {
	none                          = 0
	begin_rendering               = 1
	draws                         = 4
	draws_indirect                = 5
	end_rendering                 = 6
	dispatch                      = 7
	dispatch_indirect             = 8
	begin_region                  = 10
	end_region                    = 11
	begin_timing                  = 12
	end_timing                    = 13
	resolve_timings               = 14
	copy_buffer                   = 15
	copy_texture                  = 16
	update_texture                = 17
	update_acceleration_structure = 18
	barriers_2                    = 19
}

@enum Filter {
	nearest = 0
	linear  = 1
}

@enum Address_Mode {
	wrap        = 0
	mirror      = 1
	clamp       = 2
	border      = 3
	mirror_once = 4
}

// Synchronization

@hidden(c=true)
@enum Sync_Flags_Enum {
	Sync_Flag_indirect               = 0 
	Sync_Flag_vertex                 = 1
	Sync_Flag_pixel                  = 2
	Sync_Flag_compute                = 3
	Sync_Flag_amplification          = 4
	Sync_Flag_mesh                   = 5
	Sync_Flag_render_target          = 6
	Sync_Flag_depth_stencil          = 7
	Sync_Flag_copy                   = 8
	Sync_Flag_clear                  = 9
	Sync_Flag_acceleration_structure = 10
}

@bit_set Sync_Flags {
	enum = Sync_Flags_Enum
	type = u32

	composites {
		any_shader [ vertex, pixel, compute, amplification, mesh ]
		all        [ indirect, vertex, pixel, compute, amplification, mesh, render_target, depth_stencil, copy, clear, acceleration_structure ]
	}
}

@enum Texture_Layout {
	undefined             = 0
	general               = 1
	present               = 2
	render_target         = 3
	unordered_access      = 4
	depth_stencil_write   = 5
	depth_stencil_read    = 6
	shader_resource       = 7
	copy_source           = 8
	copy_dest             = 9
	resolve_source        = 10
	resolve_dest          = 11
	shading_rate_source   = 12
	video_decode_read     = 13
	video_decode_write    = 14
	video_process_read    = 15
	video_process_write   = 16
	video_encode_read     = 17
	video_encode_write    = 18
}

@enum Shader_Bytecode_Format {
	unknown  = 0
	dxbc     = 1
	dxil     = 2
	spirv    = 4
	metallib = 5
}

@enum Acceleration_Structure_Kind {
	top_level    = 0
	bottom_level = 1
}

@enum Acceleration_Structure_Build_Node {
	build  = 0
	update = 1
}

@hidden(c=true)
@enum Acceleration_Structure_Build_Flags_Enum {
	prefer_fast_trace = 0
	prefer_fast_build = 1
}

@bit_set Acceleration_Structure_Build_Flags {
	enum = Acceleration_Structure_Build_Flags_Enum
	type = u32
}

@hidden(c=true)
@enum Acceleration_Structure_Geometry_Flags_Enum {
	opaque = 0
}

@bit_set Acceleration_Structure_Geometry_Flags {
	enum = Acceleration_Structure_Geometry_Flags_Enum
	type = u32
}

@enum Acceleration_Structure_Geometry_Kind {
	triangles = 0
	aabbs     = 1
}

//
// Structs
//

@hidden(odin=true)
@struct String {
	data = [^]byte
	len  = int
}

@struct Source_Code_Location {
	file_path = cstring
	procedure = cstring
	line      = i32
}

@fn_ptr(returns=Error) Allocator_Proc { user_data = rawptr, mode = Allocator_Mode, size = int, align = int, old_memory = rawptr, result = ^rawptr }

@struct Allocator {
	procedure = Allocator_Proc
	user_data = rawtpr
}

@alias(distinct=true) Buffer                 = u32
@alias(distinct=true) Texture                = u32
@alias(distinct=true) Texture_View           = u32
@alias(distinct=true) Sampler                = u32
@alias(distinct=true) Pipeline               = u32
@alias(distinct=true) Swapchain              = u32
@alias(distinct=true) Resource_Bundle        = u32
@alias(distinct=true) Timing_Heap            = u32
@alias(distinct=true) Resource_Layout        = u32
@alias(distinct=true) Acceleration_Structure = u32
@alias(distinct=true) Dynamic_Constants      = u32

@struct Vulkan_Instance_Desc {
	enable_validation_layers = bool
	application_name         = string
	engine_name              = string
}

@struct D3D12_Instance_Desc {
	enable_debug_layer          = bool
	enable_gpu_based_validation = bool
}

@struct Validation_Message_Desc {
	severity = Validation_Severity
	message  = string // null-terminated
	ctx      = string // null-terminated
}

@fn_ptr(returns=bool) Validation_Message_Proc { user_data = rawptr, @by_ptr desc = Validation_Message_Desc }

@struct Instance_Desc {
	allocator = Allocator

	validation_message_proc           = Validation_Message_Proc
	validation_message_proc_user_data = rawptr

	vulkan = Vulkan_Instance_Desc
	d3d12  = D3D12_Instance_Desc
}

@struct Color {
	@union _ {
		@struct _ {
			r = u8
			g = u8
			b = u8
			a = u8
		}
		rgba = u32
		e    = [4]u8
	}
}

@struct Device_Properties {
	@bit_field(bits=1) regions_available                        = bool
	@bit_field(bits=1) regions_enabled                          = bool
	@bit_field(bits=1) timing_enabled                           = bool
	@bit_field(bits=1) structured_buffers_require_element_align = bool
	@bit_field(bits=1) raytracing_available                     = bool

	min_constant_buffer_alignment   = int
	min_structured_buffer_alignment = int
	timestamp_period                = u64
}

@struct Rect {
	min = [2]i32
	max = [2]i32
}

@struct Box {
	min = [3]i32
	max = [3]i32
}

@struct Viewport {
	x         = f32
	y         = f32
	width     = f32
	height    = f32
	min_depth = f32
	max_depth = f32
}

@struct Timed_Region {
	start = u64
	end   = u64
}

@struct Pixel_Format_Info {
	non_srgb_equivalent = Pixel_Format
	srgb_equivalent     = Pixel_Format
	srv_depth_format    = Pixel_Format
	depth_compatible    = bool
	stencil_compatible  = bool
	plane_count         = u8
}

@struct Vulkan_Device_Desc {
	placeholder = i32
}

@struct D3D12_Device_Desc {
	placeholder = i32
}

@struct Metal_Device_Desc {
	placeholder = i32
}

@struct Mock_Device_Desc {
	placeholder = i32
}

@struct Device_Desc {
	allocator = Allocator

	maximum_frames_in_flight = u64 // TODO(daniel): Hardcode to 3?

	@bit_field(bits=1) list_validation_enabled = bool
	@bit_field(bits=1) raytracing              = bool
	@bit_field(bits=1) regions                 = bool
	@bit_field(bits=1) timing                  = bool

	backend = Backend

	@union _ {
		vulkan = Vulkan_Device_Desc
		d3d12  = D3D12_Device_Desc
		metal  = Metal_Device_Desc
		mock   = Mock_Device_Desc
	}
}

@struct Timing_Heap_Desc {
	capacity = int
}

@struct Resolve_Timings_Desc {
	first_region = u32
	region_count = u32
	dst          = Buffer
}

@struct Buffer_Range {
	buffer = Buffer
	offset = int
	size   = int
}

@struct Mapped_Buffer_Range {
	@union _ {
		@struct _ {
			buffer = Buffer
			offset = offset
			size   = size
		}
		range = Buffer_Range
	}

	host   = rawptr
	device = Device_Address
}

@struct Buffer_Desc {
	memory_kind    = Memory_Kind
	size           = int
	element_size   = int          // TODO(daniel): I don't want to have this!
	element_format = Pixel_Format // TODO(daniel): I don't want to have this!
	flags          = Buffer_Flags
}

@struct Texture_Subresource_Range {
	first_mip_level   = int
	mip_level_count   = int
	first_array_layer = int
	array_layer_count = int
	aspect            = Texture_Aspect_Mask
}

@struct Texture_Desc {
	memory_kind  = Memory_Kind
	dimension    = Texture_Dimension
	width        = int
	height       = int
	depth        = int
	layer_count  = int
	mip_count    = int
	sample_count = int
	format       = Pixel_Format
	flags        = Texture_Flags
}

@struct Texture_View_Desc {
	texture           = Texture
	view_dimension    = Texture_Dimension
	format            = Pixel_Format
	subresource_range = Texture_Subresource_Range
}

@struct Swapchain_Desc {
	// TODO(daniel): Colorspace management
	window_handle = rawptr
	format        = Pixel_Format
	present_mode  = Present_Mode
	buffer_count  = int
}

@struct Present_Desc {
	sync_interval = int
}

@struct Shader_Bytecode {
	bytes       = [^]u8
	len         = int
	entry_point = string
	format      = Shader_Bytecode_Format
}

@struct Buffer_Descriptor {
	@union _ {
		@struct _ {
			buffer = Buffer
			offset = int
			size   = int
		}
		range = Buffer_Range
	}
}

@struct Buffer_CBV {
	@union _ {
		@struct _ {
			buffer = Buffer
			offset = int
			size   = int
		}
		range = Buffer_Range
	}
}

@struct Buffer_SRV {
	@union _ {
		@struct _ {
			buffer = Buffer
			offset = int
			size   = int
		}
		range = Buffer_Range
	}
}

@struct Buffer_UAV {
	@union _ {
		@struct _ {
			buffer = Buffer
			offset = int
			size   = int
		}
		range = Buffer_Range
	}
}

@struct Texture_Descriptor {
	texture = Texture
	view    = Texture_View
	aspect  = Texture_Aspect_Mask // ignored if view is non-nil
}

@struct Texture_SRV {
	texture = Texture
	view    = Texture_View
	aspect  = Texture_Aspect_Mask // ignored if view is non-nil
}

@struct Texture_UAV {
	texture = Texture
	view    = Texture_View
	aspect  = Texture_Aspect_Mask // ignored if view is non-nil
}

@struct Clear_Value {
	@union _ {
		floats = [4]f32
		uints  = [4]u32
	}
	stencil = u32
}

@struct Pass_Attachment {
	texture     = texture
	load_op     = Load_Op
	store_op    = Store_Op
	clear_value = Clear_Value
}

@struct Render_Info {
	render_area_w          = int
	render_area_h          = int
	viewport               = ^Viewport  // optional
	rect                   = ^Rect      // optional

	color_attachments      = [MAX_COLOR_ATTACHMENTS]Pass_Attachment
	color_attachment_count = int

	depth_attachment   = Pass_Attachment
	stencil_attachment = Pass_Attachment

	writes_depth_stencil = bool // expected depth-stencil texture layout is depth_read if false, depth_write if true.
}

@struct Border_Color {
	is_uint = bool
	@union _ {
		floats = [4]f32
		uints  = [4]u32
	}
}

@struct Sampler_Desc {
	min_filter        = Filter
	mag_filter        = Filter
	mip_filter        = Filter
	address_u         = Address_Mode
	address_v         = Address_Mode
	address_w         = Address_Mode
	mip_lod_bias      = f32
	anisotropy_enable = bool
	max_anisotropy    = f32
	compare_enable    = bool
	comparison_func   = Comparison_Func
	border_color      = Border_Color
	min_lod           = f32
	max_lod           = f32
}

@struct Descriptor_Desc {
	debug_name  = string
	reg         = Register
	kind        = Descriptor_Kind
	count       = int
	data_offset = int
}

@struct Resource_Layout_Desc {
	flags             = Resource_Layout_Flags
	shader_access     = Shader_Kind // TODO(daniel): Shader_Kind is kind of a bad name for what are actually bit flags
	descriptors       = [^]Descriptor_Desc
	descriptors_count = int
}

@struct Compute_Pipeline_Desc {
	resource_layout = Resource_Layout
	compute_shader  = Shader_Bytecode

	// Required for Metal
	threads_per_group = [3]int
}

@struct Blend_Desc {
	blend_enable = bool

	src_blend = Blend
	dst_blend = Blend
	blend_op  = Blend_Op

	src_blend_alpha = Blend
	dst_blend_alpha = Blend
	blend_op_alpha  = Blend

	write_mask = Color_Write_Mask
}

@struct Color_Attachment_Desc {
	format = Pixel_Format
	blend  = Blend_Desc
}

@struct Depth_Stencil_Op_Desc {
	// TODO(daniel): Is this complete?
	stencil_fail_op       = Stencil_Op
	stencil_depth_fail_op = Stencil_Op
	stencil_pass_op       = Stencil_Op
	stencil_func          = Comparison_Func
}

@struct Rasterizer_State {
	fill_mode                  = Fill_Mode
	cull_mode                  = Cull_Mode
	front_winding              = Winding
	depth_clip_enable          = bool
	multisample_enable         = bool
	anti_aliased_line_enable   = bool
	conservative_rasterization = bool
}

@struct Depth_Stencil_State {
	depth_test_enable   = bool
	depth_write_enable  = bool
	stencil_test_enable = bool
	stencil_read_mask   = u8
	stencil_write_mask  = u8
	front               = Depth_Stencil_Op_Desc
	back                = Depth_Stencil_Op_Desc
}

@struct Multisample_State {
	count             = u32
	sample_mask       = u32 // TODO(daniel): Remove this? I think it doesn't exist on Metal and I don't know when you ever use it.
	alpha_to_coverage = bool
}

@struct Attachment_State {
	logic_op_enable        = bool // TODO(daniel): Remove this? Who even uses it
	logic_op               = Logic_Op
	depth_stencil_format   = Pixel_Format
	independent_blend      = bool
	color_attachments      = [MAX_COLOR_ATTACHMENTS]Color_Attachment_Desc
	color_attachment_count = int
}

@struct Graphics_Pipeline_Desc {
	resource_layouts       = [MAX_RESOURCE_BUNDLES]Resource_Layout
	dynamic_constants_size = int

	vertex_shader = Shader_Bytecode
	pixel_shader  = Shader_Bytecode

	topology_class = Primitive_Topology_Class
	rasterizer     = Rasterizer_State
	depth_stencil  = Depth_Stencil_State
	multisample    = Multisample_State
	attachments    = Attachment_State
}

@struct Acceleration_Structure_Build_Info {
	result_size         = int
	build_scratch_size  = int
	upload_scratch_size = int
}

@struct Acceleration_Structure_Geometry_Triangles {
	vertex_format      = Pixel_Format
	vertex_data        = Device_Address
	vertex_data_stride = int
	max_vertex         = int // TODO(daniel): ??
	index_format       = Index_Format
	index_data         = Device_Address
	index_count        = int
	transform_data     = Device_Address
}

@struct Acceleration_Structure_Geometry_AABBs {
	count  = int
	stride = int
	aabbs  = Device_Address
}

@struct Acceleration_Structure_Instance {
	transform = [3][4]f32

	@bit_field(bits=24) instance_id                              = u32
	@bit_field(bits= 8) instance_mask                            = u32
	@bit_field(bits=24) instance_contribution_to_hit_group_index = u32
	@bit_field(bits= 8) flags                                    = u32

	acceleration_structure = Device_Address
}

@struct Acceleration_Structure_Geometry {
	kind  = Acceleration_Structure_Geometry_Kind
	flags = Acceleration_Structure_Geometry_Flags
	@union _ {
		triangles = Acceleration_Structure_Geometry_Triangles
		aabbs     = Acceleration_Structure_Geometry_AABBs
	}
}

@struct Acceleration_Structure_Inputs {
	kind  = Acceleration_Structure_Kind
	mode  = Acceleration_Structure_Build_Mode
	flags = Acceleration_Structure_Build_Flags

	desc_count              = int
	instance_descs          = Device_Address
	instance_flags          = Acceleration_Structure_Geometry_Flags
	geometry_descs          = [^]Acceleration_Structure_Geometry
	geometry_descs_pointers = [^][^]Acceleration_Structure_Geometry
}

@struct Acceleration_Structure_Build_Desc {
	kind         = Acceleration_Structure_Kind
	buffer_range = Buffer_Range
}

// Matches GPU layout
@struct Dispatch_Indirect_Args {
	dispatch_x = u32
	dispatch_y = u32
	dispatch_z = u32
}

// Matches GPU layout
@struct Draw_Indirect_Args {
	vertex_count    = u32
	instance_count  = u32
	vertex_offset   = u32
	instance_offset = u32
}

// Matches GPU layout
@struct Draw_Indexed_Indirect_Args {
	index_count          = u32
	instance_count       = u32
	index_offset         = u32
	base_vertex_location = i32
	instance_offset      = u32
}

@struct Draw {
	pipeline = Pipeline

	resources         = [MAX_RESOURCE_BUNDLES]rawptr
	dynamic_constants = Dynamic_Constants

	@union _ {
		index_count  = u32
		vertex_count = u32
	}

	index_offset    = u32
	vertex_offset   = u32
	instance_count  = u32
	instance_offset = u32
	topology        = Primitive_Topology
	index_format    = Index_Format
	index_buffer    = Buffer
}

@struct Draw_Indirect {
	pipeline               = Pipeline
	resources              = [MAX_RESOURCE_BUNDLES]rawptr
	argument_buffer        = Buffer
	argument_buffer_offset = u64
	count_buffer           = Buffer
	count_buffer_offset    = u64
	max_draw_count         = u32
	stride                 = u32
	index_buffer           = Buffer
	index_format           = Index_Format
	topology               = Primitive_Topology
}

@struct Texture_Barrier {
	prev_sync   = Sync_Flags
	next_sync   = Sync_Flags
	prev_layout = Texture_Layout
	next_layout = Texture_Layout
	texture     = Texture
	subresource = Texture_Subresource_Layout
	discard     = bool
}

@struct Submit_Command_Lists_Desc {
	debug_name = string

	lists      = [^]Command_List
	list_count = int

	submission_fence_value = [^]u64
}

@struct Command_Breadcrumb {
	location  = Source_Code_Location
	user_data = rawptr
}

@struct Command_Chunk {
	next = [^]Command_Chunk

	count       = int
	capacity    = int
	kind        = [^]Command_Kind
	data        = [^]rawptr
	sort_keys   = [^]u64
	breadcrumbs = [^]Command_Breadcrumb
}

@struct Command_Begin_Rendering {
	render_info = Render_Info
}

@struct Command_Draws {
	count        = int
	sort_indices = [^]u32
	draw_calls   = [^]Draw // TODO(daniel): Flexible array member
}

@struct Command_Draws_Indirect {
	count        = int
	sort_indices = [^]u32
	draw_calls   = [^]Draw_Indirect // TODO(daniel): Flexible array member
}

@struct Command_Dispatch {
	pipeline   = Pipeline
	resources  = rawptr
	dispatch_x = u32
	dispatch_y = u32
	dispatch_z = u32
}

@struct Command_Dispatch_Indirect {
	pipeline               = Pipeline
	resources              = rawptr
	argument_buffer        = Buffer
	argument_buffer_offset = u64
}

@struct Command_Copy_Buffer {
	dst        = Buffer
	dst_offset = int
	src        = Buffer_Range
}

@struct Texture_Update {
	dst_subresource      = Texture_Subresource_Range
	dst_texture_region   = Box
	src_buffer_offset    = int
	src_buffer_row_pitch = int
}

@struct Command_Update_Texture {
	dst_texture  = Texture
	src_buffer   = Buffer
	update_count = int
	updates      = [^]Texture_Update // TODO(daniel): Flexible array member
}

@struct Copy_Texture_Region {
	dst_subresource = Texture_Subresource_Range
	src_subresource = Texture_Subresource_Range
	dst_offset      = [3]i32
	src_offset      = [3]i32
	extent          = [3]i32
}

@struct Command_Copy_Texture {
	dst          = Texture
	src          = Texture
	region_count = int
	regions      = [^]Copy_Texture_Region // TODO(daniel): Flexible array member
}

@struct Command_Barriers {
	prev_sync = Sync_Flags
	next_sync = Sync_Flags

	texture_count = int
	textures      = [0]Texture_Barrier // Flexible Array Member
}

@struct Command_Begin_Region {
	name  = string
	color = Color
}

@struct Command_Begin_Timing {
	heap  = Timing_Heap
	index = u32
}

@struct Command_End_Timing {
	heap  = Timing_Heap
	index = u32
}

@struct Command_Resolve_Timings {
	heap = Timing_Heap
	desc = Resolve_Timings_Desc
}

@struct Command_Update_Acceleration_Structure {
	desc = Acceleration_Structure_Build_Desc
}

//
// Functions
//

// Instance
@fn(returns=Error)            create_instance                 { @by_ptr desc = Instance_Desc, out_instance = ^Instance }
@fn(returns=Error)            destroy_instance                { instance = Instance }
@fn(returns=Instance_Version) get_instance_version            { instance = Instance }

// Debug
@fn                           set_validation_message_callback { instance = Instance, callback = Validation_Message_Proc, user_data = rawptr }
@fn                           print_debug_diagnostics         { instance = Instance }
@fn(returns=string)           error_to_string                 { error = Error }

// Device (Basics)
@fn(returns=Error)          create_device      { instance = Instance, @by_ptr desc = Device_Desc, out_device = ^Device }
@fn(returns=Error)          destroy_device     { device = Device }
@fn(returns=Device_Version) device_get_version { device = Device }
@fn(returns=Instance)       device_get_parent  { device = Device }

// Device (Synchronization)
@fn(returns=Error) get_queue_completed_value { device = Device, queue = Queue_Kind, out_fence_value = ^u64 }
@fn(returns=Error) fence_value_completed     { device = Device, queue = Queue_Kind, fence_value = u64, result = ^bool }
@fn(returns=Error) wait_on_fence_value       { device = Device, queue = Queue_Kind, fence_value = u64 }
@fn(returns=Error) next_frame                { device = Device } // TODO(daniel)

// Device (Swapchain)
@fn(returns=Error)         create_swapchain                   { device = Device, @by_ptr desc = Swapchain_Desc, name = string, out_swapchain = ^Swapchain }
@fn(returns=Error)         destroy_swapchain                  { device = Device, swapchain = Swapchain }
@fn(returns=Error)         wait_and_acquire_swapchain_texture { device = Device, swapchain = Swapchain, command_list = Command_List, out_texture = ^Texture }
@fn(returns=^Texture_Desc) get_swapchain_texture_desc         { device = Device, swapchain = Swapchain }

// Device (Resource Creation / Destruction)
@fn(returns=Error) create_buffer                  { device = Device, @by_ptr desc = Buffer_Desc, name = string, out_buffer = ^Buffer }
@fn(returns=Error) destroy_buffer                 { device = Device, buffer = Buffer }
@fn(returns=Error) create_texture                 { device = Device, @by_ptr desc = Texture_Desc, name = string, out_texture = ^Texture }
@fn(returns=Error) destroy_texture                { device = Device, texture = Texture }
@fn(returns=Error) create_texture_view            { device = Device, @by_ptr desc = Texture_View_Desc, name = string, out_view = ^Texture_View }
@fn(returns=Error) destroy_texture_View           { device = Device, view = Texture_View }
@fn(returns=Error) create_sampler                 { device = Device, @by_ptr desc = Sampler_Desc, name = string, out_sampler = ^Sampler }
@fn(returns=Error) destroy_sampler                { device = Device, sampler = Sampler }
@fn(returns=Error) create_acceleration_structure  { device = Device, @by_ptr desc = Acceleration_Structure_Desc, name = string, out_as = ^Acceleration_Structure }
@fn(returns=Error) destroy_acceleration_structure { device = Device, as = Acceleration_Structure }
@fn(returns=Error) create_timing_heap             { device = Device, @by_ptr desc = Timing_Heap_Desc, name = string, out_heap = ^Timing_Heap }
@fn(returns=Error) destroy_timing_heap            { device = Device, heap = Timing_Heap }
@fn(returns=Error) create_resource_layout         { device = Device, @by_ptr desc = Resource_Layout_Desc, name = string, out_layout = ^Resource_Layout }
@fn(returns=Error) destroy_resource_layout        { device = Device, layout = Resource_Layout }
@fn(returns=Error) create_compute_pipeline        { device = Device, @by_ptr desc = Compute_Pipeline_Desc, name = string, out_pipeline = ^Pipeline }
@fn(returns=Error) create_graphics_pipeline       { device = Device, @by_ptr desc = Graphics_Pipeline_Desc, name = string, out_pipeline = ^Pipeline }
@fn(returns=Error) destroy_pipeline               { device = Device, pipeline = Pipeline }

// Device (Buffers)
@fn(returns=bool)           buffer_is_valid           { device = Device, buffer = Buffer }
@fn(returns=[^]Buffer_Desc) buffer_get_desc           { device = Device, buffer = Buffer }
@fn(returns=string)         buffer_get_name           { device = Device, buffer = Buffer }
@fn(returns=u32)            buffer_get_bindless_index { device = Device, buffer = Buffer }
@fn(returns=rawptr)         buffer_get_host_address   { device = Device, buffer = Buffer }
@fn(returns=Device_Address) buffer_get_device_address { device = Device, buffer = Buffer }

// Device (Textures)
@fn(returns=bool)            texture_is_valid           { device = Device, texture = Texture }
@fn(returns=[^]Texture_Desc) texture_get_desc           { device = Device, texture = Texture }
@fn(returns=string)          texture_get_name           { device = Device, texture = Texture }
@fn(returns=u32)             texture_get_bindless_index { device = Device, texture = Texture }
@fn(returns=rawptr)          texture_get_host_address   { device = Device, texture = Texture }
@fn(returns=Device_Address)  texture_get_device_address { device = Device, texture = Texture }

// Device (Texture Views)
@fn(returns=bool)                 texture_view_is_valid           { device = Device, view = Texture_View }
@fn(returns=[^]Texture_View_Desc) texture_view_get_desc           { device = Device, view = Texture_View }
@fn(returns=string)               texture_view_get_name           { device = Device, view = Texture_View }
@fn(returns=u32)                  texture_view_get_bindless_index { device = Device, view = Texture_View }

// Device (Command Lists)
@fn(returns=Error) begin_commands               { device = Device, queue = Queue_Kind, out_list = ^Command_List }
@fn(returns=Error) abort_commands               { device = Device, list = Command_List }
@fn(returns=Error) submit_commands              { device = Device, @by_ptr desc = Submit_Command_Lists_Desc }
@fn(returns=Error) recycle_command_lists        { device = Device }
@fn(returns=Error) device_wait_for_command_list { device = Device, wait_queue = Queue, execute_queue = Queue, fence_value = u64 }

// Device (Queues)
@fn queue_begin_region { device = Device, queue = Queue, name = string, color = Color }
@fn queue_end_region   { device = Device, queue = Queue }

// Command List (Basics)
@fn(returns=Command_List_Version) list_get_version { list = Command_List }
@fn(returns=Device)               list_get_parent  { list = Command_List }

// Command List (Transient Host Allocator)
@fn(returns=rawptr) list_cpu_alloc { list = Command_List, size = int, align = int, zero_memory = false }
@fn(returns=rawptr) list_cpu_copy  { list = Command_List, size = int, align = int, source = rawptr }

// Command List (Transient Buffer Allocator)
@fn(returns=Mapped_Buffer_Range) list_gpu_alloc                   { list = Command_List, size = int, align = int }
@fn(returns=Mapped_Buffer_Range) list_gpu_alloc_constant_buffer   { list = Command_List, size = int }
@fn(returns=Mapped_Buffer_Range) list_gpu_alloc_structured_buffer { list = Command_List, element_size = int, element_count = int }

// Command List (Command Management)
@fn list_set_sort_key        { list = Command_List, sort_key = u64 }
@fn list_set_next_breadcrumb { list = Command_List, @by_ptr breadcrumb = Command_Breadcrumb }

// Command List (Commands, Regions)
@fn begin_region { list = Command_List, name = string, color = Color }
@fn end_region   { list = Command_List }

// Command List (Commands, Timing)
@fn begin_timing    { list = Command_List, heap = Timing_Heap, region = u32 }
@fn end_timing      { list = Command_List, heap = Timing_Heap, region = u32 }
@fn resolve_timings { list = Command_List, heap = Timing_Heap, @by_ptr desc = Resolve_Timings_Desc }

// Command List (Commands, Rendering)
@fn begin_rendering { list = Command_List, @by_ptr info = Render_Info }
@fn draw            { list = Command_List, @by_ptr draw = Draw }
@fn draw_indirect   { list = Command_List, @by_ptr draw = Draw_Indirect }
@fn end_rendering   { list = Command_List }

// Command List (Commands, Dispatch)
@fn dispatch          { list = Command_List, pipeline = Pipeline, resources = rawptr, dispatch = [3]u32 }
@fn dispatch_indirect { list = Command_List, pipeline = Pipeline, resources = rawptr, args = Buffer, args_offset = u32 }

// Command List (Commands, Transfer)
@fn copy_buffer                   { list = Command_List, dst = Buffer, dst_offset = int, src = Buffer_Range }
@fn copy_texture                  { list = Command_List, dst = Texture, src = Texture }
@fn copy_texture_regions          { list = Command_List, dst = Texture, src = Texture, region_count = int, regions = [^]Copy_Texture_Regions }
@fn update_texture                { list = Command_List, dst = Texture, src = Buffer, update_count = int, updates = [^]Texture_Update }
@fn update_acceleration_structure { list = Command_List, @by_ptr desc = Acceleration_Structure_Build_Desc }

// Command List (Commands, Barriers)
@fn barrier               { list = Command_List, before = Sync_Flags, after = Sync_Flags }
@fn transition            { list = Command_List, texture = Texture, before = Sync_Flags, after = Sync_Flags, before_layout = Texture_Layout, after_layout = Texture_Layout }
@fn transition_ex         { list = Command_List, @by_ptr barrier = Texture_Barrier }
@fn flush_staged_barriers { list = Command_List }

// Command List (Deferred Destroy)
@fn deferred_destroy_buffer       { list = Command_List, buffer      = Buffer }
@fn deferred_destroy_texture      { list = Command_List, texture     = Texture }
@fn deferred_destroy_texture_view { list = Command_List, view        = Texture_View }
@fn deferred_destroy_sampler      { list = Command_List, sampler     = Sampler }
@fn deferred_destroy_pipeline     { list = Command_List, pipeline    = Pipeline }
@fn deferred_destroy_swapchain    { list = Command_List, swapchain   = Swapchain }
@fn deferred_destroy_timing_heap  { list = Command_List, timing_heap = Timing_Heap }

// Command List (Internals)
@fn(returns=^Command_Chunk) list_get_first_command_chunk { list = Command_List }
