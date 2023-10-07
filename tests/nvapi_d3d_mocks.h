#pragma once

#include "nvapi_tests_private.h"

class UnknownMock : public trompeloeil::mock_interface<IUnknown> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
};
