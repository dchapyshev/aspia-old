/*
 * PROJECT:         Aspia
 * FILE:            aspia\tests.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"

VOID
TESTS_MemTest(VOID)
{
    IoAddColumnsList(StdColumnList);
}
VOID
TESTS_HDDTest(VOID)
{
    IoAddColumnsList(StdColumnList);
}

VOID
TESTS_CPUTest(VOID)
{
    IoAddColumnsList(StdColumnList);
}

VOID
TESTS_StressTest(VOID)
{
    IoAddColumnsList(StdColumnList);
}
