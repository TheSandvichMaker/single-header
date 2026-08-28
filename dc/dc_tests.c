#define DC_WITH_TESTING
#define DC_STATIC
#define DC_IMPL
#include "dc.h"

void test_integer_parsing(Test_Context *t)
{
	String i64_min = S("-9223372036854775807");
	String i64_max = S("9223372036854775807");

	Parse_Number_Result result;

	result = string_parse_i64(i64_min);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_i64 == INT64_MIN);

	result = string_parse_i64(i64_max);
	TEST_CHECK(t, result.is_valid);
	TEST_CHECK(t, result.value_i64 == INT64_MAX);

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

	for (i64 i = INT32_MIN; i < INT32_MAX; i += 1337)
	{
		char buf[64];
		String str = string_format_into_buffer(buf, sizeof(buf), "%lld", i);

		result = string_parse_i32(str);
		TEST_CHECK(t, result.is_valid);
		TEST_CHECK(t, result.value_i32 == i);
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

int entry_point(void)
{
	Test_Context *t = &(Test_Context){0};

	TEST_RUN(t, test_integer_parsing);
	TEST_RUN(t, test_unicode_conversion);

	return test_report(t);
}