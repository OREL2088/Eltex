#include "test_permissions.h"

#include <CUnit/Basic.h>
#include <sys/types.h>

#include "../permissions.h"

void test_parse_octal_permissions(void)
{
    mode_t mode = 0;

    CU_ASSERT_TRUE(parse_octal_permissions("755", &mode));
    CU_ASSERT_EQUAL(mode, 0755);
    CU_ASSERT_TRUE(parse_octal_permissions("000", &mode));
    CU_ASSERT_EQUAL(mode, 0000);
    CU_ASSERT_FALSE(parse_octal_permissions("888", &mode));
    CU_ASSERT_FALSE(parse_octal_permissions("0755", &mode));
}

void test_parse_letter_permissions(void)
{
    mode_t mode = 0;

    CU_ASSERT_TRUE(parse_letter_permissions("rwxr-xr--", &mode));
    CU_ASSERT_EQUAL(mode, 0754);
    CU_ASSERT_FALSE(parse_letter_permissions("rwxr-xrw", &mode));
    CU_ASSERT_FALSE(parse_letter_permissions("rwxr-xr-", &mode));
}

void test_format_permissions(void)
{
    char letters[10];
    char bits[10];

    format_letter_permissions(0755, letters);
    format_binary_permissions(0755, bits);

    CU_ASSERT_STRING_EQUAL(letters, "rwxr-xr-x");
    CU_ASSERT_STRING_EQUAL(bits, "111101101");
}

void test_apply_add_and_remove(void)
{
    mode_t result = 0;

    CU_ASSERT_TRUE(apply_permission_command(0644, "u+x", &result));
    CU_ASSERT_EQUAL(result, 0744);
    CU_ASSERT_TRUE(apply_permission_command(0777, "go-w", &result));
    CU_ASSERT_EQUAL(result, 0755);
}

void test_apply_assignment_and_sequence(void)
{
    mode_t result = 0;

    CU_ASSERT_TRUE(apply_permission_command(0777, "u=rw,g=r,o=", &result));
    CU_ASSERT_EQUAL(result, 0640);
    CU_ASSERT_TRUE(apply_permission_command(0600, "ug+rx,o+r", &result));
    CU_ASSERT_EQUAL(result, 0754);
}

void test_reject_command_atomically(void)
{
    mode_t result = 0123;

    CU_ASSERT_FALSE(apply_permission_command(0644, "u+x,g+z", &result));
    CU_ASSERT_EQUAL(result, 0644);
    CU_ASSERT_FALSE(apply_permission_command(0644, "u+", &result));
    CU_ASSERT_EQUAL(result, 0644);
    CU_ASSERT_FALSE(apply_permission_command(0644, "ua+x", &result));
    CU_ASSERT_EQUAL(result, 0644);
}
