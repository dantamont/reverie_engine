#include <gtest/gtest.h>
#include "fortress/templates/GTemplates.h"

namespace rev {

TEST(TemplateTests, UniquePointerCheck) {
    static_assert(is_unique_ptr<std::unique_ptr<int>>(), "Should be a pointer");
    static_assert(!is_unique_ptr<int>(), "Better not be a pointer");
}


} /// End rev namespace
