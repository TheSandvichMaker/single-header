#define DC_WITH_TESTING
#define DC_STATIC
#define DC_IMPL
#include "dc.h"

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
	}
}

int entry_point(void)
{
	Test_Context *t = &(Test_Context){0};

	TEST_RUN(t, test_unicode_conversion);

	return test_report(t);
}