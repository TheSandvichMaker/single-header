#define DC_WITH_TESTING
#define DC_STATIC
#define DC_IMPL
#include "dc.h"

void test_integer_parsing(Test_Context *t)
{
	String i64_min = S("-9223372036854775808");
	String i64_max = S("9223372036854775807");
	String u64_max = S("0xffffffffffffffff");

	Parse_Number_Result result;

	result = string_parse_u64(u64_max);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_u64 == UINT64_MAX);
	TEST_CHECK(t, !result.overflowed);

	result = string_parse_i64(u64_max);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_i64 == INT64_MAX);
	TEST_CHECK(t, result.overflowed == 1);

	result = string_parse_i64(i64_min);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_i64 == INT64_MIN);
	TEST_CHECK(t, result.overflowed == 0);

	result = string_parse_i64(i64_max);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_i64 == INT64_MAX);
	TEST_CHECK(t, !result.overflowed);

	String hex_upper = S("0X123456789ABCDEF");
	String hex_lower = S("0x123456789abcdef");

	result = string_parse_u64(hex_upper);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_u64 == 0x123456789ABCDEFull);

	result = string_parse_u64(hex_lower);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_u64 == 0x123456789ABCDEFull);

	String octal = S("01234567");

	result = string_parse_u64(octal);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_u64 == 01234567);

	for (i64 i = INT16_MIN; i <= INT16_MAX; i += 1)
	{
		char buf[64];
		String str = string_format_into_buffer(buf, sizeof(buf), "%lld", i);

		result = string_parse_i16(str);
		TEST_CHECK(t, result.is_valid);
		TEST_CHECK(t, result.value_i16 == i);
	}

	// Too small
	result = string_parse_i32(i64_min);
	TEST_CHECK(t, result.overflowed == -1);

	// Too big
	result = string_parse_i32(i64_max);
	TEST_CHECK(t, result.overflowed == +1);
}

void test_unicode_conversion(Test_Context *t)
{
	Arena_ScopedTemp {
		// utf8 -> utf16
		{
			String str = S("Hello Sailor!");
			String16 expected = S16("Hello Sailor!");
			String16 result = utf16_from_utf8(temp, str);
			TEST_CHECK(t, string16_match(result, expected));
		}
		// utf16 -> utf8
		{
			String16 str = S16("Hello Sailor!");
			String expected = S("Hello Sailor!");
			String result = utf8_from_utf16(temp, str);
			TEST_CHECK(t, string_match(result, expected), "expected: \"%cs\", result: \"%cs\"", expected, result);
		}
		// roundtrip
		{
			String str = S("Langt borte herfra, der hvor svalerne flyver hen, når vi har vinter, boede en konge, som havde elve sønner og én datter, Elisa. De elve brødre, prinser var de, gik i skole med stjerne på brystet og sabel ved siden; de skrev på guldtavle med diamantgriffel og læste lige så godt udenad, som indeni; man kunne straks høre, at de var prinser. Søsteren Elisa sad på en lille skammel af spejlglas og havde en billedbog, der var købt for det halve kongerige.");
			String16 ping = utf16_from_utf8(temp, str);
			String pong = utf8_from_utf16(temp, ping);
			TEST_CHECK(t, string_match(str, pong));
		}
	}
}

fn_local void test_job(Job *job, Job_Execution_Context *ctx)
{
	int *values = (int *)job->user_ptrs[0];

	values[ctx->worker_thread_index] += 1;
}

fn_local void finalize_job(Job *job, Job_Execution_Context *ctx)
{
	(void)ctx;

	int *sum    = (int *)job->user_ptrs[0];
	int *values = (int *)job->user_ptrs[1];
	for (int i = 0; i < 8; i += 1)
	{
		*sum += values[i];
	}
}

typedef struct Parallel_For_Test_Data
{
	int  addend;
	int *values;
} Parallel_For_Test_Data;

void parallel_for_proc(void *in_data, Range range)
{
	Parallel_For_Test_Data *data = in_data;

	for (EachInRange(i, range))
	{
		data->values[i] += data->addend;
	}
}

void test_job_system(Test_Context *t)
{
	{
		int values[8] = {0};
		int finalized = 0;

		Job *root = job_create(NULL, NULL);
	
		for (int i = 0; i < 1024; i += 1)
		{
			Job *job = job_create(test_job, root);
			job->user_ptrs[0] = values;

			job_run(job);
		}

		Job *finalizer = job_create(finalize_job, NULL);
		job_add_continuation(root, finalizer);
		finalizer->user_ptrs[0] = &finalized;
		finalizer->user_ptrs[1] = &values[0];

		job_run(root);
		job_wait(root);

		int sum = 0;
		for (int i = 0; i < 8; i += 1) sum += values[i];

		TEST_CHECK(t, sum == finalized);
	}

	Arena_ScopedTemp {
		Parallel_For_Test_Data data = {
			.addend = 2,
			.values = arena_alloc_array(temp, 8192, int),
		};
		Job *job = parallel_for(parallel_for_proc, 8192, &data);
		job_run(job);
		job_wait(job);

		for (isz i = 0; i < 8192; i += 1)
		{
			TEST_CHECK(t, data.values[i] == data.addend);
		}
	}
}

int entry_point(void)
{
	job_system_init(8);

	Test_Context *t = &(Test_Context){0};

	TEST_RUN(t, test_integer_parsing);
	TEST_RUN(t, test_unicode_conversion);
	TEST_RUN(t, test_job_system);

	return test_report(t);
}